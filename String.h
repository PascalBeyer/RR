#ifndef RR_STRING
#define RR_STRING
#include "BasicTypes.h"
#include "Arena.h"

struct String
{
	char *string;
	u32 length;
	char operator[](u32 i)
	{
		return string[i];
	}
};

String CopyString(Arena *arena, String str);
String Append(Arena *arena, String a, String b);
String CreateString(Arena *arena, int integer);
String CreateString(Arena *arena, u32 integer);
String CreateString(Arena *arena, bool val);
String CreateString(Arena *arena, float rational);
String CreateString(Arena *arena, char character);
String CreateString(Arena *arena, float rational, u32 numberOfDigits);
String CreateString(Arena *arena, char *string, u32 size);
String CreateString(Arena *arena, char *nullTerminatedString);
String CreateString(char *string, u32 size);
String CreateString(char *nullTerminatedString);
char* ToZeroTerminated(Arena *arena, String string);


#endif
