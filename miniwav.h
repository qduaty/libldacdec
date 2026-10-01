#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint16_t channels;
    uint32_t sampleRate;
    uint32_t totalSamples;
    uint32_t dataSize;
    uint64_t dataOffset; // pozycja danych LDAC w pliku
} WavLdacInfo;

int writeWavLdacHeader(FILE* f, uint16_t channels, uint32_t sampleRate);
void updateWavLdacHeader(FILE* f, uint32_t sampleRate, uint32_t newTotalSamples, uint32_t newDataSize);
int parseWavLdac(FILE* f, WavLdacInfo* info);
