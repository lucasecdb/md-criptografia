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
		if (!checkRiff(riff) || !checkFormat(riff))
		{
			std::cerr << "Invalid file format.. exiting" << std::endl;
			exit(1);
		}

		// get fmt chunk header
		audio.read((char*) &fmt, sizeof(FMT_CHUNK));
		// get data metadata
		audio.read((char*) &data, sizeof(DATA_CHUNK));

		data_chunk = new BYTE [data.sub_chunk_size];

		// read all data
		for (int i = 0; i < data.sub_chunk_size; i++)
		{
			audio.read((char*) &data_chunk[i], sizeof(BYTE));
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
	delete[] data_chunk;
}

bool PCM::checkFormat(RIFF_CHUNK r)
{
	for (int i = 0; i < 4; i++)
		if (riff.format[i] != formatId[i])
			return false;
	return true;
}

bool PCM::checkRiff(RIFF_CHUNK r)
{
	for (int i = 0; i < 4; i++)
		if (riff.chunk_id[i] != riffId[i])
			return false;
	return true;
}

BYTE* PCM::getData()
{
	return data_chunk;
}

DWORD PCM::getDataSize()
{
	return data.sub_chunk_size;
}

} // end of namespace
