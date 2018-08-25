#ifndef RR_SOUND
#define RR_SOUND

#include "File.h"
#include "BasicTypes.h"

class Sound
{
public:
	Sound();
	Sound(char *filename);
	Sound(void *samples, u32 sampleAmount);
	~Sound();

	s16 *samples[2];
	u32 chanelCount;
	u32 sampleAmount;
private:
	
};


#define RIFF_CODE(a,b,c,d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

#pragma pack(push, 1)
struct WaveHeader
{
	u32 riffId;
	u32 size;
	u32 waveId;
};
enum
{
	WaveChunkId_fmt = RIFF_CODE('f', 'm', 't', ' '),
	WaveChunkId_data = RIFF_CODE('d', 'a', 't', 'a'),
	WaveChunkId_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
	WaveChunkId_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
};
struct  WaveChunk
{
	u32 id;
	u32 size;
};
struct Wavefmt
{
	u16 wFormatTag;
	u16 nChannels;
	u32 nSamplesPerSec;
	u32 nAvgBytesPerSec;
	u16 nBlockAlign;
	u16 wBitsPerSample;
	u16 cbSize;
	u16 wValidBitsPerSample;
	u32 dwChannelMask;
	u8 subFormat[16];
};
#pragma pack(pop)
Sound::Sound()
{
	samples[0] = NULL;
	samples[1] = NULL;
	sampleAmount = 0;
}
Sound::Sound(char *fileName)
{
	File tempFile = LoadFile(fileName);
	if (tempFile.fileSize > 0)
	{
		WaveHeader *waveHeader = (WaveHeader *)tempFile.memory;

		u8 *it = (u8 *)(waveHeader + 1);
		u8 *end = it + (waveHeader->size - 4);
		while (it < end) {
			WaveChunk *waveChunk = (WaveChunk *)it;
			it += sizeof(WaveChunk);
			switch (waveChunk->id)
			{
			case WaveChunkId_data:
			{
				sampleAmount = waveChunk->size / (chanelCount * sizeof(u16));
				samples[0] = (s16 *)it;
			}break;
			case WaveChunkId_fmt:
			{
				Wavefmt *fmt = (Wavefmt *)it;
				chanelCount = fmt->nChannels;
				int x = 0;
			}break;
			case WaveChunkId_RIFF:
			{

			}break;
			case WaveChunkId_WAVE:
			{

			}break;
			default:
				break;
			}
			u32 chunkSize = (waveChunk->size + 1) & ~1;
			it += chunkSize;
		}

		if (chanelCount == 2)
		{
			u16 *sourceSamples = (u16 *)samples[0];
			for (u32 index = 0; index < sampleAmount; index++)
			{
				u16 randomSamples = sourceSamples[2 * index];
				sourceSamples[2 * index] = sourceSamples[index];
				sourceSamples[index] = randomSamples;
			}
		}
	}
}
Sound::~Sound()
{

}


#endif
