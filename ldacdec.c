#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <string.h>


#include "ldacdec.h"

#include "sndfile.h"
#include "miniwav.h"

SNDFILE *openAudioFile( const char *fileName, int freq, int channels, int floatOutput )
{
    SF_INFO sfinfo = { 
        .samplerate = freq, 
        .channels = channels,
        .format = SF_FORMAT_WAV | (floatOutput ? SF_FORMAT_FLOAT : SF_FORMAT_PCM_16),
    };
    fprintf(stderr, "opening \"%s\" for writing ...\n", fileName );
    fprintf(stderr, "sampling frequency: %d\n", freq );
    fprintf(stderr, "channel count:      %d\n", channels );
    SNDFILE *out = sf_open( fileName, SFM_WRITE, &sfinfo );
    if( out == NULL )
    {
        fprintf(stderr, "can't open output: %s\n", sf_strerror( NULL ) );
        return NULL;
    }
    return out;
}


#define BUFFER_SIZE (680*2)
#define PCM_BUFFER_SIZE (256*2)

int main(int argc, char * args[] )
{
    if( argc < 2 )
    {
        fprintf(stderr, "usage:\n\t%s [-float] [input] [output]\n", args[0] );
        return EXIT_SUCCESS;
    }

	int floatOutput = 0;
	if (argc > 1 && strcmp(args[1], "-float") == 0)
	{
		floatOutput = 1;
		argc--;
        args++;
	}

    const char *inputFile = args[1];
    const char *audioFile = "output.wav";

    if( argc > 2 )
        audioFile = args[2];

    fprintf(stderr, "opening \"%s\" ...\n", inputFile );

    ldacdec_t dec;
    ldacdecInit( &dec );

    FILE *in = strcmp(inputFile, "-") == 0 ? stdin : fopen( inputFile, "rb" );
    if( in == NULL )
    {
        perror("can't open stream file");
        return EXIT_FAILURE;
    }

    SNDFILE *out = NULL;

    uint8_t buf[BUFFER_SIZE];
    uint8_t *ptr = NULL;
    int16_t pcm16[PCM_BUFFER_SIZE] = { 0 };
    float pcmf[PCM_BUFFER_SIZE] = { 0 };
	void* pcm = floatOutput ? (void*)pcmf : (void*)pcm16;
    size_t filePosition = 0;
    int blockId = 0;
    int bytesInBuffer = 0;

	WavLdacInfo info = { 0 };
    if (parseWavLdac(in, &info) == 0) {
        filePosition = info.dataOffset;
    }
    while(1)
    {
        LOG("%ld =>\n", filePosition );
        int ret = fseek( in, filePosition, SEEK_SET );
        if( ret < 0 )
            break;
        bytesInBuffer = fread( buf, 1, BUFFER_SIZE, in );
        ptr = buf;
        if( bytesInBuffer == 0 )
            break;
#if 1
        if(ptr[1] == 0xAA) 
        {
            ptr++;
            filePosition++;
        }
#endif        
        LOG("sync: %02x\n", ptr[0] );
        int bytesUsed = 0;
      
        memset( pcm, 0, sizeof(pcm) );
        LOG("count === %4d ===\n", blockId++ );
        ret = ldacDecode( &dec, ptr, pcm, &bytesUsed, floatOutput ? SAMPLE_TYPE_FLOAT : SAMPLE_TYPE_SHORT );
        if( ret < 0 )
            break;
        LOG_ARRAY( pcm, "%4d, " );
        if( out == NULL )
        {
            fprintf(stderr, "auto detect format!\n");
            out = openAudioFile( audioFile, ldacdecGetSampleRate( &dec ), ldacdecGetChannelCount( &dec ), floatOutput );
            if( out == NULL )
                return EXIT_FAILURE;
        }

        if (floatOutput)
            sf_writef_float( out, pcm, dec.frame.frameSamples );
        else
            sf_writef_short( out, pcm, dec.frame.frameSamples );
        filePosition += bytesUsed;
    }

    fprintf(stderr, "done.\n");

    sf_close( out );
    fclose(in);

    return EXIT_SUCCESS;
}
