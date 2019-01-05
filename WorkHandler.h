#ifndef RR_WORKHANDLER
#define RR_WORKHANDLER

#define WorkQueueSize 5000

struct Work
{
	void *data;
	void(*callback)(void *data);
};

struct WorkHandler
{
    Work *queue;
	i64 volatile workIndex;
	i64 volatile storeIndex;
	i64 volatile completionCount;
	i64 volatile completionGoal;
};

static i64 AtomicCompareExchange(i64 volatile *oldValue, i64 newValue, i64 expectedOldValue);
static i64 AtomicIncrement(i64 volatile *oldValue);
static void WakeThreads();


static void PushBack(WorkHandler *handler, Work work)
{
	handler->queue[handler->storeIndex] = work;
	handler->completionGoal++;
	u32 newStoreIndex = (handler->storeIndex + 1) % WorkQueueSize;
	handler->storeIndex = newStoreIndex;
    
	WakeThreads();
}

static void DoWork(WorkHandler *handler)
{
	i64 workIndexSave = handler->workIndex;
	if (workIndexSave != handler->storeIndex)
	{
        i64 newWorkIndex = (workIndexSave + 1) % WorkQueueSize;
        i64 index = AtomicCompareExchange(&handler->workIndex, newWorkIndex, workIndexSave);
        
		if (index == workIndexSave)
		{
			Work *work = handler->queue + index;
			work->callback(work->data);
			AtomicIncrement(&handler->completionCount);
		}
	}
}

bool WorkDone(WorkHandler *handler)
{
	return (handler->completionCount == handler->completionGoal);
}

void DoAllWork(WorkHandler *handler)
{
	while (!WorkDone(handler))
	{
		DoWork(handler);
	}
	handler->completionCount = 0;
	handler->completionGoal = 0;
}

#endif // !RR_WORKHANDLER

