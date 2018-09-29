#ifndef RR_SOUNDMIXER
#define RR_SOUNDMIXER

#include "AssetHandler.h"
#include "Sound.h"

struct SoundListEntry
{
	float volume0;
	float volume1;
	i32 samplesPlayed;
	AssetId soundId;
	u32 soundIndex;
	SoundListEntry *next;
	SoundListEntry *prev;

	u32 listIndex;

};


class PlayingSoundList
{//negative samplesPlayed means wait, then play
public:
	PlayingSoundList();

	void Add(float volume0, float volume1, i32 samplesPlayed, AssetId soundId, u32 soundIndex);
	void Free(SoundListEntry *soundListEntry);

	SoundListEntry *first;
	u32 maxListSize;
	SoundListEntry *list;
	bool *empty;
};


class SoundMixer
{
public:
	SoundMixer();
	SoundMixer(AssetHandler *assetHanlder);
	~SoundMixer();

	void PlaySound(AssetId soundId, u32 soundIndex);
	void ToOutput(SoundBuffer *soundBuffer);

	AssetHandler *assetHanlder;
	PlayingSoundList list;
private:

};


#define PlayingListSize 256

PlayingSoundList::PlayingSoundList()
{
	maxListSize = PlayingListSize;
	list = new SoundListEntry[maxListSize];
	empty = new bool[maxListSize];
	first = NULL;
}
u32 FirstZeroIndex(bool *boolArray, u32 size)
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

void PlayingSoundList::Add(float volume0, float volume1, i32 samplesPlayed, AssetId soundId, u32 soundIndex)
{
	u32 leastUnfilledIndex = FirstZeroIndex(empty, maxListSize);
	if (leastUnfilledIndex < maxListSize)
	{
		empty[leastUnfilledIndex] = false;

		SoundListEntry *entry = list + leastUnfilledIndex;
		entry->volume0 = volume0;
		entry->volume1 = volume1;
		entry->samplesPlayed = samplesPlayed;
		entry->soundId = soundId;
		entry->soundIndex = soundIndex;
		entry->next = first;
		entry->prev = NULL;
		entry->listIndex = leastUnfilledIndex;
		first = entry;
	}
}
//he does this by having 2 lists, one of freed things and one of empty things
void PlayingSoundList::Free(SoundListEntry *entry)
{
	if (entry)
	{
		if (entry == first)
		{
			first = entry->next;
		}
		u32 indexOfNext = entry->listIndex;
		empty[indexOfNext] = false;
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
}


SoundMixer::SoundMixer()
{
}
SoundMixer::SoundMixer(AssetHandler *assetHandler)
{
	this->assetHanlder = assetHandler;
	list = PlayingSoundList();
}


SoundMixer::~SoundMixer()
{
}

void SoundMixer::PlaySound(AssetId soundId, u32 soundIndex)
{
	list.Add(1.0f, 1.0f, 0, soundId, soundIndex);
}

void SoundMixer::ToOutput(SoundBuffer *buffer)
{
	float *floatChanel0 = new float[buffer->sampleAmount];
	float *floatChanel1 = new float[buffer->sampleAmount];

	{
		float *dest0 = floatChanel0;
		float *dest1 = floatChanel1;
		for (int sampleIt = 0; sampleIt < buffer->sampleAmount; sampleIt++)
		{
			*dest0++ = 0.0f;
			*dest1++ = 0.0f;
		}
	}


	for (SoundListEntry *it = list.first; it; it = it->next)
	{
		Sound *sound = assetHanlder->GetSound(it->soundId, it->soundIndex);
		if (sound)
		{
			float volume0 = it->volume0;
			float volume1 = it->volume1;

			float *dest0 = floatChanel0;
			float *dest1 = floatChanel1;

			u32 samplesAlreadyPlayed = it->samplesPlayed;

			u32 samplesToMix = buffer->sampleAmount;
			if (sound->sampleAmount < samplesToMix + samplesAlreadyPlayed)
			{
				samplesToMix = sound->sampleAmount - samplesAlreadyPlayed;
			}

			for (u32 sampleIt = 0; sampleIt < samplesToMix; sampleIt++)
			{
				u32 testSoundSampleIndex = (samplesAlreadyPlayed + sampleIt) % sound->sampleAmount;

				float sampleValue0 = (float)sound->samples[0][testSoundSampleIndex];
				//float sampleValue1 = (float)sound->samples[1][testSoundSampleIndex];

				*dest0++ += volume0 * sampleValue0;
				*dest1++ += volume1 * sampleValue0;
			}

			it->samplesPlayed += samplesToMix;
			if ((u32)it->samplesPlayed == sound->sampleAmount)
			{
				list.Free(it);
			}

		}
	}
	{
		float *source0 = floatChanel0;
		float *source1 = floatChanel1;

		s16 *sampleOut = buffer->soundSamples;
		for (int sampleIt = 0; sampleIt < buffer->sampleAmount; sampleIt++)
		{
			*sampleOut++ = (s16)(*source0++ + 0.5f);
			*sampleOut++ = (s16)(*source1++ + 0.5f);
		}
	}
	delete[] floatChanel0;
	delete[] floatChanel1;
}

#endif // !RR_SOUNDMIXER

