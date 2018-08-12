#include "File.h"

static File *LoadEntireFileStub(char *fileName)
{
	return 0;
}
void FreeFileStub(File *file)
{

}

bool WriteEntireFileStub(char *fileName, File *file)
{
	return false;
}

File * (*File::LoadEntireFile)(char *fileName) = &LoadEntireFileStub;
void (*File::FreeFile)(File *file) = &FreeFileStub;
bool (*File::WriteEntireFile)(char *fileName, File *file) = &WriteEntireFileStub;
bool File::initialized = false;

File::File()
{
}

File::File(void* memory, unsigned int fileSize)
{

	this->memory = memory;
	this->fileSize = fileSize;

}


File::File(char* fileName)
{
	File *tempFile;
	tempFile = LoadEntireFile(fileName);

	this->memory = tempFile->GetMemory();
	this->fileSize = tempFile->GetSize();
}


File::~File()
{
	
}

void File::Initialize(File *(*LoadEntireFile)(char *fileName), void (*FreeFile)(File *file), bool (* WriteEntireFile)(char *fileName, File *file))
{
	initialized = true;
	File::LoadEntireFile = LoadEntireFile;
	File::FreeFile = FreeFile;
	File::WriteEntireFile = WriteEntireFile;
}

void File::Free()
{
	FreeFile(this);
}

void File::Write(char *fileName)
{
	WriteEntireFile(fileName, this);
}

void* File::GetMemory()
{
	return memory;
}

int File::GetSize()
{
	return fileSize;
}