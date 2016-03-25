#include "pcm.h"

namespace PCM_MD
{

PCM::PCM(string file)
{
	ifstream audio(file, ios::binary);

	if (audio.is_open())
	{
		// read riff chunk header
		audio.read((char*) &riff, sizeof(RIFF_CHUNK));

		// check for proper file format
		if (!checkRiff(riff) || !checkFormat(riff))
			throw exception(); // invalid file format

		// get fmt chunk header
		audio.read((char*) &fmt, sizeof(FMT_CHUNK));
		// get data metadata
		audio.read((char*) &data, sizeof(DATA_CHUNK));

		data_chunk = new BYTE [data.sub_chunk_size];

		// read all data
		audio.read((char*) &data_chunk, data.sub_chunk_size);

		audio.close();
	}
	else
		throw exception(); // unable to open file
}

PCM::~PCM()
{
	delete[] data_chunk;
}

bool PCM::checkFormat(RIFF_CHUNK r)
{
	return riff.format == formatId;
}

bool PCM::checkRiff(RIFF_CHUNK r)
{
	for (int i = 0; i < 4; i++)
		if (riff.chunk_id[i] != riffId[i])
			return false;

	return true;
}

DWORD PCM::getDataSize()
{
	return data.sub_chunk_size;
}

} // end of namespace
