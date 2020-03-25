/*
 * @Author: Bingo
 * @Date: 2020-03-25 11:28:34
 * @LastEditTime: 2020-03-25 14:21:17
 * @Description: 
 *  The canonical WAVE format starts with the RIFF header:
 *  0         4   ChunkID          Contains the letters "RIFF" in ASCII form
 *                                 (0x52494646 big-endian form).
 *  4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
 *                                 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
 *                                 This is the size of the rest of the chunk 
 *                                 following this number.  This is the size of the 
 *                                 entire file in bytes minus 8 bytes for the
 *                                 two fields not included in this count:
 *                                 ChunkID and ChunkSize.
 *  8         4   Format           Contains the letters "WAVE"
 *                                 (0x57415645 big-endian form).
 *    
 *  The "WAVE" format consists of two subchunks: "fmt " and "data":
 *  The "fmt " subchunk describes the sound data's format:
 *  
 *  12        4   Subchunk1ID      Contains the letters "fmt "
 *                                 (0x666d7420 big-endian form).
 *  16        4   Subchunk1Size    16 for PCM.  This is the size of the
 *                                 rest of the Subchunk which follows this number.
 *  20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
 *                                 Values other than 1 indicate some 
 *                                 form of compression.
 *  22        2   NumChannels      Mono = 1, Stereo = 2, etc.
 *  24        4   SampleRate       8000, 44100, etc.
 *  28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
 *  32        2   BlockAlign       == NumChannels * BitsPerSample/8
 *                                 The number of bytes for one sample including
 *                                 all channels. I wonder what happens when
 *                                 this number isn't an integer?
 *  34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
 *            2   ExtraParamSize   if PCM, then doesn't exist
 *            X   ExtraParams      space for extra parameters
 *  
 *  The "data" subchunk contains the size of the data and the actual sound:
 *  
 *  36        4   Subchunk2ID      Contains the letters "data"
 *                                 (0x64617461 big-endian form).
 *  40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
 *                                 This is the number of bytes in the data.
 *                                 You can also think of this as the size
 *                                 of the read of the subchunk following this 
 *                                 number.
 *  44        *   Data             The actual sound data.
 * @FilePath: /pcm2wav/c/main.c
 */
#include <stdio.h>
#include <stdint.h>

typedef unsigned char ID[4];

typedef struct
{
  ID          chunkID;  /* {'f', 'm', 't', ' '} */
  uint32_t    chunkSize;

  uint16_t    audioFormat;
  uint16_t    numChannels;
  uint32_t    sampleRate;
  uint32_t    byteRate;
  uint16_t    blockAlign;
  uint16_t    bitsPerSample;
} FormatChunk;

typedef struct
{
  ID             chunkID;  /* {'d', 'a', 't', 'a'}  */
  uint32_t       chunkSize;
  unsigned char  data[];
} DataChunk;

void usage(char *command)
{
  printf("usage:\n"
         "\t%s pcmfile wavfile channel samplerate bitspersample\n", command);
}

int main(int argc, char *argv[])
{
  FILE *pcmfile, *wavfile;
  uint32_t  pcmfile_size, chunk_size;
  FormatChunk formatchunk;
  DataChunk   datachunk;
  int i, read_len;
  char buf[1024];

  if (argc != 6) {
    usage(argv[0]);
    return 1;
  }
 
  pcmfile = fopen(argv[1], "rb");
  if (pcmfile == NULL) {
    printf("!Error: Can't open pcmfile.\n");
    return 1;
  }
  fseek(pcmfile, 0, SEEK_END);
  pcmfile_size = ftell(pcmfile);
  fseek(pcmfile, 0, SEEK_SET);

  wavfile = fopen(argv[2], "wb");
  if (wavfile == NULL) {
    printf("!Error: Can't create wavfile.\n");
    return 1;
  }

  fwrite("RIFF", 1, 4, wavfile);
  fwrite("xxxx", 1, 4, wavfile);  //reserved for the total chunk size
  fwrite("WAVE", 1, 4, wavfile);

  formatchunk.chunkID[0] = 'f';
  formatchunk.chunkID[1] = 'm';
  formatchunk.chunkID[2] = 't';
  formatchunk.chunkID[3] = ' ';
  formatchunk.chunkSize  = 16;
  formatchunk.audioFormat = 1;
  formatchunk.numChannels = atoi(argv[3]);
  formatchunk.sampleRate = atoi(argv[4]);
  formatchunk.bitsPerSample = atoi(argv[5]);
  formatchunk.byteRate = formatchunk.sampleRate * formatchunk.numChannels * (formatchunk.bitsPerSample >> 3);
  formatchunk.blockAlign = formatchunk.numChannels * (formatchunk.bitsPerSample >> 3);
  fwrite(&formatchunk, 1, sizeof(formatchunk), wavfile);

  datachunk.chunkID[0] = 'd';
  datachunk.chunkID[1] = 'a';
  datachunk.chunkID[2] = 't';
  datachunk.chunkID[3] = 'a';
  datachunk.chunkSize = pcmfile_size;
  fwrite(&datachunk, 1, sizeof(ID)+sizeof(uint32_t), wavfile);

  while((read_len = fread(buf, 1, sizeof(buf), pcmfile)) != 0) {
    fwrite(buf, 1, read_len, wavfile);
  }

  fseek(wavfile, 4, SEEK_SET);
  chunk_size = 4 + (8 + formatchunk.chunkSize) + (8 + datachunk.chunkSize);
  fwrite(&chunk_size, 1, 4, wavfile);

  fclose(pcmfile);
  fclose(wavfile);
}
