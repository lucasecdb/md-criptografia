#include "pcm.h"

namespace PCM_MD
{

BYTE riffId[4] = {'R', 'I', 'F', 'F'};
BYTE formatId[4] = {'W', 'A', 'V', 'E'};

PCM::PCM(std::string file)
{
	std::ifstream audio(file.c_str(), std::ios::binary);

	if (audio.is_open())
	{
		// read riff chunk header
		audio.read((char*) &riff, sizeof(RIFF_CHUNK));

		// check for proper file format
		if (!check_riff(riff) || !check_format(riff))
		{
			std::cerr << "Invalid file format.. exiting" << std::endl;
			exit(1);
		}

		// get fmt chunk header
		audio.read((char*) &fmt, sizeof(FMT_CHUNK));
		// get data metadata
		audio.read((char*) &data_chunk, sizeof(DATA_CHUNK));

		data = new BYTE [data_chunk.sub_chunk_size];

		// read all data
		for (int i = 0; i < data_chunk.sub_chunk_size; i++)
		{
			audio.read((char*) &data[i], sizeof(BYTE));
		}

		audio.close();
	}
	else
	{
		std::cerr << "Couldn't open the file.. exiting" << std::endl;
		exit(1);
	}
}

PCM::~PCM()
{
	delete[] data;
}

bool PCM::check_format(RIFF_CHUNK r)
{
	for (int i = 0; i < 4; i++)
		if (riff.format[i] != formatId[i])
			return false;
	return true;
}

bool PCM::check_riff(RIFF_CHUNK r)
{
	for (int i = 0; i < 4; i++)
		if (riff.chunk_id[i] != riffId[i])
			return false;
	return true;
}

BYTE* PCM::get_data()
{
	return data;
}

RIFF_CHUNK PCM::get_riff()
{
	return riff;
}

FMT_CHUNK PCM::get_FMT()
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

} // end of namespace
