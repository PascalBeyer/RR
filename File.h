#ifndef RR_FILE
#define RR_FILE

struct File
{
	void *memory;
	u32 fileSize;
};

static File CreateFile(void *memory, u32 fileSize)
{
	File ret;
	ret.memory = memory;
	ret.fileSize = fileSize;
	return ret;
}

static bool WriteEntireFile(char *fileName, File file);
static File LoadFile(char *fileName, Arena *arena);
static File LoadFile(char *fileName);
static void FreeFile(File file);
static File LoadFile(char *fileName, void *dest, u32 destSize);


#endif