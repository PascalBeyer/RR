#ifndef RR_SOUND
#define RR_SOUND
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


#endif
