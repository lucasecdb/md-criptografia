#include <iostream>
#include <unistd.h>

// PCM library
#include "pcm.h"

// AES crypto library
#include <crypto++/aes.h>
#include <crypto++/modes.h>
#include <crypto++/filters.h>

using namespace std;
using namespace PCM_MD;
using namespace CryptoPP;

bool ENCRYPT = false;

void usage()
{
	printf("usage: ./main [OPTIONS...] [in_file] [out_file]\n");
	printf("\nOptions:\n");
	printf("\t-d Decrypt in_file and put the decryption result in out_file\n");
	printf("\t-e Encrypt in_file and put the encryption result in out_file\n");
}

void decrypt(string in, string out, char *key)
{
}

void encrypt(string in, string out, char *key)
{
}

int main(int argc, char* argv[])
{
	// ensure proper usage
	if (argc != 5)
	{
		usage();
		return 1;
	}

	string in_file = argv[3];
	string out_file = argv[4];

	// check for file existence
	if (access(in_file.c_str(), F_OK) == -1)
	{
		fprintf(stderr, "No such file %s\n", in_file.c_str());
		usage();
		return 1;
	}

	// key to encrypt and decrypt
	char *key;

	// get command line arguments
	char c;
	while ((c = getopt(argc, argv, "e:d:")) != -1)
	{
		switch (c)
		{
			case 'e':
				ENCRYPT = true;
				key = optarg;
				break;
			case 'd':
				key = optarg;
				break;
			default:
				usage();
				return 1;
		}
	}

	if (ENCRYPT)
	{
		encrypt(in_file, out_file, key);
	}
	else
	{
	}
}
