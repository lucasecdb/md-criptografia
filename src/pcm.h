#ifndef _PCM_H
#define _PCM_H

#include <iostream>
#include <exception>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
	BYTE format[4];
} __attribute__((__packed__))
RIFF_CHUNK;

typedef struct
{
	BYTE sub_chunk_id[4];
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
	BYTE sub_chunk_id[4];
	DWORD sub_chunk_size;
} __attribute__((__packed__))
DATA_CHUNK;

class PCM
{
public:
	PCM(std::string file);

	DWORD get_data_size();
	BYTE* get_data();

	RIFF_CHUNK get_riff();
	FMT_CHUNK get_fmt();
	DATA_CHUNK get_data_chunk();
private:
	RIFF_CHUNK riff;
	FMT_CHUNK fmt;
	DATA_CHUNK data_chunk;
	BYTE *data;
};

class PCM_exception : public std::exception
{
public:
	virtual const char* what() const throw();
	PCM_exception(std::string message) : message(message) {};
	~PCM_exception() throw();

private:
	std::string message;
};

} // end of namespace

#endif
