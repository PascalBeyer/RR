#ifndef RR_WORKHANDLER
#define RR_WORKHANDLER

#include "BasicTypes.h"

#define WorkQueueSize 5000

struct Work
{
	void *data;
	void(*callback)(void *data);
};

class WorkHandler
{
public:
	WorkHandler();
	static void Initiate(
		u32(*InterlockedCompareExchange)(u32 volatile *oldValue, u32 newValue, u32 expectedOldValue),
		void(*WakeThreads)(),
		u32(*InterlockedIncrement)(u32 volatile *oldValue),
		u32 treadAmount);

	void PushBack(Work work);
	bool WorkDone();
	void DoAllWork();
	void DoWork();

private:	
	Work *queue;
	u32 volatile workIndex;
	u32 volatile storeIndex;
	u32 volatile completionCount;
	u32 volatile completionGoal;

	static u32(*InterlockedCompareExchange)(u32 volatile *oldValue, u32 newValue, u32 expectedOldValue);
	static u32(*InterlockedIncrement)(u32 volatile *oldValue);
	static void(*WakeThreads)();
	static u32 treadAmount;
	static bool initiated;
};

#endif // !RR_WORKHANDLER

