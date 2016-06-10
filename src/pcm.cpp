#include "pcm.h"

namespace PCM_MD {

PCM_exception::~PCM_exception() throw() {
}

const char* PCM_exception::what() const throw() {
	return this->message.c_str();
}

bool bytecmp(const char* w1, const char* w2, size_t n) {
	for (int i = 0; i < n; i++) {
		if (w1[i] != w2[i])
			return false;
	}

	return true;
}

PCM::PCM(std::string file) throw(PCM_exception) {
	FILE* audio = fopen(file.c_str(), "rb");

	if (audio != NULL) {
		fread(&wav_hdr, 1, sizeof(WAV_HDR), audio);

		if (!bytecmp((char*)wav_hdr.riff.chunk_id, "RIFF", 4) || !bytecmp((char*)wav_hdr.riff.format, "WAVE", 4)) {
			throw PCM_exception("[!] Invalid file format");
		}

		do {
			fread(&data_chunk, 1, sizeof(DATA_CHUNK), audio);
		} while (!bytecmp((char*)data_chunk.sub_chunk_id, "data", 4));

		data = new BYTE[data_chunk.sub_chunk_size];

		fread((void*)data, data_chunk.sub_chunk_size, sizeof(BYTE), audio);

		fclose(audio);
	} else {
		throw PCM_exception("[!] Couldn't open the file");
	}
}

PCM::~PCM() {
	delete[] data;
}

BYTE* PCM::get_data() {
	return data;
}

DATA_CHUNK PCM::get_data_chunk() {
	return data_chunk;
}

DWORD PCM::get_data_size() {
	return data_chunk.sub_chunk_size;
}

WAV_HDR PCM::get_wav() {
	return wav_hdr;
}

} // namespace
