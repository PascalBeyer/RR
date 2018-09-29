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

extern bool (*WriteEntireFile)(char *fileName, File file); // todo make these either allways take arenas or make arena versions at least
extern File (*LoadFile)(char *fileName);
extern void (*FreeFile)(File file);



#endif