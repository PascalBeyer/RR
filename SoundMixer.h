#ifndef RR_SOUNDMIXER
#define RR_SOUNDMIXER

//later put this into the cpp
#include "Sound.h"
#include "buffers.h"
#include "AssetHandler.h"

struct SoundListEntry
{
	float volume0;
	float volume1;
	s32 samplesPlayed;
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

	void Add(float volume0, float volume1, s32 samplesPlayed, AssetId soundId, u32 soundIndex);
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



#endif // !RR_SOUNDMIXER

