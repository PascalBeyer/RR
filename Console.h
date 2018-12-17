#ifndef RR_CONSOLE
#define RR_CONSOLE

enum ConsoleSelectTag
{
	SelectTag_Selecting = 0x1,
	SelectTag_TextField = 0x2,
	SelectTag_History = 0x4,
};

static v4 ColorForHistoryEntry(HistoryEntryEnum flag)
{
	Tweekable(v4, consoleCommandTextColor);
	Tweekable(v4, consoleErrorTextColor);
	Tweekable(v4, consoleDefaultTextColor);
	

	switch (flag)
	{
	case HistoryEntry_Command:
	{
		return consoleCommandTextColor;
	}break;
	case HistoryEntry_Error:
	{
		return consoleErrorTextColor;
	}break;
	default:
	{
		return consoleDefaultTextColor;
	}break;

	}
}

static ConsoleCommand CreateCommand(char* name, void (*interp)(StringArray args), u32 minArgs, u32 maxArgs)
{
	ConsoleCommand ret;
	ret.name = CreateString(name);
	ret.interp = interp;
	ret.minArgs = minArgs;
	ret.maxArgs = maxArgs;
	return ret;
}

static void InitConsole(Arena *constantArena)
{
	console.buffer = PushData(constantArena, Char, console.bufferSize);
	console.arena = InitArena(console.buffer, console.bufferSize * sizeof(Char));
	console.history = PushData(constantArena, HistoryEntry, console.maxHistoryLength);
	console.commandHistory = PushData(constantArena, String, console.maxCommandHistoryLength);
	console.inputString = PushArray(constantArena, unsigned char, console.maxInputStringLength);
	console.inputString.length = 0;

	console.commands = PushArray(constantArena, ConsoleCommand, 0);

	BuildStaticArray(constantArena, console.commands, CreateCommand("hello", HelloHelper, 0, 0));
	BuildStaticArray(constantArena, console.commands, CreateCommand("add", AddHelper, 2, 2));
	BuildStaticArray(constantArena, console.commands, CreateCommand("clear", ClearHelper, 0, 0));
	BuildStaticArray(constantArena, console.commands, CreateCommand("lorum", LorumHelper, 0, 0));
	BuildStaticArray(constantArena, console.commands, CreateCommand("help", HelpHelper, 0, 0));
	BuildStaticArray(constantArena, console.commands, CreateCommand("tweek", TweekHelper, 0, 2));
	BuildStaticArray(constantArena, console.commands, CreateCommand("save", SaveTweekersHelper, 1, 1));
	BuildStaticArray(constantArena, console.commands, CreateCommand("tweekers", TweekersHelper, 0, 0));
	BuildStaticArray(constantArena, console.commands, CreateCommand("grisu", GrisuHelper, 1, 1));
	BuildStaticArray(constantArena, console.commands, CreateCommand("convert", ConvertHelper, 1, 1));
	BuildStaticArray(constantArena, console.commands, CreateCommand("saveLevel", SaveLevelHelper, 1, 1));
	BuildStaticArray(constantArena, console.commands, CreateCommand("loadLevel", LoadLevelHelper, 1, 1));
	BuildStaticArray(constantArena, console.commands, CreateCommand("newLevel", NewLevelHelper, 0, 0));
	BuildStaticArray(constantArena, console.commands, CreateCommand("addMesh", AddMeshHelper, 1, 1));
	BuildStaticArray(constantArena, console.commands, CreateCommand("saveCamera", SaveCameraHelper, 0, 0));
}

static bool ConsoleActive()
{
	return (console.selecting);
}

static bool ConsoleReadyToType()
{
	return (console.selecting & SelectTag_TextField);
}

static bool ConsoleOpen()
{
	return (console.intendedOpenness > 0.0f);
}

static void AddSingleLineToHistory(String string, HistoryEntryEnum flag)
{
	if (console.historyLength + 1 < console.maxHistoryLength)
	{
		String copy = CopyString(string, console.arena);
		console.history[console.historyLength].entry = copy;
		console.history[console.historyLength].flag = flag;
		console.historyLength++;
	}
	else
	{
		Die;
	}
	if (!console.intendedOpenness) console.intendedOpenness = 4.0f * console.fontSize;
}

static void AddStringToHistory(String string, HistoryEntryEnum flag = HistoryEntry_Default)
{
	float fScale = console.fontSize / (f32)globalFont.charHeight;
	String toProcess = string;
	toProcess.length = string.length;

	String gatheredString = string;
	gatheredString.length = 0;
	f32 currentLength = 0.0f;

	while(toProcess.length)
	{
		String newWord = EatToNextSpaceReturnHead(&toProcess);
		if (toProcess.length)
		{
			newWord.length++; //there was a space
			toProcess.length--;
			toProcess.data++;
		}
		f32 newLength = GetActualStringLength(newWord, console.fontSize, globalFont);

		if (currentLength + newLength > (1.0f - console.scrollbarWidth) - 2.0f * console.fontSize)
		{
			AddSingleLineToHistory(gatheredString, flag);
			
			currentLength = newLength;
			gatheredString = newWord;
		}
		else
		{
			currentLength += newLength;
			gatheredString.length += newWord.length;
		}
	}
	AddSingleLineToHistory(gatheredString, flag);
}

static void ConsoleOutput(String toPrint)
{
	AddStringToHistory(toPrint, HistoryEntry_Default);
}
static void ConsoleOutputError(String toPrint)
{
	AddStringToHistory(toPrint, HistoryEntry_Error);
}

static void ConsoleOutput(char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	String toPrint = StringFormatHelper(frameArena, format, argList);
	va_end(argList);

	AddStringToHistory(toPrint, HistoryEntry_Default);
}
static void ConsoleOutputError(char *format, ...)
{
	String toPrint = BeginConcatenate(frameArena);
	va_list argList;
	va_start(argList, format);
	StringFormatHelper(frameArena, format, argList);
	va_end(argList);
	EndConcatenate(&toPrint, frameArena);

	AddStringToHistory(toPrint, HistoryEntry_Error);
}

static u32 GetBestCursorLocationForString(String string, float xPos, f32 xOffset)
{
	Font font = globalFont;

	f32 stringSize = xOffset;
	float fScale = console.fontSize / (f32)font.charHeight;

	f32 bestDistance = Dist(xPos, stringSize);

	for (u32 i = 0; i < string.length; i++)
	{
		Char currentChar = string[i];
		if (currentChar < font.amountOfChars)
		{
			CharData charData = font.charData[currentChar];
			stringSize += charData.xAdvance * fScale;
			f32 newDist = Dist(xPos, stringSize);
			if (newDist > bestDistance)
			{
				return i;
			}
			bestDistance = newDist;
		}
	}

	return string.length;
}

static u32 GetBestCursorLocationForConsoleHistory(Input input)
{
	Font font = globalFont;

	f32 lineSize = 1.5f * console.fontSize;
	f32 amountOfDisplayedLinesf = console.openness / lineSize;
	u32 amountOfDisplayedLines = (u32)amountOfDisplayedLinesf + 1;

	i32 topLine = (i32)(console.historyLength - amountOfDisplayedLines - console.historyPos);
	f32 yOffset = 0;
	if (topLine < 0)
	{
		topLine = 0;
		yOffset = console.openness - console.textInputFieldSize - (f32)(console.historyLength - console.historyPos - topLine) * lineSize;
	}

	if (yOffset > input.mouseZeroToOne.y) return 0;

	i32 bottomLine = console.historyLength - 1 - console.historyPos;

	f32 linef = MapRangeToRangeCapped(input.mouseZeroToOne.y, yOffset, console.openness - console.textInputFieldSize, (f32)topLine, (f32)bottomLine);

	u32 line = (u32)(linef + 0.5f);

	u32 bestInLine = GetBestCursorLocationForString(console.history[line].entry, input.mouseZeroToOne.x, console.historyXOffset);

	return bestInLine + (u32)(console.history[line].entry.data - console.history[0].entry.data);
}

static void ConsoleResetSelection()
{
	console.firstSelectPos = 0;
	console.endSelectPos = 0;
	console.selecting &= ~SelectTag_Selecting;
}

static b32 ConsoleHasActiveTextFieldSelection()
{
	return (console.firstSelectPos != console.endSelectPos) && (console.selecting & SelectTag_TextField);
}
static b32 ConsoleHasActiveHistorySelection()
{
	return (console.firstSelectPos != console.endSelectPos) && (console.selecting & SelectTag_History);
}

static void ConsoleHandleCommand(String inputLine)
{
	if (console.commandHistoryLength + 1 < console.maxCommandHistoryLength)
	{
		console.commandHistory[console.commandHistoryLength++] = inputLine;
	}

	String remaining = inputLine;
	EatSpaces(&remaining);
	if (!remaining.length) { return; }

	String command = EatToNextSpaceReturnHead(&remaining);

	bool parsedCommand = false;

	For(console.commands)
	{
		if (it->name == command)
		{
			parsedCommand = true;
			ConsoleCommand selectedCommand = *it;

			StringArray args = {};
			Clear(gameState.workingArena);
			args = PushArray(gameState.workingArena, String, 0);

			EatSpaces(&remaining);
			while (remaining.length)
			{
				String arg = EatToNextSpaceReturnHead(&remaining);
				Assert(arg.length);
				BuildStaticArray(gameState.workingArena, args, arg);
				EatSpaces(&remaining);
			}

			if (!(selectedCommand.minArgs <= args.amount && args.amount <= selectedCommand.maxArgs))
			{
				ConsoleOutput("Error: Wrong amount of Arguments!");
				break;
			}

			selectedCommand.interp(args);
			break;
		}
	}
	
	if (!parsedCommand)
	{
		ConsoleOutput("Error: This is not a command.");
	}
}

static void RemoveSubstring(String *string, u32 first, u32 last)
{
	if (first == last) return;
	Assert(first < last);
	Assert(last <= string->length);

	String rest = *string;
	rest.data += last;
	rest.length -= last;

	String toWrite = *string;
	toWrite.data += first;

	CopyStringToString(rest, &toWrite);
	string->length -= (last - first);
}

static void ShiftSelectionUpdate(u32 intendedPos)
{
	if (ConsoleHasActiveTextFieldSelection())
	{
		console.endSelectPos = intendedPos;
	}
	else
	{
		console.selecting = SelectTag_TextField;
		console.firstSelectPos = console.cursorPos;
		console.endSelectPos = intendedPos;
	}

}

static void ConsoleHandleEvent(KeyStateMessage message, Input *input)
{
	TimedBlock;

	if (message.flag & (KeyState_PressedThisFrame | KeyState_Repeaded))
	{
		switch (message.key)
		{
		case Key_F1:
		{
			f32 newIntededOpenness = (message.flag & KeyState_ShiftDown) ? console.openWide : console.open;

			if (console.intendedOpenness == newIntededOpenness)
			{
				console.intendedOpenness = 0.0f;
				console.selecting = 0;
			}
			else
			{
				console.intendedOpenness = newIntededOpenness;
				console.selecting = SelectTag_TextField;
			}
		}break;
		case Key_leftMouse:
		{
			if (console.openness - console.textInputFieldSize <= input->mouseZeroToOne.y && input->mouseZeroToOne.y <= console.openness)
			{
				console.firstSelectPos = GetBestCursorLocationForString(console.inputString, console.typeFieldTextScrollOffset + input->mouseZeroToOne.x, console.typeFieldXOffset);
				console.selecting = SelectTag_Selecting | SelectTag_TextField;
			}
			else if (input->mouseZeroToOne.y <= console.openness - console.textInputFieldSize && input->mouseZeroToOne.x <= 1.0f - console.scrollbarWidth)
			{
				console.firstSelectPos = GetBestCursorLocationForConsoleHistory(*input);
				console.selecting = SelectTag_Selecting | SelectTag_History;
			}
			else if (input->mouseZeroToOne.y < console.openness - console.textInputFieldSize && 1.0f - console.scrollbarWidth < input->mouseZeroToOne.x)
			{
				console.scrollingScrollbar = true;
			}
			else
			{
				console.selecting = 0;
			}


		}break;
		
		}
	}

	if (message.flag & (KeyState_ReleasedThisFrame))
	{
		switch (message.key)
		{
		case Key_leftMouse:
		{
			console.selecting &= ~SelectTag_Selecting;
			console.scrollingScrollbar = false;
		}break;
		case Key_escape:
		{
			console.intendedOpenness = 0.0f;
		}break;

		}

	}

	if (!ConsoleActive())
	{
		return;
	}

	if (message.flag & (KeyState_PressedThisFrame | KeyState_Repeaded))
	{
		switch (message.key)
		{
		
		case Key_mouseWheelForward:
		{
			f32 lineSize = 1.5f * console.fontSize;
			f32 amountOfDisplayedLinesf = console.openness / lineSize;
			u32 amountOfDisplayedLines = (u32)amountOfDisplayedLinesf;

			if (console.historyLength > amountOfDisplayedLines + console.historyPos)
			{
				console.historyPos++;
			}

		}break;
		case Key_mouseWheelBack:
		{
			if (console.historyPos)
			{
				console.historyPos--;
			}
		}break;
		case Key_c:
		{
			if (message.flag & KeyState_ControlDown)
			{
				u32 first = Min((u32)console.firstSelectPos, (u32)console.endSelectPos);
				u32 end = Max((u32)console.firstSelectPos, (u32)console.endSelectPos);

				if (console.selecting & SelectTag_History)
				{
					String stringToCopy;
					stringToCopy.data = console.history[0].entry.data + first;
					stringToCopy.length = end - first;
					OSSetClipBoard(stringToCopy);
				}
				else if (console.selecting & SelectTag_TextField)
				{
					String stringToCopy;
					stringToCopy.data = console.inputString.data + first;
					stringToCopy.length = end - first;
					OSSetClipBoard(stringToCopy);
				}

				return;
			}
		}break;
		}
	}


	if (!ConsoleReadyToType())
	{
		return;
	}

	if (message.flag & (KeyState_PressedThisFrame | KeyState_Repeaded))
	{
		switch (message.key)
		{
		case Key_v:
		{
			if (message.flag & KeyState_ControlDown)
			{
				if (console.selecting & SelectTag_History)
				{
					return;
				}

				String toAdd = OSGetClipBoard();

				if (console.inputString.length + toAdd.length > console.maxInputStringLength)
				{
					return;
				}
				u32 first = Min((u32)console.firstSelectPos, (u32)console.endSelectPos);
				u32 end = Max((u32)console.firstSelectPos, (u32)console.endSelectPos);

				RemoveSubstring(&console.inputString, first, end);

				String head = CreateString(console.inputString.data, console.cursorPos);
				String tailToCopy = CreateString(console.inputString.data + console.cursorPos, console.inputString.length);

				String tail = CreateString(console.inputString.data + console.cursorPos + toAdd.length, 0);
				CopyStringToString(tailToCopy, &tail);
				CopyStringToString(toAdd, &tailToCopy);
				console.inputString.length += toAdd.length;

				console.cursorPos += toAdd.length - (end - first);

				ConsoleResetSelection();

				return;
			}
		}break;

		case Key_backSpace:
		{
			if (ConsoleHasActiveTextFieldSelection())
			{
				u32 first = Min((u32)console.firstSelectPos, (u32)console.endSelectPos);
				u32 end = Max((u32)console.firstSelectPos, (u32)console.endSelectPos);

				RemoveSubstring(&console.inputString, first, end);
				console.cursorPos = first;
				console.recallHistoryPos = 0;

				ConsoleResetSelection();
			}
			else if (console.cursorPos)
			{
				RemoveSubstring(&console.inputString, console.cursorPos - 1, console.cursorPos);
				console.cursorPos--;
				console.recallHistoryPos = 0;
			}
		}break;
		
		case Key_enter:
		{
			Char *beginOfCommand = PushData(console.arena, Char, 0); // todo: stupid hack
			AddStringToHistory(console.inputString, HistoryEntry_Command);
			String command = CreateString(beginOfCommand, console.inputString.length);

			ConsoleHandleCommand(command);

			console.typeFieldTextScrollOffset = 0;
			console.inputString.length = 0;
			console.cursorPos = 0;
			console.historyPos = 0;
			console.recallHistoryPos = 0;

			ConsoleResetSelection();

		}break;
		case Key_up:
		{
			if (console.recallHistoryPos < console.commandHistoryLength)
			{
				console.recallHistoryPos++;
				String toCopy = console.commandHistory[console.commandHistoryLength - console.recallHistoryPos];
				CopyStringToString(toCopy, &console.inputString);
				console.cursorPos = console.inputString.length;

				ConsoleResetSelection();
			}
		}break;
		case Key_down:
		{
			if (console.recallHistoryPos > 1)
			{
				console.recallHistoryPos--;
				String toCopy = console.commandHistory[console.commandHistoryLength - console.recallHistoryPos];
				CopyStringToString(toCopy, &console.inputString);
				console.cursorPos = console.inputString.length;

				ConsoleResetSelection();
			}
			else if (console.recallHistoryPos == 1)
			{
				console.recallHistoryPos = 0;
				console.inputString.length = 0;
				console.cursorPos = 0;
			}

		}break;
		
		case Key_left:
		{
			if ((message.flag & KeyState_ShiftDown))
			{
				if (console.cursorPos > 0)
				{
					ShiftSelectionUpdate(console.cursorPos - 1);
					console.cursorPos--;
				}
			}
			else if (ConsoleHasActiveTextFieldSelection())
			{
				console.cursorPos = Min(console.firstSelectPos, console.endSelectPos);
				ConsoleResetSelection();
			}
			else if (console.cursorPos > 0)
			{
				console.cursorPos--;
			}

		}break;
		case Key_right:
		{
			if ((message.flag & KeyState_ShiftDown) )
			{
				if (console.cursorPos < console.inputString.length)
				{
					ShiftSelectionUpdate(console.cursorPos + 1);
					console.cursorPos++;
				}
			}
			else if (ConsoleHasActiveTextFieldSelection())
			{
				console.cursorPos = Max(console.firstSelectPos, console.endSelectPos);
				ConsoleResetSelection();
			}
			else if (console.cursorPos < console.inputString.length)
			{
				console.cursorPos++;
			}

		}break;
		case Key_end:
		{
			if ((message.flag & KeyState_ShiftDown))
			{
				ShiftSelectionUpdate(console.inputString.length);
			}
			else
			{
				ConsoleResetSelection();
			}
			console.cursorPos = console.inputString.length;

		}break;
		case Key_pos1:
		{
			if ((message.flag & KeyState_ShiftDown))
			{
				ShiftSelectionUpdate(0);
			}
			else
			{
				ConsoleResetSelection();
			}
			console.cursorPos = 0;

		}break;
		
		default:
		{

		}break;
		}

		Char charPressed = KeyToChar(message.key, message.flag & KeyState_ShiftDown);

		if (charPressed)
		{
			console.recallHistoryPos = 0;
			if (ConsoleHasActiveTextFieldSelection())
			{
				u32 first = Min((u32)console.firstSelectPos, (u32)console.endSelectPos);
				u32 end = Max((u32)console.firstSelectPos, (u32)console.endSelectPos);

				if (first < (end - 1))
				{
					RemoveSubstring(&console.inputString, first, end - 1);
				}
				
				console.inputString[first] = charPressed;

				console.cursorPos = first + 1;
				ConsoleResetSelection();

			}
			else if (console.inputString.length + 1 < console.maxInputStringLength)
			{
				String rest = console.inputString;
				rest.data += console.cursorPos;
				rest.length -= console.cursorPos;
				String copy = CopyString(rest);
				console.inputString.data[console.cursorPos++] = charPressed;
				console.inputString.length++;

				for (u32 i = 0; i < copy.length; i++)
				{
					console.inputString.data[console.cursorPos + i] = copy.data[i];
				}

			}
		}
		console.cursorTimer = 0.0f;
	}

}

static void UpdateConsole(Input input) 
{
	TimedBlock;

	if (console.intendedOpenness > console.openness)
	{
		console.openness += console.dt * input.dt;
		if(console.openness > console.intendedOpenness)
		{ 
			console.openness = console.intendedOpenness;
		}
	}
	else
	{
		console.openness -= console.dt * input.dt;
		if (console.openness < console.intendedOpenness)
		{
			console.openness = console.intendedOpenness;
		}
	}

	String preCursorString = console.inputString;
	preCursorString.length = console.cursorPos;

	f32 stringLengthUpToCursor = GetActualStringLength(preCursorString, console.fontSize, globalFont);
	
	if ((stringLengthUpToCursor - console.typeFieldTextScrollOffset) > (1.0f - console.cursorScrollEdge))
	{
		console.typeFieldTextScrollOffset = stringLengthUpToCursor - (1.0f - console.cursorScrollEdge);
	}
	else if ((stringLengthUpToCursor >  console.cursorScrollEdge) && (stringLengthUpToCursor - console.typeFieldTextScrollOffset) < console.cursorScrollEdge)
	{
		console.typeFieldTextScrollOffset = stringLengthUpToCursor - console.cursorScrollEdge;
	}

	console.cursorTimer += input.dt;
	if (console.cursorTimer > 1.0f)
	{
		console.cursorTimer -= 1.0f;
	}

	if (console.selecting & SelectTag_Selecting)
	{
		if (console.selecting & SelectTag_TextField)
		{
			u32 bestLocation = GetBestCursorLocationForString(console.inputString, console.typeFieldTextScrollOffset + input.mouseZeroToOne.x, console.typeFieldXOffset);
			console.endSelectPos = bestLocation;

			console.cursorPos = bestLocation;
			console.cursorTimer = 0.0f;
		}
		else if (console.selecting & SelectTag_History)
		{
			u32 bestLocation = GetBestCursorLocationForConsoleHistory(input);
			console.endSelectPos = bestLocation;

			console.cursorTimer = 0.0f;
			//ConsoleOutputString(CreateString(workingArena, bestLocation));
		}
		else
		{
			ConsoleOutput("What are we selecting?");
		}
		
	}
	else if (console.scrollingScrollbar)
	{
		f32 lineSize = 1.5f * console.fontSize;
		f32 amountOfDisplayedLinesf = console.openness / lineSize;
		u32 amountOfDisplayedLines = (u32)amountOfDisplayedLinesf + 1;

		i32 topLine = (i32)(console.historyLength - amountOfDisplayedLines - console.historyPos);
		topLine = Max(0, topLine);
		i32 bottomLine = console.historyLength - 1 - console.historyPos;

		f32 scrollbarBottom = console.openness - console.textInputFieldSize;
		f32 scrollMiny = (f32)(topLine) / (f32)console.historyLength * scrollbarBottom;
		f32 scrollMaxy = (f32)(bottomLine + 1) / (f32)console.historyLength * scrollbarBottom;

		f32 scrollbarHalfLength = 0.5f * (scrollMaxy - scrollMiny);

		f32 inpMid = input.mouseZeroToOne.y;
		f32 bottomMinusHalfLength = scrollbarBottom - scrollbarHalfLength;

		f32 intendedScollbarMid = Clamp(inpMid, scrollbarHalfLength, bottomMinusHalfLength);
		f32 scrollbarIntendedBot = intendedScollbarMid + scrollbarHalfLength;
		f32 intendedBottomLine = scrollbarIntendedBot / scrollbarBottom * (f32)console.historyLength - 1.0f;
		u32 intendedHistoryPos = console.historyLength - 1 - (u32)intendedBottomLine;
		console.historyPos = intendedHistoryPos;

	}
}

static void ConsoleDrawSelection(RenderGroup *rg, String string, u32 first, u32 end, f32 yMin, f32 yMax, f32 xOffset)
{
	String prefirst = string;
	prefirst.length = first;
	f32 stringLengthUpToSelection = GetActualStringLength(prefirst, console.fontSize, globalFont);
	String selection = string;
	selection.length = (end - first);
	f32 selectionLength = GetActualStringLength(selection, console.fontSize, globalFont);

	f32 selectionMinX = stringLengthUpToSelection - console.typeFieldTextScrollOffset + xOffset;
	f32 selectionMaxX = selectionMinX + selectionLength;

	Tweekable(v4, consoleSelectionColor); // V4(1.0f, 0.0f, 0.2f, 0.45f)

	PushRectangle(rg, V2(selectionMinX, yMin), V2(selectionMaxX, yMax), consoleSelectionColor);
}


// 0 to 1 screen space
static void DrawConsole(RenderGroup *rg) 
{
	if (console.openness == 0.0f) return;

	v2 p1 = V2(0, 0);
	v2 p2 = V2(1, 0);
	v2 p3 = V2(0, console.openness);
	v2 p4 = V2(1, console.openness);

	//mainField
	PushRectangle(rg, p1, V2(1 - console.scrollbarWidth, console.openness) - V2(0, console.textInputFieldSize), V4(1.0f, 0.5f, 0.6f, 0.85f));

	//typeField
	Tweekable(v4, consoleTextFieldColor); //V4(1.0f, 0.6f, 0.7f, 0.85f)
	v2 typeFieldPos = p3 - V2(0, console.textInputFieldSize);
	PushRectangle(rg, typeFieldPos, p4, consoleTextFieldColor);

	//cursor
	if ((console.cursorTimer < 0.5f) && !(console.selecting & SelectTag_History))
	{
		String preCursorString = console.inputString;
		preCursorString.length = console.cursorPos;

		f32 stringLengthUpToCursor = GetActualStringLength(preCursorString, console.fontSize, globalFont); 
		v2 cursorPos = V2(console.typeFieldXOffset + stringLengthUpToCursor - console.typeFieldTextScrollOffset, typeFieldPos.y);
		PushRectangle(rg, cursorPos, console.cursorWidth, console.fontSize, V4(1.0f, 1.0f, 1.0f, 0.85f));
	}

	//typefield Selection
	if (ConsoleHasActiveTextFieldSelection())
	{
		u32 first = Min((u32)console.firstSelectPos, (u32)console.endSelectPos);
		u32 end = Max((u32)console.firstSelectPos, (u32)console.endSelectPos);

		ConsoleDrawSelection(rg, console.inputString, first, end, typeFieldPos.y, p4.y, console.typeFieldXOffset);
	}

	f32 lineSize = 1.5f * console.fontSize;
	f32 amountOfDisplayedLinesf = console.openness / lineSize;
	u32 amountOfDisplayedLines = (u32)amountOfDisplayedLinesf + 1;

	if (console.historyLength < amountOfDisplayedLines)
	{
		amountOfDisplayedLines = console.historyLength;
	}

	v2 consoleFieldFirstRow = V2(console.historyXOffset, typeFieldPos.y);
	i32 topLine = (i32)(console.historyLength - amountOfDisplayedLines - console.historyPos);
	topLine = Max(0, topLine);
	i32 bottomLine = console.historyLength - 1 - console.historyPos;

	//History Selection
	if (ConsoleHasActiveHistorySelection())
	{
		u32 first = Min((u32)console.firstSelectPos, (u32)console.endSelectPos);
		u32 end = Max((u32)console.firstSelectPos, (u32)console.endSelectPos);

		u32 firstLine = 0;
		for (i32 i = topLine; i <= bottomLine; i++)
		{
			u32 linePos = (u32)(console.history[i].entry.data - console.history[0].entry.data);
 			if (linePos > first)
			{
				break;
			}
			firstLine = i;
		}

		u32 endLine = firstLine;
		for (i32 i = firstLine; i <= bottomLine; i++)
		{
			u32 linePos = (u32)(console.history[i].entry.data - console.history[0].entry.data);
			if (linePos > end)
			{
				break;
			}
			endLine = i;
		}
		
		if (endLine == firstLine)
		{
			u32 firstLinePosInBuffer = (u32)(console.history[firstLine].entry.data - console.history[0].entry.data);
			u32 newFirst = SaveSubstract(first, firstLinePosInBuffer);
			first = newFirst;
			u32 newEnd = SaveSubstract(end, firstLinePosInBuffer);
			end = newEnd;

			f32 pos = (f32)(console.historyLength - console.historyPos - endLine);

			ConsoleDrawSelection(rg, console.history[firstLine].entry, first, end, consoleFieldFirstRow.y - pos * lineSize, consoleFieldFirstRow.y - (pos - 1) * lineSize, console.historyXOffset);
		}
		else
		{
			u32 firstLinePosInBuffer = (u32)(console.history[firstLine].entry.data - console.history[0].entry.data);
			u32 firstLineEndInBuffer = (u32)(console.history[firstLine + 1].entry.data - console.history[0].entry.data);
			u32 endLinePosInBuffer = (u32)(console.history[endLine].entry.data - console.history[0].entry.data);
			u32 endLineEndInBuffer = (u32)(console.history[endLine + 1].entry.data - console.history[0].entry.data);

			//draw head
			{
				u32 firstHead = SaveSubstract(first, firstLinePosInBuffer);
				u32 endHead = firstLineEndInBuffer;

				f32 pos = (f32)(console.historyLength - console.historyPos - firstLine);

				ConsoleDrawSelection(rg, console.history[firstLine].entry, firstHead, endHead, consoleFieldFirstRow.y - pos * lineSize, consoleFieldFirstRow.y - (pos - 1) * lineSize, console.historyXOffset);
			}


			//fill inbetween
			for (u32 i = firstLine + 1; i < endLine; i++)
			{
				u32 firstI = 0;
				u32 endI = console.history[i].entry.length;

				f32 pos = (f32)(console.historyLength - console.historyPos - i);

				ConsoleDrawSelection(rg, console.history[i].entry, firstI, endI, consoleFieldFirstRow.y - pos * lineSize, consoleFieldFirstRow.y - (pos - 1) * lineSize, console.historyXOffset);
			}

			//draw tail
			{
				u32 firstEnd = 0;
				u32 endEnd = SaveSubstract(end, endLinePosInBuffer);

				f32 pos = (f32)(console.historyLength - console.historyPos - endLine);

				ConsoleDrawSelection(rg, console.history[endLine].entry, firstEnd, endEnd, consoleFieldFirstRow.y - pos * lineSize, consoleFieldFirstRow.y - (pos - 1) * lineSize, console.historyXOffset);
			}
		}
	}

	//typefield Text
	v2 typeFieldTextPos = p3 + V2(console.typeFieldXOffset - console.typeFieldTextScrollOffset, -console.textInputFieldSize);
	PushString(rg, typeFieldTextPos - V2(0.001f, 0.001f), console.inputString, console.fontSize, globalFont, V4(1, 0.5f, 0.5f, 0.5f));
	PushString(rg, typeFieldTextPos, console.inputString, console.fontSize, globalFont, V4(1.0f, 1.0f, 1.0f, 1.0f)); 

	//History Text
	for (i32 i = bottomLine; i >= topLine; i--)
	{
		f32 pos = (f32)(console.historyLength - console.historyPos - i);
		v4 color = ColorForHistoryEntry(console.history[i].flag);
		PushString(rg, consoleFieldFirstRow - pos * V2(0, lineSize), console.history[i].entry, console.fontSize, globalFont, color);
	}
	
	//scrollbar
	f32 scrollbarBottom = console.openness - console.textInputFieldSize;
	v2 scrollbarFieldMin = V2(1.0f - console.scrollbarWidth, 0.0f);
	v2 scrollbarFieldMax = V2(1.0f, scrollbarBottom);
	Tweekable(v4, consoleScrollbarBackGroundColor);
	PushRectangle(rg, scrollbarFieldMin, scrollbarFieldMax, consoleScrollbarBackGroundColor);

	Tweekable(v4, consoleScrollbarColor); //V4(1.0f, 0.5f, 0.6f, 0.85f)
	if(console.historyLength)
	{
		f32 scrollMiny = (f32)(topLine) / (f32)console.historyLength * scrollbarBottom;
		f32 scrollMaxy = (f32)(bottomLine + 1) / (f32)console.historyLength * scrollbarBottom;

		v2 scrollbarMin = V2(1.0f - console.scrollbarWidth + 0.003f, scrollMiny);
		v2 scrollbarMax = V2(1.0f - 0.003f, scrollMaxy);

		PushRectangle(rg, scrollbarMin, scrollbarMax, consoleScrollbarColor);
	}
}

//todo:

// control left right
// maybe handle to long strings?
// make all arrays dynamic?
// dont die on to many symbols i.e dynamic arrays or rolling buffer maybe 65536 long for that juicy speed?
// sanitise strings and handle \n, \r
// dont paste if the copy buffer is a file or smth

#endif // !RR_CONSOLE

