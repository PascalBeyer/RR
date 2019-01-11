#ifndef RR_FILE
#define RR_FILE

struct File
{
	union
	{
		u8 *memory;
		u8 *data;
	};
	
	union
	{
		u32 fileSize;
		u32 amount;
	};
	
};

static File CreateFile(void *memory, u32 fileSize)
{
	File ret;
	ret.memory = (u8 *)memory;
	ret.fileSize = fileSize;
	return ret;
}

static bool WriteEntireFile(char *fileName, File file);
static File LoadFile(char *fileName, Arena *arena);
static File LoadFile(char *fileName, BuddyAllocator *alloc);
static void FreeFile(BuddyAllocator *alloc, File file);
static File LoadFile(char *fileName, void *dest, u32 destSize);
static StringArray FindAllFiles(char *path, char *type, Arena *stringArena);


#endif