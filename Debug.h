#ifndef RR_DEBUG
#define RR_DEBUG

struct timed_block
{
	u64 startCycleCount;
	u32 id;

	timed_block(char * fileName, int lineNumber, char *function)
	{
		startCycleCount = __rdtsc();
	}

	~timed_block()
	{
		u64 endCycleCount = __rdtsc();
		u32 cyclesUsed = (u32)(endCycleCount - startCycleCount);
	}

};

static u32 GetDebugCounterIndex(char *fileName, int lineNumber, char *function)
{
	return 0;
}


#endif
