#include "pcm.h"

namespace PCM_MD {

// PCM_exception destructor
PCM_exception::~PCM_exception() throw() {
}

// PCM_exception what() function (Equivalent to Java's Exception e.getMessage())
const char* PCM_exception::what() const throw() {
	return this->message.c_str();
}

/**
 * Compares the first n bytes of each string
 */
bool bytecmp(const char* w1, const char* w2, size_t n) {
	for (int i = 0; i < n; i++) {
		if (w1[i] != w2[i])
			return false;
	}

	return true;
}

/**
 * Constructor for class PCM
 */
PCM::PCM(std::string file) throw(PCM_exception) {
	// open the audio with read-binary option
	FILE* audio = fopen(file.c_str(), "rb");

	// if we sucessfully opened it
	if (audio != NULL) {
		// read wav chunk header
		fread(&wav_hdr, 1, sizeof(WAV_HDR), audio);

		// check for proper file format
		if (!bytecmp((char*)wav_hdr.riff.chunk_id, "RIFF", 4) || !bytecmp((char*)wav_hdr.riff.format, "WAVE", 4)) {
			throw PCM_exception("[!] Invalid file format");
		}

		// skip any chunk that comes before the data chunk
		do {
			fread(&data_chunk, 1, sizeof(DATA_CHUNK), audio);
		}
		while (!bytecmp((char*)data_chunk.sub_chunk_id, "data", 4));

		// allocate enough memory for our data in the audio
		data = new BYTE[data_chunk.sub_chunk_size];

		// read all data
		fread((void*)data, data_chunk.sub_chunk_size, sizeof(BYTE), audio);

		// finally, close the file
		fclose(audio);
	}
	else {
		throw PCM_exception("[!] Couldn't open the file");
	}
}

// PCM object destructor
PCM::~PCM() {
	// free data allocated in constructor
	delete[] data;
}

/**
 * Return byte array of raw WAV data
 */
BYTE* PCM::get_data() {
	return data;
}

/**
 * Return DATA_CHUNK struct from file
 */
DATA_CHUNK PCM::get_data_chunk() {
	return data_chunk;
}

/**
 * Return size of the data in file
 */
DWORD PCM::get_data_size() {
	return data_chunk.sub_chunk_size;
}

/**
 * Return WAV_HDR struct from file
 */
WAV_HDR PCM::get_wav() {
	return wav_hdr;
}

} // end of namespace
