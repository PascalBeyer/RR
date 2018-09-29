#ifndef RR_DEBUG_BODY
#define RR_DEBUG_BODY

struct SplittBlock // rolling buffer of these?
{
	u16 debugInfoIndex;
	u16 hitCount;
	u32 cycles;
	u8 eventTag;
};


#define DEBUG_SPLITT_BLOCK_ARRAY_SIZE (4 * 63)

SplittBlock debugSplittBlocks[DEBUG_SPLITT_BLOCK_ARRAY_SIZE];

struct DebugFrameList
{
	u32 firstIndex;
	u32 lastIndex;
	DebugFrameList *next;
};

//todo collaps everything down into a debug memory kind of a thing

//todo maybe make this fast
inline b32 ModRangeIntersect(u32 first1, u32 last1, u32 first2, u32 last2, u32 mod)
{
	return ((first2 - first1) % mod) <= ((last1 - first1) % mod) || ((first1 - first2) % mod) <= ((last2 - first2) % mod);
}

inline void AdvanceMod(u32 *a, u32 mod) // todo can make fast version of this by using shift up, shift down if mod is a known power of 2
{
	*a = ((*a + 1) % mod);
}

// We want three informations
// 1: how long took a scope (including every child)
// 2: how long took a scope (excluding every child)
// 3: how long took a block (might be between two children of the scope) (more for drawing)

// last one gives the top two, so we will for now only compute that one

//this really does not do much now i guess...
static void CollectDebugRecords()
{
	TimedBlock;
	if (globalDebugState.paused) return;

	// for now we assume there is one "all enclosing" timed block - in this case "WinMain" which times our main loop

	DebugState *s = &globalDebugState;
	if (!s->lastFramesEventIndex) return;

	DebugFrameList *f;
	if (s->frameFree)
	{
		f = s->frameFree;
		s->frameFree = s->frameFree->next;
	}
	else
	{
		f = PushStruct(constantArena, DebugFrameList);
	}
	u32 firstIndex;
	if (s->frameTail)
	{
		firstIndex = (s->frameTail->lastIndex + 1) % DEBUG_SPLITT_BLOCK_ARRAY_SIZE;
	}
	else
	{
		s->frameHead = f;
		s->frameTail = f;
		firstIndex = 0;
	}
	u32 currentEnd = firstIndex;

	f->firstIndex = firstIndex;
	s->frameTail->next = f;
	s->frameTail = f;
	f->next = NULL;

	// last frames stuff
	u32 eventArrayIndex = !s->eventArrayIndex;
	u32 amountOfEventsToHandle = s->lastFramesEventIndex;

	//fill new frame into rolling buffer
	DebugEvent *firstEvent = globalDebugState.events[eventArrayIndex];

	Assert(firstEvent->type == DebugEvent_BeginTimedBlock);

	SplittBlock *firstBlock = debugSplittBlocks + firstIndex;

	firstBlock->debugInfoIndex = firstEvent->debugRecordIndex;
	firstBlock->cycles = firstEvent->cycles;
	firstBlock->hitCount = firstEvent->hitCount;
	firstBlock->eventTag = (u8)firstEvent->type;

	for (u32 i = 1; i < amountOfEventsToHandle - 1; i++)
	{
		DebugEvent *e = globalDebugState.events[eventArrayIndex] + i;

		switch (e->type)
		{
			//todo maybe ignore blocks that are to short?
		case DebugEvent_BeginTimedBlock:
		{
			SplittBlock *lastBlock = debugSplittBlocks + currentEnd;
			lastBlock->cycles = e->cycles - lastBlock->cycles;

			AdvanceMod(&currentEnd, DEBUG_SPLITT_BLOCK_ARRAY_SIZE);

			SplittBlock *newBlock = debugSplittBlocks + currentEnd;

			newBlock->debugInfoIndex = e->debugRecordIndex;
			newBlock->cycles = e->cycles;
			newBlock->hitCount = e->hitCount;
			newBlock->eventTag = (u8)e->type;

		}break;
		case DebugEvent_EndTimedBlock:
		{

			SplittBlock *lastBlock = debugSplittBlocks + currentEnd;
			lastBlock->cycles = e->cycles - lastBlock->cycles;

			AdvanceMod(&currentEnd, DEBUG_SPLITT_BLOCK_ARRAY_SIZE);

			SplittBlock *newBlock = debugSplittBlocks + currentEnd;

			newBlock->debugInfoIndex = lastBlock->debugInfoIndex;
			newBlock->cycles = e->cycles;
			newBlock->hitCount = lastBlock->hitCount;
			newBlock->eventTag = (u8)e->type;

		}break;

		default:
		{
			ConsoleOutput("Unknown DebugEvent Received!");
		}break;
		}
	}

	DebugEvent *lastEvent = globalDebugState.events[eventArrayIndex] + (amountOfEventsToHandle - 1);

	Assert(lastEvent->type == DebugEvent_EndTimedBlock);
	b32 sad = strcmp(debugInfoArray[lastEvent->debugRecordIndex].function, "WinMain");
	Assert(!sad);

	SplittBlock *lastBlock = debugSplittBlocks + currentEnd;
	lastBlock->cycles = lastEvent->cycles - lastBlock->cycles;

	f->lastIndex = currentEnd;

	while (s->frameHead != f)
	{
		DebugFrameList *toCheck = s->frameHead;
		if (!ModRangeIntersect(f->firstIndex, f->lastIndex, toCheck->firstIndex, toCheck->lastIndex, DEBUG_SPLITT_BLOCK_ARRAY_SIZE))
		{
			break;
		}

		s->frameHead = s->frameHead->next;

		toCheck->next = s->frameFree;
		s->frameFree = toCheck;
	}
}


//todo make left bound render for strings

static void DrawDebugRecords(RenderGroup *rg, Font font, f32 secondsPerFrame, Input input)
{
	TimedBlock;

	DebugState *s = &globalDebugState;
	// last frames stuff
	u32 eventArrayIndex = !globalDebugState.eventArrayIndex;
	u32 amountOfEventsToHandle = globalDebugState.lastFramesEventIndex;

	u32 maxFramesDisplayed = 30;

	//todo  make this render from the other side, and probably vertically
	u32 j = 1;
	Tweekable(renderCycleThreshhold, u32);
	for (DebugFrameList *it = s->frameHead; it; it = it->next, j++)
	{
		
		f32 currentHeight = 0.0f;

		Assert(debugSplittBlocks[it->firstIndex].eventTag == DebugEvent_BeginTimedBlock /*&& debugSplittBlocks[it->firstIndex].debugInfoIndex*/);

		for (u32 i = it->firstIndex; ; AdvanceMod(&i, DEBUG_SPLITT_BLOCK_ARRAY_SIZE))
		{
			SplittBlock block = debugSplittBlocks[i];

			if (block.cycles > renderCycleThreshhold)
			{
				DebugBlockInfo *info = debugInfoArray + block.debugInfoIndex;

				f32 barSize = 200.0f * (f32)block.cycles / 60000000.0f;

				if (!info->color)
				{
					RandomSeries series = { RandomSeed() };
					info->color = RandomColorU32(&series);
				}

				u32 color = info->color;

				PushRectangle(rg, V2(20 * j, 100.0f + currentHeight), (f32)10, barSize, Unpack4x8(color));

				if (PointInRectangle(V2(20 * j, 100.0f + currentHeight), (f32)10, barSize, input.mousePos))
				{
					String name = FormatString("%c*: %u32 cy", info->function, block.cycles);
					f32 actualFloatWidth = PushString(rg, input.mousePos, name, 20.0f, font);
					PushRectangle(rg, input.mousePos, actualFloatWidth, 20.0f, 0.05f, V4(0.9f, 0.0f, 0.0f, 0.0f));
				}

				currentHeight += barSize;
			}
			if (i == it->lastIndex) break;
		}

		if (j > maxFramesDisplayed) break;
	}
	PushLine(rg, V2(20, 300.0f), V2(20 * j, 300.0f));
}


static void ReportLoadDebugVariablesError(char *string, u32 lineNumber)
{
	ConsoleOutputError("LoadDebugVariables: Line : %u32. %c*", lineNumber, string);
}

//todo maybe speed pass for all tweeker stuff, pretty janky for now.
//todo add a save tweeker value, so we can save after initial frame.
static void *MaybeAddTweekerReturnValue(char *_name, TweekerType type) 
{
	String name = S(_name);
	Tweeker *toTweek = GetTweeker(name);
	if (!toTweek)
	{
		Tweeker t;
		t.name = name; // I don't think we have to copy, as it is in the string table
		t.type = type;
		t.vec4 = V4(); // clear
		Tweeker *added = ArrayAdd(&globalDebugState.tweekers, t);
		return (void *)(&added->u);
	}
	return (void *)(&toTweek->u);
}

// name value
// interpret everything we can't parse as strings?
static void LoadDebugVariables() //todo strings?
{
	File file = LoadFile("DebugVariables.txt");
	char *data = (char *)file.memory;
	String inputString = CreateString(data);

	u32 lineNumber = 0;

	while (inputString.length)
	{
		String head = ConsumeNextLine(&inputString); // could have like a eat all spaces version of sanitize and then every thing is a bit easier... thinking.
		Sanitize(&head);
		EatSpaces(&head);

		if (!head.length) { lineNumber++; continue; }

		//ConsoleOutput(head);

		String name = CopyString(EatToNextSpaceReturnHead(&head), globalDebugState.arena);

		EatSpaces(&head);
		Sanitize(&head);

		if (!head.length)
		{
			ReportLoadDebugVariablesError("Value not found!", lineNumber++);
			continue;
		}
		
		if (head[0] == '(') //vector
		{
			Eat1(&head);
			EatSpaces(&head);

			String val1 = EatToCharReturnHead(&head, ','); // for now it has to be 1.0, not 1.0  ,

			b32 success = true;
			f32 a1 = StoF(val1, &success);
			
			if (!head.length)
			{
				ReportLoadDebugVariablesError("Expected ',' after first value!", lineNumber++);
				continue;
			}

			Eat1(&head);
			EatSpaces(&head);
			String val2 = EatToCharReturnHead(&head, ',', ')');
			f32 a2 = StoF(val2, &success);

			if (!head.length)
			{
				ReportLoadDebugVariablesError("Expected ',' or ')' after second value!", lineNumber++);
				continue;
			}

			if (head[0] == ')')
			{
				if (!success)
				{
					ReportLoadDebugVariablesError("Error reading values of v2!", lineNumber++);
					continue;
				}

				Tweeker t;
				t.type = Tweeker_v2;
				t.vec2 = V2(a1, a2);
				t.name = name;
				ArrayAdd(&globalDebugState.tweekers, t);
				lineNumber++; continue;
			}

			Eat1(&head);
			EatSpaces(&head);
			String val3 = EatToCharReturnHead(&head, ',', ')');
			f32 a3 = StoF(val3, &success);

			if (!head.length || !((head[0] == ',') | (head[0] == ')')))
			{
				ReportLoadDebugVariablesError("Expected ',' or ')' after first value!", lineNumber++);
				continue;
			}

			if (head[0] == ')')
			{
				if (!success)
				{
					ReportLoadDebugVariablesError("Error reading values of v3!", lineNumber++);
					continue;
				}

				Tweeker t;
				t.type = Tweeker_v3;
				t.vec3 = V3(a1, a2, a3);
				t.name = name;
				ArrayAdd(&globalDebugState.tweekers, t); 
				lineNumber++; continue;
			}

			Eat1(&head);
			EatSpaces(&head);
			String val4 = EatToCharReturnHead(&head, ')');
			f32 a4 = StoF(val4, &success);

			if (!head.length)
			{
				ReportLoadDebugVariablesError("Expected ',' or ')' after first value!", lineNumber++);
				continue;
			}

			if (!success)
			{
				ReportLoadDebugVariablesError("Error reading values of v4!", lineNumber++);
				continue;
			}

			Tweeker t;
			t.type = Tweeker_v4;
			t.vec4 = V4(a1, a2, a3, a4);
			t.name = name;
			ArrayAdd(&globalDebugState.tweekers, t);
			lineNumber++; continue;

		}
		else
		{
			//non vector
			b32 success = true;
			u32 u = StoU(head, &success);
			if (success)
			{
				Tweeker t;
				t.type = Tweeker_u32;
				t.u = u;
				t.name = name;
				ArrayAdd(&globalDebugState.tweekers, t);
				lineNumber++; continue;
			}

			success = true;
			f32 f = StoF(head, &success);
			if (success)
			{
				Tweeker t;
				t.type = Tweeker_f32;
				t.f = f;
				t.name = name;
				ArrayAdd(&globalDebugState.tweekers, t);
				lineNumber++; continue;
			}

			success = true;
			b32 b = StoB(head, &success);
			if (success)
			{
				Tweeker t;
				t.type = Tweeker_b32;
				t.b = b;
				t.name = name;
				ArrayAdd(&globalDebugState.tweekers, t);
				lineNumber++; continue;
			}

			ReportLoadDebugVariablesError("Could not read value", lineNumber++);
		}

	}
	
	FreeFile(file);
}

static void WriteDebugVariables()
{
	u8 *data = PushArray(frameArena, u8, 0);

	For(globalDebugState.tweekers)
	{
		CopyString(it->name, frameArena);
		S(" ", frameArena);
		TweekerToString(*it, frameArena);
		S('\n', frameArena);
	}

	File file = CreateFile(data, (u32)(frameArena->current - data));

	WriteEntireFile("DebugVariables.txt", file);

	//clearing back for the lulz
	frameArena->current = data;
}


#endif // !RR_DEBUG_BODY

