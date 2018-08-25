#ifndef RR_WORKHANDLER
#define RR_WORKHANDLER

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


u32 ICEStub(u32 volatile *oldValue, u32 newValue, u32 expectedOldValue)
{
	return 0;
}
u32 IIStub(u32 volatile *oldValue)
{
	return 0;
}
void WTStub()
{

}

bool WorkHandler::initiated = false;
u32(*WorkHandler::InterlockedCompareExchange)(u32 volatile *oldValue, u32 newValue, u32 expectedOldValue) = &ICEStub;
u32(*WorkHandler::InterlockedIncrement)(u32 volatile *oldValue) = &IIStub;
void(*WorkHandler::WakeThreads)() = &WTStub;
u32 WorkHandler::treadAmount = 0;

WorkHandler::WorkHandler()
{
	if (initiated)
	{
		workIndex = 0;
		storeIndex = 0;
		completionCount = 0;
		completionGoal = 0;
	}
	queue = new Work[WorkQueueSize];
}

void WorkHandler::PushBack(Work work)
{
	queue[storeIndex] = work;
	completionGoal++;
	u32 newStoreIndex = (storeIndex + 1) % WorkQueueSize;
	storeIndex = newStoreIndex;

	WakeThreads();
}

void WorkHandler::DoWork()
{
	u32 workIndexSave = workIndex;
	if (workIndexSave != storeIndex)
	{
		u32 newWorkIndex = (workIndexSave + 1) % WorkQueueSize;
		u32 index = InterlockedCompareExchange(&workIndex, newWorkIndex, workIndexSave);

		if (index == workIndexSave)
		{
			Work *work = queue + index;
			work->callback(work->data);
			InterlockedIncrement(&completionCount);
		}
	}
}

bool WorkHandler::WorkDone()
{
	return completionCount == completionGoal;
}

void WorkHandler::DoAllWork()
{
	while (!WorkDone())
	{
		DoWork();
	}
	completionCount = 0;
	completionGoal = 0;
}

void WorkHandler::Initiate(u32(*InterlockedCompareExchange)(u32 volatile *oldValue, u32 newValue, u32 expectedOldValue), void(*WakeThreads)(), u32(*InterlockedIncrement)(u32 volatile *oldValue), u32 treadAmount)
{
	if (!initiated)
	{
		WorkHandler::InterlockedCompareExchange = InterlockedCompareExchange;
		WorkHandler::WakeThreads = WakeThreads;
		WorkHandler::treadAmount = treadAmount;
		WorkHandler::InterlockedIncrement = InterlockedIncrement;
		initiated = true;
	}
}
#endif // !RR_WORKHANDLER

