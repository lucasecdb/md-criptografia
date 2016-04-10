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

enum {
	ENC,
	DEC
};

void usage() {
	printf("usage: ./pcm-crypt [OPTIONS...] in_file out_file\n");
	printf("\nOptions:\n");
	printf("\t-d key_file     Decrypt in_file with key in the key_file and put the decryption result in out_file\n");
	printf("\t-e [key_file]   Encrypt in_file with optional key in key_file and put the encryption result in out_file\n");
}

// generate random key using AutoSeededRandomPool (provided by CryptoPP)
void gen_key(byte key[AES::DEFAULT_KEYLENGTH + 1]) {
	AutoSeededRandomPool rnd;
	rnd.GenerateBlock(key, AES::DEFAULT_KEYLENGTH);
}

// read key from key_file and puts it in key
void read_key(char* key_file, byte key[16]) {
	// check for file existence (unistd.h)
	if (access(key_file, F_OK) == -1) {
		fprintf(stderr, "No such file %s\n", key_file);
		usage();
		exit(1);
	}

	// open file with read-binary mode
	FILE* file = fopen(key_file, "rb");

	// check if we opened it sucessfully
	if (file != NULL) {
		// get file size and assert it to 16 bytes
		fseek(file, 0, SEEK_END);
		int size = ftell(file);

		if (size != 16) {
			fprintf(stderr, "File %s must be only 16 bytes long\n", key_file);
			usage();
			exit(1);
		}

		fseek(file, 0, SEEK_SET);

		// read 16 bytes from the file
		for (int i = 0; i < 16; i++) {
			fread(&key[i], 1, sizeof(key[i]), file);
		}

		// finally, close the file
		fclose(file);
	}
	else {
		fprintf(stderr, "Couldn't open file %s, exiting...\n", key_file);
		exit(1);
	}
}

// write audio headers from in_audio in out file with data as data
void write_audio(PCM *in_audio, string out, char* data) {
	// open file with write-binary mode
	FILE* out_file = fopen(out.c_str(), "wb");

	// check if we opened it sucessfully
	if (out_file != NULL) {
		printf("[*] Writing headers\n");

		// write WAV_HDR headers
		WAV_HDR wav = in_audio->get_wav();
		fwrite(&wav, 1, sizeof(wav), out_file);

		// write DATA_CHUNK headers
		DATA_CHUNK data_chunk = in_audio->get_data_chunk();
		fwrite(&data_chunk, 1, sizeof(data_chunk), out_file);

		printf("[*] Writing data\n");

		// write actual crypted data
		for (int i = 0; i < data_chunk.sub_chunk_size; i++) {
			fwrite(&data[i], 1, sizeof(data[i]), out_file);
		}

		printf("[*] Finished writing to file %s\n", out.c_str());

		// finally, close the file
		fclose(out_file);
	}
	else {
		fprintf(stderr, "[!!] Couldn't open %s, exiting..\n", out.c_str());
		exit(1);
	}
}

// crypt the data using CryptoPP library
void cfb_algo(string in, string out, const byte* key, int opt) {
	printf("[*] %s %s\n", opt == ENC ? "Encrypting": "Decrypting", in.c_str());

	// create PCM object dynamically to avoid double free errors
	PCM *in_audio = new PCM(in);

	// used to generate the iv randomly
	AutoSeededRandomPool rnd;

	// get raw audio data
	char* data = (char*) in_audio->get_data();

	// gen random iv
	byte iv[AES::BLOCKSIZE];
	rnd.GenerateBlock(iv, AES::BLOCKSIZE);

	if (opt == ENC) {
		// use CFB_Mode<AES>::Encryption object to encrypt data
		CFB_Mode<AES>::Encryption cfb(key, AES::DEFAULT_KEYLENGTH, iv, 1);
		cfb.ProcessData((byte*)data, (byte*)data, in_audio->get_data_size());
	}
	else {
		// use CFB_Mode<AES>::Decryption object to decrypt data
		CFB_Mode<AES>::Decryption cfb(key, AES::DEFAULT_KEYLENGTH, iv, 1);
		cfb.ProcessData((byte*)data, (byte*)data, in_audio->get_data_size());
	}

	printf("[*] Finished %s\n", opt == ENC ? "encrypting": "decrypting");

	printf("[*] Writing to file %s\n", out.c_str());

	// finally, write down to the file
	write_audio(in_audio, out, data);
	
	// free in_audio to avoid leaks
	delete in_audio;
}

int main(int argc, char* argv[]) {
	// ensure proper usage
	if (argc < 4 || argc > 5) {
		usage();
		return 1;
	}

	// key to encrypt and decrypt
	byte key[AES::DEFAULT_KEYLENGTH + 1];
	char* key_file;

	char c;

	// get command line arguments
	while ((c = getopt(argc, argv, "ed:")) != -1) {
		switch (c) {
			// in case of -e flag passed
			case 'e':
				ENCRYPT = true;
				if (argc == 5) {
					NO_GEN = true;
					key_file = argv[optind];
					read_key(key_file, key);
				}
				break;

			// in case of -d flag passed
			case 'd':
				// check for proper usage
				if (argc != 5) {
					fprintf(stderr, "In decryption mode, you must give a file with a key of %d bytes long.\n", AES::DEFAULT_KEYLENGTH);
					usage();
					return 1;
				}
				key_file = optarg;
				read_key(key_file, key);
				break;
			// if no flag passed
			default:
				usage();
				return 1;
		}
	}

	string in_file = argv[argc-2];
	string out_file = argv[argc-1];

	// check for file existence
	if (access(in_file.c_str(), F_OK) == -1) {
		fprintf(stderr, "No such file %s\n", in_file.c_str());
		usage();
		return 1;
	}

	if (ENCRYPT) {
		if (!NO_GEN) {
			printf("[*] Generating key\n");
			gen_key(key);

			// write key to file.key
			int counter = 0;
			char key_file_name[12];

			// write the format string to the key_file_name variable
			// until we found a file that doesn't exist yet
			sprintf(key_file_name, "%07d.key", counter);

			while (access(key_file_name, F_OK) != -1) {
				counter++;
				sprintf(key_file_name, "%07d.key", counter);
			}

			printf("[*] Writing key into %s\n", key_file_name);

			// open file with write-binary option
			FILE* out_key_file = fopen(key_file_name, "wb");
			// write the key
			fwrite(key, 1, sizeof(byte) * 16, out_key_file);

			// finally, close the file
			fclose(out_key_file);
		}
		// crypt the file
		cfb_algo(in_file, out_file, key, ENC);
	}
	else {
		// decrypt the file
		cfb_algo(in_file, out_file, key, DEC);
	}
}
