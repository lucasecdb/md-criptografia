#include <iostream>
#include <unistd.h>

// PCM library
#include "pcm.h"

// AES crypto library
#include <crypto++/osrng.h> // required by AutoSeededRandomPool
#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <crypto++/filters.h>

using namespace std;
using namespace PCM_MD;
using namespace CryptoPP;

bool ENCRYPT = false;

void usage()
{
	printf("usage: ./main [OPTIONS...] in_file out_file\n");
	printf("\nOptions:\n");
	printf("\t-d key     Decrypt in_file with key=key and put the decryption result in out_file\n");
	printf("\t-e         Encrypt in_file and put the encryption result in out_file\n");
}

void gen_key(byte key[AES::DEFAULT_KEYLENGTH])
{
	AutoSeededRandomPool rnd;

	rnd.GenerateBlock(key, AES::DEFAULT_KEYLENGTH);
}

void write_audio(PCM in_audio, string out, char* data)
{
	ofstream out_file(out.c_str(), ios::binary);

	if (out_file.is_open())
	{
		printf("[*] Writing headers\n");

		// RIFF headers
		RIFF_CHUNK in_riff = in_audio.get_riff();

		out_file.write((char*)&in_riff.chunk_id, sizeof(in_riff.chunk_id));
		out_file.write((char*)&in_riff.chunk_size, sizeof(in_riff.chunk_size));
		out_file.write((char*)&in_riff.format, sizeof(in_riff.format));

		// FMT headers
		FMT_CHUNK in_fmt = in_audio.get_fmt();

		out_file.write((char*)&in_fmt.sub_chunk_id, sizeof(in_fmt.sub_chunk_id));
		out_file.write((char*)&in_fmt.sub_chunk_size, sizeof(in_fmt.sub_chunk_size));
		out_file.write((char*)&in_fmt.audio_format, sizeof(in_fmt.audio_format));
		out_file.write((char*)&in_fmt.num_channels, sizeof(in_fmt.num_channels));
		out_file.write((char*)&in_fmt.sample_rate, sizeof(in_fmt.sample_rate));
		out_file.write((char*)&in_fmt.byte_rate, sizeof(in_fmt.byte_rate));
		out_file.write((char*)&in_fmt.block_align, sizeof(in_fmt.block_align));
		out_file.write((char*)&in_fmt.bits_per_sample, sizeof(in_fmt.bits_per_sample));

		// DATA_CHUNK headers
		DATA_CHUNK in_data_chunk = in_audio.get_data_chunk();

		out_file.write((char*)&in_data_chunk.sub_chunk_id, sizeof(in_data_chunk.sub_chunk_id));
		out_file.write((char*)&in_data_chunk.sub_chunk_size, sizeof(in_data_chunk.sub_chunk_size));

		printf("[*] Writing data\n");

		// write actual crypted data
		for (int i = 0; i < in_data_chunk.sub_chunk_size; i++)
		{
			out_file.write(&data[i], sizeof(data[i]));
		}

		printf("[*] Finished writing to file %s\n", out.c_str());

		out_file.close();
	}
	else
	{
		fprintf(stderr, "[!!] Couldn't open %s, exiting..\n", out.c_str());
		exit(1);
	}
}

void decrypt(string in, string out, const byte* key)
{
	printf("[*] Decrypting %s with key %s\n", in.c_str(), key);

	PCM in_audio(in);
	AutoSeededRandomPool rnd;

	// get raw audio data
	char* data = (char*) in_audio.get_data();

	// gen random iv
	byte iv[AES::BLOCKSIZE];
	rnd.GenerateBlock(iv, AES::BLOCKSIZE);

	// decrypt
	CFB_Mode<AES>::Decryption cfb_decryption(key, strlen((char*)key), iv);
	cfb_decryption.ProcessData((byte*)data, (byte*)data, in_audio.get_data_size());

	printf("[*] Finished decryption\n");

	printf("[*] Writing to file %s\n", out.c_str());
	write_audio(in_audio, out, data);
}

void encrypt(string in, string out, const byte* key)
{
	printf("[*] Encrypting %s with key %s\n", in.c_str(), key);

	PCM in_audio(in);
	AutoSeededRandomPool rnd;

	// get raw audio data
	char* data = (char*) in_audio.get_data();

	// gen random iv
	byte iv[AES::BLOCKSIZE];
	rnd.GenerateBlock(iv, AES::BLOCKSIZE);

	// encrypt
	CFB_Mode<AES>::Encryption cfb_encryption(key, strlen((char*)key), iv);
	cfb_encryption.ProcessData((byte*)data, (byte*)data, in_audio.get_data_size());

	printf("[*] Finished encryption\n");

	printf("[*] Writing to file %s\n", out.c_str());
	write_audio(in_audio, out, data);
}

int main(int argc, char* argv[])
{
	// ensure proper usage
	if (argc < 4 || argc > 5)
	{
		usage();
		return 1;
	}

	// key to encrypt and decrypt
	byte key[AES::DEFAULT_KEYLENGTH];

	// get command line arguments
	char c;
	while ((c = getopt(argc, argv, "ed:")) != -1)
	{
		switch (c)
		{
			case 'e':
				ENCRYPT = true;
				break;
			case 'd':
				// check for proper usage
				if (argc != 4 || strlen(optarg) != AES::DEFAULT_KEYLENGTH)
				{
					fprintf(stderr, "In decryption mode, one must give a 16 bytes long key.\n");
					usage();
					return 1;
				}
				strncpy((char*)key, optarg, AES::DEFAULT_KEYLENGTH);
				break;
			default:
				usage();
				return 1;
		}
	}

	string in_file = argv[optind++];
	string out_file = argv[optind];

	// check for file existence
	if (access(in_file.c_str(), F_OK) == -1)
	{
		fprintf(stderr, "No such file %s\n", in_file.c_str());
		usage();
		return 1;
	}

	if (ENCRYPT)
	{
		gen_key(key);
		encrypt(in_file, out_file, key);
	}
	else
	{
		decrypt(in_file, out_file, key);
	}
}
