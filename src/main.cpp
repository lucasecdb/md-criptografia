#include <iostream>
#include <unistd.h>
#include <ctime>

// PCM library
#include "pcm.h"

// AES crypto library
#include <crypto++/osrng.h> // required by AutoSeededRandomPool
#include <crypto++/modes.h>

using namespace std;
using namespace PCM_MD;
using namespace CryptoPP;

bool ENCRYPT = false;
bool NO_GEN = false;

enum
{
	ENC,
	DEC
};

void usage()
{
	printf("usage: ./pcm-crypt [OPTIONS...] in_file out_file\n");
	printf("\nOptions:\n");
	printf("\t-d key_file     Decrypt in_file with key in the key_file and put the decryption result in out_file\n");
	printf("\t-e [key_file]   Encrypt in_file with optional key in key_file and put the encryption result in out_file\n");
}

void gen_key(byte key[AES::DEFAULT_KEYLENGTH + 1])
{
	AutoSeededRandomPool rnd;
	rnd.GenerateBlock(key, AES::DEFAULT_KEYLENGTH);
}

void read_key(char* key_file, byte key[16])
{
	if (access(key_file, F_OK) == -1)
	{
		fprintf(stderr, "No such file %s\n", key_file);
		usage();
		exit(1);
	}

	ifstream file(key_file, ios::binary);

	if (file.is_open())
	{
		// get file size and assert it to 16 bytes
		file.seekg(0, ios_base::end);
		int size = file.tellg();

		if (size != 16)
		{
			fprintf(stderr, "File %s must be only 16 bytes long\n", key_file);
			usage();
			exit(1);
		}

		file.seekg(0, ios_base::beg);
		
		for (int i = 0; i < 16; i++) {
			file.read((char*)&key[i], sizeof(byte));
		}

		file.close();
	}
	else
	{
		fprintf(stderr, "Couldn't open file %s, exiting...\n", key_file);
		exit(1);
	}
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

void cfb_algo(string in, string out, const byte* key, int opt)
{
	printf("[*] %s %s\n", opt == ENC ? "Encrypting": "Decrypting", in.c_str());

	PCM in_audio(in);
	AutoSeededRandomPool rnd;

	// get raw audio data
	char* data = (char*) in_audio.get_data();

	// gen random iv
	byte iv[AES::BLOCKSIZE];
	rnd.GenerateBlock(iv, AES::BLOCKSIZE);

	if (opt == ENC)
	{
		CFB_Mode<AES>::Encryption cfb(key, AES::DEFAULT_KEYLENGTH, iv, 1);
		cfb.ProcessData((byte*)data, (byte*)data, in_audio.get_data_size());
	}
	else
	{
		CFB_Mode<AES>::Decryption cfb(key, AES::DEFAULT_KEYLENGTH, iv, 1);
		cfb.ProcessData((byte*)data, (byte*)data, in_audio.get_data_size());
	}

	printf("[*] Finished %s\n", opt == ENC ? "encrypting": "decrypting");

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
	byte key[AES::DEFAULT_KEYLENGTH + 1];
	char* key_file;

	// get command line arguments
	char c;
	while ((c = getopt(argc, argv, "ed:")) != -1)
	{
		switch (c)
		{
			case 'e':
				ENCRYPT = true;
				if (argc == 5)
				{
					NO_GEN = true;
					key_file = argv[optind];
					read_key(key_file, key);
				}
				break;
			case 'd':
				// check for proper usage
				if (argc != 5)
				{
					fprintf(stderr, "In decryption mode, you must give a file with a key of %d bytes long.\n", AES::DEFAULT_KEYLENGTH);
					usage();
					return 1;
				}
				key_file = optarg;
				read_key(key_file, key);
				break;
			default:
				usage();
				return 1;
		}
	}

	string in_file = argv[argc-2];
	string out_file = argv[argc-1];

	// check for file existence
	if (access(in_file.c_str(), F_OK) == -1)
	{
		fprintf(stderr, "No such file %s\n", in_file.c_str());
		usage();
		return 1;
	}

	if (ENCRYPT)
	{
		if (!NO_GEN)
		{
			printf("[*] Generating key\n");
			gen_key(key);
			
			// write key to file.key
			int counter = 0;
			char key_file_name[12];

			sprintf(key_file_name, "%07d.key", counter);

			while (access(key_file_name, F_OK) != -1)
			{
				counter++;
				sprintf(key_file_name, "%07d.key", counter);
			}

			printf("[*] Writing key into %s\n", key_file_name);

			ofstream out_key_file(key_file_name, ios::binary);
			out_key_file.write((char*)key, 16);

			out_key_file.close();
		}
		cfb_algo(in_file, out_file, key, ENC);
	}
	else
	{
		cfb_algo(in_file, out_file, key, DEC);
	}
}
