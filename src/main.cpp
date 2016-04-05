#include <iostream>
#include <unistd.h>
#include <stdio.h>
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

	FILE* file = fopen(key_file, "rb");

	if (file != NULL)
	{
		// get file size and assert it to 16 bytes
		fseek(file, 0, SEEK_END);
		int size = ftell(file);

		if (size != 16)
		{
			fprintf(stderr, "File %s must be only 16 bytes long\n", key_file);
			usage();
			exit(1);
		}

		fseek(file, 0, SEEK_SET);

		for (int i = 0; i < 16; i++) {
			fread(&key[i], 1, sizeof(key[i]), file);
		}

		fclose(file);
	}
	else
	{
		fprintf(stderr, "Couldn't open file %s, exiting...\n", key_file);
		exit(1);
	}
}

void write_audio(PCM in_audio, string out, char* data)
{
	FILE* out_file = fopen(out.c_str(), "wb");

	if (out_file != NULL)
	{
		printf("[*] Writing headers\n");

		// WAV_HDR headers
		WAV_HDR wav = in_audio.get_wav();
		fwrite(&wav, 1, sizeof(wav), out_file);

		// DATA_CHUNK headers
		DATA_CHUNK data_chunk = in_audio.get_data_chunk();
		fwrite(&data_chunk, 1, sizeof(data_chunk), out_file);

		printf("[*] Writing data\n");

		// write actual crypted data
		for (int i = 0; i < data_chunk.sub_chunk_size; i++)
		{
			fwrite(&data[i], 1, sizeof(data[i]), out_file);
		}

		printf("[*] Finished writing to file %s\n", out.c_str());

		fclose(out_file);
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

			FILE* out_key_file = fopen(key_file_name, "wb");
			fwrite(key, 1, sizeof(byte) * 16, out_key_file);

			fclose(out_key_file);
		}
		cfb_algo(in_file, out_file, key, ENC);
	}
	else
	{
		cfb_algo(in_file, out_file, key, DEC);
	}
}
