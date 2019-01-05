#ifndef RR_CONSOLEHEADER
#define RR_CONSOLEHEADER


enum HistoryEntryEnum
{
	HistoryEntry_Default,
	HistoryEntry_Command,
	HistoryEntry_Error,
    
};

static String OSGetClipBoard();
static void OSSetClipBoard(String string);

struct HistoryEntry
{
	String entry;
	HistoryEntryEnum flag;
};

struct ConsoleCommand
{
	String name;
	void(*interp)(StringArray args);
	u32 minArgs;
	u32 maxArgs;
};

DefineArray(ConsoleCommand);

struct Console
{
	f32 fontSize = 0.023f;
	f32 openness = 0.0f;
	f32 dt = 1.5f;
	f32 intendedOpenness = 0.0f;
    
	f32 textInputFieldSize = 0.03f;
	String inputString;
	u32 maxInputStringLength = 200;
	f32 typeFieldTextScrollOffset = 0.0f;
	f32 typeFieldXOffset = 0.015f;
    
	Char *buffer;
	u32 bufferSize = 80000;
	Arena *arena;
    
	HistoryEntry *history;
	u32 maxHistoryLength = 300;
	u32 historyLength = 0;
    
	String *commandHistory;
	u32 maxCommandHistoryLength = 200;
	u32 commandHistoryLength = 0;
	u32 recallHistoryPos = 0;
	u32 historyPos = 0;
	f32 historyXOffset = 0.03f;
    
	f32 openWide = 1.0f;
	f32 open = 0.3f;
    
	u32 cursorPos = 0;
	f32 cursorScrollEdge = 0.1f;
	f32 cursorTimer = 0.0f;
	f32 cursorWidth = 0.001f;
    
	
	u32 firstSelectPos = 0;
	u32 endSelectPos = 0;
	b32 selecting = false;
    
	f32 scrollbarWidth = 0.02f;
	b32 scrollingScrollbar = false;
    
	ConsoleCommandArray commands;
};


static Console console;

static void ConsoleOutput(char* format, ...);
static void ConsoleOutputError(char *format, ...);
static void ConsoleOutputError(String toPrint);
static void ConsoleOutput(String toPrint);

#endif // !RR_ConsoleHeader

