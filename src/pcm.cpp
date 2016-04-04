#include "pcm.h"

namespace PCM_MD
{

PCM::PCM(std::string file)
{
	FILE* audio = fopen(file.c_str(), "rb");

	if (audio != NULL)
	{
		// read riff chunk header
		fread(&riff, 1, sizeof(RIFF_CHUNK), audio);

		// check for proper file format
		if (!(strcmp((char*)riff.chunk_id, "RIFF") == 0) || !(strcmp((char*)riff.format, "WAVE") == 0))
		{
			throw PCM_exception("[!] Invalid file format");
		}

		// get fmt chunk header
		fread(&fmt, 1, sizeof(FMT_CHUNK), audio);
		// get data metadata
		fread(&data_chunk, 1, sizeof(DATA_CHUNK), audio);

		//data = new BYTE [data_chunk.sub_chunk_size];
		BYTE data[data_chunk.sub_chunk_size];

		// read all data
		for (int i = 0; i < data_chunk.sub_chunk_size; i++)
		{
			fread(&data[i], 1, sizeof(BYTE), audio);
		}

		this->data = data;

		fclose(audio);
	}
	else
	{
		throw PCM_exception("[!] Couldn't open the file");
	}
}

BYTE* PCM::get_data()
{
	return data;
}

RIFF_CHUNK PCM::get_riff()
{
	return riff;
}

FMT_CHUNK PCM::get_fmt()
{
	return fmt;
}

DATA_CHUNK PCM::get_data_chunk()
{
	return data_chunk;
}

DWORD PCM::get_data_size()
{
	return data_chunk.sub_chunk_size;
}

const char* PCM_exception::what() const throw()
{
	return this->message.c_str();
}

} // end of namespace
