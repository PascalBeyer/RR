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

static void LoadDebugVariablesFile() //todo strings?
{
	File *file = PushStruct(globalDebugState.arena, File);
	*file = LoadFile("DebugVariables.txt"); // todo : make own version, we need general purpose allocation
	globalDebugState.tweekerFile = file;
}



#if 0
static String CreateSaneFileName(Char *fileName)
{
	Assert(fileName);
	Char *lastSanePlace = fileName;
	Char *it = fileName;
	while (*it)
	{
		if (*it == '\\')
		{
			while(*++it == '\\');
			lastSanePlace = it;
		}
		it++;
	}
	return CreateString(lastSanePlace);
}
#endif

enum DebugUIElementType
{
	DebugUIType_Graph,
	DebugUItype_Tweekers
};

enum DebugUIElementFlag
{
	DebugUIFlag_Closed = 0x0,
	DebugUIFlag_Open   = 0x1,

};

static void InitDebug()
{
	DebugState *s = &globalDebugState;

	s->tweekers = CreateDynamicArray(s->arena);
	LoadDebugVariablesFile();
	
	s->uiElements = PushArray(s->arena, DebugUIElement, 0);

	//BuildStaticArray(s->arena, s->uiElements, )
}

static void DrawTweekers(RenderGroup *rg, Font font)
{
	Tweekable(f32, tweekerFontSize);
	DebugUITweekerFile *stack = NULL;
	For (globalDebugState.tweekers)
	{
		auto s = stack;
		while (s)
		{
			// this should work as we get these of the string table and never change 'em.
			if (s->function == it->function.data) 
			{
				// todo  maybe lex sorting here if we feel fancy
				auto data = PushStruct(frameArena, TweekerRenderList); 
				data->next = s->first;
				data->tweeker = it;
				s->first = data;
				break;
			}

			s = s->next;
		}
		if (!s)
		{
			auto data = PushStruct(frameArena, DebugUITweekerFile);
			data->function = it->function.data;
			data->next = stack;
			stack = data;
			data->first = PushStruct(frameArena, TweekerRenderList);
			data->first->next = NULL;
			data->first->tweeker = it;
		}
	}

	u32 i = 1;
	while (stack)
	{
		PushString(rg, V2(tweekerFontSize, i++ * 1.1f * tweekerFontSize), (char *)stack->function, tweekerFontSize, font);
		for (auto it = stack->first; it; it = it->next)
		{
			PushString(rg, V2(3.0f * tweekerFontSize, i++ * 1.1f * tweekerFontSize), it->tweeker->name, tweekerFontSize, font);
		}
		stack = stack->next;
	}
	
}

//todo make left bound render for strings
//todo maybe make it so that we can click on a frame and jump to it, by saving the gamestate. has a lot of issues I guess (rendering and such)... but may work
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
	Tweekable(u32, renderCycleThreshhold);
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
	ConsoleOutputError("ExtractTweekerFromFile: Line : %u32. %c*", lineNumber, string);
}


// todo hex
// tweekers are of the form:
// name value
// u32 = 342432
// b32 = true
// v3  = (1, 23.4, 0.1)
// f32 = 1.2
// todo interpret everything we can't parse as strings? I think I want to do ""

// could save lineNumbers and such, but I think saveing will be "rare" so we are not to concerned with speed
static Tweeker ExtractTweekerFromFile(String toFind)
{
	Assert(toFind.length);

	File *file = globalDebugState.tweekerFile;

	char *data = (char *)file->memory;
	String inputString = CreateString(data);

	//Sanitize(&inputString);

	u32 lineNumber = 0;

	while (inputString.length)
	{
		String head = ConsumeNextLine(&inputString); // could have like a eat all spaces version of sanitize and then every thing is a bit easier... thinking.
		
		EatSpaces(&head);

		if (!head.length) { lineNumber++; continue; } 

		String name = EatToNextSpaceReturnHead(&head);

		if (!(name == toFind)) { lineNumber++; continue; }

		if (head[head.length - 1] == '\n') head.length--;

		EatSpaces(&head);

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
				t.name = CopyString(name, globalDebugState.arena);
				return t;
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
				t.name = CopyString(name, globalDebugState.arena);
				return t;
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
			t.name = CopyString(name, globalDebugState.arena);
			return t;
		}
		else
		{
			//non vector
			b32 success = true;
			EatSpacesFromEnd(&head);

			u32 u = StoU(head, &success);
			if (success)
			{
				Tweeker t;
				t.type = Tweeker_u32;
				t.u = u;
				t.name = name;
				t.name = CopyString(name, globalDebugState.arena);
				return t;
			}

			success = true;
			f32 f = StoF(head, &success);
			if (success)
			{
				Tweeker t;
				t.type = Tweeker_f32;
				t.f = f;
				t.name = name;
				t.name = CopyString(name, globalDebugState.arena);
				return t;
			}

			success = true;
			b32 b = StoB(head, &success);
			if (success)
			{
				Tweeker t;
				t.type = Tweeker_b32;
				t.b = b;
				t.name = name;
				t.name = CopyString(name, globalDebugState.arena);
				return t;
			}

			//here string. probably with ""


			ReportLoadDebugVariablesError("Could not read value", lineNumber++);
		}
	}

	Tweeker error;
	error.type = Tweeker_Invalid;
	return error;
}


//todo maybe speed pass for all tweeker stuff, pretty janky for now.
static void *MaybeAddTweekerReturnValue(char *_name, TweekerType type, char *_function) // @scope speedup part3 minute 30
{
	String name = S(_name);
	String function = S(_function);
	Tweeker *toTweek = GetTweeker(name);
	if (!toTweek)
	{
		Tweeker t = ExtractTweekerFromFile(name);
		if (t.type == Tweeker_Invalid)
		{
			t.type = type;
			t.vec4 = V4();
			t.name = name;
			WriteSingleTweeker(t);
		}
		Assert(t.type == type); // right now we can't have multiple tweekers of the same name, and thats fine

		t.name = name;
		t.function = function;

		Tweeker *added = ArrayAdd(&globalDebugState.tweekers, t);
		return (void *)(&added->u);
	}
	return (void *)(&toTweek->u);
}
static void WriteSingleTweeker(Tweeker tweeker) 
{
	File *file = globalDebugState.tweekerFile;

	char *data = (char *)file->memory;
	String inputString = CreateString(data);
	u32 initialLength = inputString.length;

	//Sanitize(&inputString); should allready be sane

	while (inputString.length)
	{
		u32 remainingLength = inputString.length;

		String head = ConsumeNextLine(&inputString); 

		EatSpaces(&head);

		if (!head.length) { continue; }

		String name = EatToNextSpaceReturnHead(&head);

		if (!(name == tweeker.name)) { continue; }

		EatSpaces(&head);

		String toWrite = TweekerToString(tweeker);

		if (toWrite.length < head.length) // there should be endlines after every single one
		{
			u32 headLenghtSave = head.length;
			CopyStringToString(toWrite, &head);
			for (u32 i = head.length; i < headLenghtSave - 1; i++)
			{
				head.data[i] = ' ';
			}
			WriteEntireFile("DebugVariables.txt", *file);
			ConsoleOutput("Saved to file.");
		}
		else
		{

			u32 headLength = initialLength - remainingLength;
			u32 tailLength = inputString.length;

			String toAdd = FormatString("%s %s\n", tweeker.name, TweekerToString(tweeker));

			File newFile;
			newFile.fileSize = (headLength + tailLength + toAdd.length)* sizeof(*toAdd.data);
			newFile.memory = PushData(frameArena, u8, newFile.fileSize);

			// source
			u8 *headMem = (u8 *)file->memory;
			u8 *tailMem = (u8 *)file->memory + (initialLength - inputString.length);

			// dest
			u8 *destHead		= (u8 *)newFile.memory;
			u8 *destInsertLine	= (u8 *)newFile.memory + headLength * sizeof(*toAdd.data);
			u8 *destTail		= (u8 *)newFile.memory + headLength * sizeof(Char) + toAdd.length * sizeof(*toAdd.data);

			memcpy(destHead,		headMem,	headLength * sizeof(Char));
			memcpy(destInsertLine,	toAdd.data, toAdd.length * sizeof(*toAdd.data));
			memcpy(destTail,		tailMem,	tailLength * sizeof(Char));

			WriteEntireFile("DebugVariables.txt", newFile);

			//todo : hack
			FreeFile(*globalDebugState.tweekerFile);
			*globalDebugState.tweekerFile = LoadFile("DebugVariables.txt");

			ConsoleOutput("Saved to file."); // todo  somehow there no failing right now
		}
		return;
	}

	String toAdd = FormatString("%s %s\n", tweeker.name, TweekerToString(tweeker));
	
	File newFile;
	newFile.fileSize = file->fileSize + toAdd.length * sizeof(*toAdd.data);
	newFile.memory = PushData(frameArena, u8, newFile.fileSize);

	memcpy(newFile.memory, file->memory, file->fileSize);
	memcpy((u8 *)newFile.memory + file->fileSize, toAdd.data, toAdd.length * sizeof(*toAdd.data));

	WriteEntireFile("DebugVariables.txt", newFile); 

	//todo : hack
	FreeFile(*globalDebugState.tweekerFile);
	*globalDebugState.tweekerFile = LoadFile("DebugVariables.txt");

	ConsoleOutputError("Saved to file."); // todo  somehow there no failing right now
}

// only for hard writes. i.e kinda depricated....
static void WriteDebugVariables() 
{
	u8 *data = PushData(frameArena, u8, 0);

	// todo : sorting by file and then lex add comments to make it pretty,

	For(globalDebugState.tweekers)
	{
		CopyString(it->name, frameArena);
		S(" ", frameArena);
		TweekerToString(*it, frameArena);
		S(" ", frameArena); //todo  hack, true and false are differantly long. just for altering on the fly, maybe I should make it above so that we replace the line.
		S('\n', frameArena);
	}
	S('\0'); // do not know if I need this, seemed to work without it, but that might just be because i call virtualAlloc... todo: test

	File file = CreateFile(data, (u32)(frameArena->current - data));

	WriteEntireFile("DebugVariables.txt", file);

	//clearing back for the lulz
	frameArena->current = data;
}

// welp this kind of allocator is really terrible I guess (fixedSizePool)
// this one is fine
#if 1
static void TestAllocator(BuddyAllocator *buddyAlloc)
{
	RandomSeries series = { RandomSeed() };
	const u32 sizeOfTest = 100;
	u8 *allocated[sizeOfTest];
	u32 numberOfAllocatedBytes = 0;
	for (u32 i = 0; i < sizeOfTest; i++)
	{
		u32 a = (RandomU32(&series) % MegaBytes(3)) + 1;
		allocated[i] = (u8 *)BuddyAlloc(buddyAlloc, a);
		numberOfAllocatedBytes += a;
	}
#if 1
	for (u32 i = 0; i < 1000000000; i++)
	{
		u32 index = (RandomU32(&series) % sizeOfTest);

		BuddyFree(buddyAlloc, allocated[index]);

		u32 a = (RandomU32(&series) % MegaBytes(3)) + 1;
		allocated[index] = (u8 *)BuddyAlloc(buddyAlloc, a);
	}
#endif
}
#endif 

#endif // !RR_DEBUG_BODY

