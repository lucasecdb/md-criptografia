#ifndef _PCM_H
#define _PCM_H

#include <iostream>
#include <exception>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace PCM_MD {

typedef uint8_t  BYTE;
typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef uint16_t WORD;

typedef struct {
	BYTE chunk_id[4];
	DWORD chunk_size;
	BYTE format[4];
} __attribute__((__packed__))
RIFF_CHUNK;

typedef struct {
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

typedef struct {
	RIFF_CHUNK riff;
	FMT_CHUNK fmt;
} __attribute__((__packed__))
WAV_HDR;

typedef struct {
	BYTE sub_chunk_id[4];
	DWORD sub_chunk_size;
} __attribute__((__packed__))
DATA_CHUNK;

class PCM_exception : public std::exception {
public:
	virtual const char* what() const throw();
	PCM_exception(std::string message) : message(message) {};
	~PCM_exception() throw();

private:
	std::string message;
};

class PCM {
public:
	PCM(std::string file) throw(PCM_exception);
	~PCM();

	DWORD get_data_size();
	DATA_CHUNK get_data_chunk();
	BYTE* get_data();

	WAV_HDR get_wav();

private:
	WAV_HDR wav_hdr;
	DATA_CHUNK data_chunk;
	BYTE *data;
};

} // namespace

#endif // _PCM_H
