#ifndef _PCM_H
#define _PCM_H

#include <fstream>
#include <exception>
#include <stdint.h>

using namespace std;

namespace PCM_MD
{

/* Data types adapted from Microsoft */
typedef uint8_t  BYTE;  // 1 byte
typedef uint32_t DWORD; // 4 bytes (unsigned)
typedef int32_t  LONG;  // 4 bytes
typedef uint16_t WORD;	// 2 bytes (unsigned)

typedef struct
{
	BYTE chunk_id[4];
	DWORD chunk_size;
	DWORD format;
} __attribute__((__packed__))
RIFF_CHUNK;

typedef struct
{
	DWORD sub_chunk_id;
	DWORD sub_chunk_size;
	WORD audio_format;
	WORD num_channels;
	DWORD sample_rate;
	DWORD byte_rate;
	WORD block_align;
	WORD bits_per_sample;
} __attribute__((__packed__))
FMT_CHUNK;

typedef struct
{
	DWORD sub_chunk_id;
	DWORD sub_chunk_size;
} __attribute__((__packed__))
DATA_CHUNK;

BYTE riffId[] = {'R', 'I', 'F', 'F'};

class PCM
{
public:
	PCM(char* file);
	~PCM();

	int getDataSize();
private:
	bool checkRiff(RIFF_CHUNK r);

	RIFF_CHUNK riff;
	FMT_CHUNK fmt;
	DATA_CHUNK data;
};

} // end of namespace

#endif
