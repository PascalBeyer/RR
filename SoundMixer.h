#ifndef RR_SOUNDMIXER
#define RR_SOUNDMIXER


//WARNING TO BE TESTED PROBABLY BUGGY AF

struct Sound
{
	i16 *samples[2];
	u32 chanelCount;
	u32 sampleAmount;
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

static Sound CreateSound(char *fileName)
{
	Sound ret;
    
	File tempFile = LoadFile(fileName);
    
	Assert(tempFile.fileSize);
    
	WaveHeader *waveHeader = (WaveHeader *)tempFile.memory;
    
	u8 *it = (u8 *)(waveHeader + 1);
	u8 *end = it + (waveHeader->size - 4);
	while (it < end) {
		WaveChunk *waveChunk = (WaveChunk *)it;
		it += sizeof(WaveChunk);
		switch (waveChunk->id)
		{
            case WaveChunkId_fmt:
            {
                Wavefmt *fmt = (Wavefmt *)it;
                ret.chanelCount = fmt->nChannels; // this has to come before the WaveChunkID_data apperantly
            }break;
            case WaveChunkId_data:
            {
                ret.sampleAmount = waveChunk->size / (ret.chanelCount * sizeof(u16));
                ret.samples[0] = (i16 *)it;
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
    
	if (ret.chanelCount == 2)
	{
		u16 *sourceSamples = (u16 *)ret.samples[0];
		for (u32 index = 0; index < ret.sampleAmount; index++)
		{
			u16 s = sourceSamples[2 * index];
			sourceSamples[2 * index] = sourceSamples[index];
			sourceSamples[index] = s;
		}
	}
}


struct SoundListEntry
{
	float volume0;
	float volume1;
	i32 samplesPlayed;
	Sound sound;
	u32 soundIndex;
	SoundListEntry *next;
	SoundListEntry *prev;
    
	u32 listIndex;
    
};

#define PlayingListSize 256

struct PlayingSoundList
{
	//negative samplesPlayed means wait, then play
	SoundListEntry *first;
	SoundListEntry *list;
	u32 maxListSize;
	bool *empty;
};


struct SoundMixer
{
	PlayingSoundList list;
};

static u32 FirstZeroIndex(bool *boolArray, u32 size)
{
	for (u32 i = 0; i < size; i++)
	{
		if (boolArray[i])
		{
			return i;
		}
	}
	return size;
}

static PlayingSoundList CreatePlayingSoundList(Arena *arena)
{
	PlayingSoundList ret;
	ret.first = NULL;
	ret.list = NULL;
	ret.maxListSize = PlayingListSize;
	ret.empty = PushData(arena, bool, ret.maxListSize);
	return ret;
}

void Add(PlayingSoundList *list, float volume0, float volume1, i32 samplesPlayed, Sound sound, u32 soundIndex)
{
	u32 leastUnfilledIndex = FirstZeroIndex(list->empty, list->maxListSize);
	if (leastUnfilledIndex < list->maxListSize)
	{
		list->empty[leastUnfilledIndex] = false;
        
		SoundListEntry *entry = list->list + leastUnfilledIndex;
		entry->volume0 = volume0;
		entry->volume1 = volume1;
		entry->samplesPlayed = samplesPlayed;
		entry->sound = sound;
		entry->soundIndex = soundIndex;
		entry->next = list->first;
		entry->prev = NULL;
		entry->listIndex = leastUnfilledIndex;
		list->first = entry;
	}
}
//he does this by having 2 lists, one of freed things and one of empty things
void Free(PlayingSoundList *list, SoundListEntry *entry)
{
	Assert(entry);
    
	if (entry == list->first)
	{
		list->first = entry->next;
	}
    
	u32 indexOfNext = entry->listIndex;
	list->empty[indexOfNext] = false;
	SoundListEntry *previous = entry->prev;
	SoundListEntry *next = entry->next;
	if (previous)
	{
		previous->next = next;
	}
	if (next)
	{
		next->prev = previous;
	}
	
}

void PlaySound(SoundMixer *mixer, Sound sound, u32 soundIndex)
{
	Add(&mixer->list, 1.0f, 1.0f, 0, sound, soundIndex);
}

void ToOutput(SoundMixer *mixer, SoundBuffer *buffer, Arena *workingArena)
{
	Clear(workingArena);
    
	float *floatChanel0 = PushData(workingArena, float, buffer->sampleAmount);
	float *floatChanel1 = PushData(workingArena, float, buffer->sampleAmount);
    
	// memset to zero
	{
		float *dest0 = floatChanel0;
		float *dest1 = floatChanel1;
		for (int sampleIt = 0; sampleIt < buffer->sampleAmount; sampleIt++)
		{
			*dest0++ = 0.0f;
			*dest1++ = 0.0f;
		}
	}
    
    
	for (SoundListEntry *it = mixer->list.first; it; it = it->next)
	{
		float volume0 = it->volume0;
		float volume1 = it->volume1;
        
		float *dest0 = floatChanel0;
		float *dest1 = floatChanel1;
        
		u32 samplesAlreadyPlayed = it->samplesPlayed;
        
		Sound sound = it->sound;
        
		u32 samplesToMix = buffer->sampleAmount;
		if (sound.sampleAmount < samplesToMix + samplesAlreadyPlayed)
		{
			samplesToMix = sound.sampleAmount - samplesAlreadyPlayed;
		}
        
		for (u32 sampleIt = 0; sampleIt < samplesToMix; sampleIt++)
		{
			u32 testSoundSampleIndex = (samplesAlreadyPlayed + sampleIt) % sound.sampleAmount;
            
			float sampleValue0 = (float)sound.samples[0][testSoundSampleIndex];
			//float sampleValue1 = (float)sound.samples[1][testSoundSampleIndex];
            
			*dest0++ += volume0 * sampleValue0;
			*dest1++ += volume1 * sampleValue0;
		}
        
		it->samplesPlayed += samplesToMix;
		if ((u32)it->samplesPlayed == sound.sampleAmount)
		{
			Free(&mixer->list, it);
		}
        
	}
    
	{
		float *source0 = floatChanel0;
		float *source1 = floatChanel1;
        
		i16 *sampleOut = buffer->soundSamples;
		for (int sampleIt = 0; sampleIt < buffer->sampleAmount; sampleIt++)
		{
			*sampleOut++ = (i16)(*source0++ + 0.5f);
			*sampleOut++ = (i16)(*source1++ + 0.5f);
		}
	}
}

#endif // !RR_SOUNDMIXER

