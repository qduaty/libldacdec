extern "C" {
#include "miniwav.h"
} // extern "C"
#include <cstdint>
#include <cstdio>
#include <string>

#pragma pack(push, 1)

struct RiffHeader {
    char     chunkId[4] = { 'R','I','F','F' };
    uint32_t chunkSize = 0;
    char     format[4] = { 'W','A','V','E' };
};

struct FmtChunkExtensible {
    char     chunkId[4] = { 'f','m','t',' ' };
    uint32_t chunkSize = 52;
    uint16_t audioFormat = 0xFFFE;   // WAVE_FORMAT_EXTENSIBLE
    uint16_t numChannels = 2;
    uint32_t sampleRate = 0;
    uint32_t byteRate = 0;
    uint16_t blockAlign = 1;
    uint16_t bitsPerSample = 0;
    uint16_t cbSize = 34;

    uint16_t validBitsPerSample = 1024;
    uint32_t channelMask = 0x3;

    // GUID LDAC (vendor-specific)
    uint8_t  subFormat[16] = {
        0x12,0x34,0x56,0x78,
        0x9A,0xBC,
        0xDE,0xF0,
        0x11,0x22,
        0x33,0x44,0x4C,0x44,0x41,0x43 // "LDAC"
    };
	uint8_t padding[12] = { 0 };
	void printAllMembers() {
		fprintf(stderr, "chunkId: %.4s\n", chunkId);
		fprintf(stderr, "chunkSize: %u\n", chunkSize);
		fprintf(stderr, "audioFormat: %u\n", audioFormat);
		fprintf(stderr, "numChannels: %u\n", numChannels);
		fprintf(stderr, "sampleRate: %u\n", sampleRate);
		fprintf(stderr, "byteRate: %u\n", byteRate);
		fprintf(stderr, "blockAlign: %u\n", blockAlign);
		fprintf(stderr, "bitsPerSample: %u\n", bitsPerSample);
		fprintf(stderr, "cbSize: %u\n", cbSize);
		fprintf(stderr, "validBitsPerSample: %u\n", validBitsPerSample);
		fprintf(stderr, "channelMask: 0x%X\n", channelMask);
		fprintf(stderr, "subFormat: ");
		for (int i = 0; i < 16; ++i) {
			fprintf(stderr, "0x%02X,", subFormat[i]);
		}
		fprintf(stderr, "\npadding: ");
		for (int i = 0; i < 12; ++i) {
			fprintf(stderr, "0x%02X,", padding[i]);
		}
		fprintf(stderr, "\n");
	}
};

struct FactChunk {
    char     chunkId[4] = { 'f','a','c','t' };
    uint32_t chunkSize = 12;
    uint32_t sampleLength = 0;
	uint8_t padding[8] = { 0 };
};

struct DataChunkHeader {
    char     chunkId[4] = { 'd','a','t','a' };
    uint32_t chunkSize = 0;
};

#pragma pack(pop)

int writeWavLdacHeader(FILE* f, uint16_t channels, uint32_t sampleRate)
{
    RiffHeader riff;
    FmtChunkExtensible fmt;
    FactChunk fact;
    DataChunkHeader data;

    fmt.numChannels = channels;
	fmt.sampleRate = sampleRate;
    fmt.blockAlign = 448; // not important, ATRAC9 has this number

	fwrite(&riff, sizeof(riff), 1, f);
    fwrite(&fmt, sizeof(fmt), 1, f);
    fwrite(&fact, sizeof(fact), 1, f);
    fwrite(&data, sizeof(data), 1, f);
	return ftell(f);
}

void updateWavLdacHeader(FILE* f, uint32_t sampleRate, uint32_t newTotalSamples, uint32_t newDataSize)
{
    if (!f) return;

	auto byteRate = uint64_t(newDataSize) * sampleRate / newTotalSamples;
	fseek(f, sizeof(RiffHeader) + offsetof(FmtChunkExtensible, byteRate), SEEK_SET);
	fwrite(&byteRate, sizeof(uint32_t), 1, f);

    fseek(f, sizeof(RiffHeader) + sizeof(FmtChunkExtensible) + offsetof(FactChunk, sampleLength), SEEK_SET);
    fwrite(&newTotalSamples, sizeof(uint32_t), 1, f);

    fseek(f, sizeof(RiffHeader) + sizeof(FmtChunkExtensible) + sizeof(FactChunk) + offsetof(DataChunkHeader, chunkSize), SEEK_SET);
    fwrite(&newDataSize, sizeof(uint32_t), 1, f);

	fseek(f, 0, SEEK_END);
	uint32_t riffSize = ftell(f) - offsetof(RiffHeader, format);
    fseek(f, offsetof(RiffHeader, chunkSize), SEEK_SET);
    fwrite(&riffSize, sizeof(uint32_t), 1, f);
}

int parseWavLdac(FILE* f, WavLdacInfo* info)
{
    if (!f || !info) {
		fprintf(stderr, "Invalid file pointer or info pointer\n");
        return -1; 
    }
	auto filePos = ftell(f);
    RiffHeader riff{};
    if (fread(&riff, sizeof(riff), 1, f) != 1) { return -1; }

    if (std::string(riff.chunkId, 4) != "RIFF" ||
        std::string(riff.format, 4) != "WAVE") {
		// fprintf(stderr, "Not a valid WAV file: invalid RIFF header\n");
        return -1;
    }

    FmtChunkExtensible fmt{};
    FactChunk fact{};
    DataChunkHeader data{};
	int allRead = 0;
	size_t dataPos = 0;
    do {
		char chunkId[4];
		fread(chunkId, sizeof(chunkId), 1, f);
		if (!strncmp(chunkId, "fmt ", 4)) {
            fprintf(stderr, "Found fmt chunk\n");
            fseek(f, -4, SEEK_CUR);
            if (fread(&fmt, sizeof(fmt), 1, f) != 1) { return -1; }
            if (fmt.audioFormat != 0xFFFE) { // WAVEFORMATEXTENSIBLE
				fprintf(stderr, "Unsupported audio format: %u\n", fmt.audioFormat);
                return -1;
            }
            allRead++;
			fseek(f, fmt.chunkSize + 8 - sizeof(FmtChunkExtensible), SEEK_CUR);
		}
        else if(!strncmp(chunkId, "fact", 4)) {
			fprintf(stderr, "Found fact chunk\n");
			fseek(f, -4, SEEK_CUR);
            if (fread(&fact, sizeof(fact), 1, f) != 1) { return -1; }
			allRead++;
            fseek(f, fact.chunkSize + 8 - sizeof(FactChunk), SEEK_CUR);
        }
        else if(!strncmp(chunkId, "data", 4)) {
			fprintf(stderr, "Found data chunk\n");
			fseek(f, -4, SEEK_CUR);
            if (fread(&data, sizeof(data), 1, f) != 1) { return -1; }
			allRead++;
            dataPos = ftell(f);
        }
    } while (allRead < 3);

	// fmt.printAllMembers();
	// fprintf(stderr, "dataPos: %u\n", dataPos);

    info->channels = fmt.numChannels;
    info->sampleRate = fmt.sampleRate;
    info->totalSamples = fact.sampleLength;
    info->dataSize = data.chunkSize;
    info->dataOffset = static_cast<uint64_t>(dataPos);

    return 0;
}
