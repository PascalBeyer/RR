#ifndef RR_FILE
#define RR_FILE

class File
{
public:
	File();
	File(char * fileName);
	File(void* memory, unsigned int fileSize);
	~File();

	static void Initialize(File* (*LoadEntireFile)(char *fileName), void (*FreeFile)(File *file), bool(*WriteEntireFile)(char *fileName, File *file));
	
	void Free();
	void Write(char *fileName);

	void *GetMemory();
	int GetSize();

protected:


private:
	void * memory;
	unsigned int fileSize;

	static bool initialized;
	static bool (*WriteEntireFile)(char *fileName, File *file);
	static File *(*LoadEntireFile)(char *fileName);
	static void (*FreeFile)(File *file);

};


#endif