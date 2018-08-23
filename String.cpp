#include "String.h"


String CopyString(Arena *arena, String str)
{
	String ret;
	ret.string = PushArray(arena, char, str.length);
	ret.length = str.length;
	for (u32 i = 0; i < str.length; i++)
	{
		ret.string[i] = str.string[i];
	}
	return ret;
}


String CreateString(char *string, u32 size)
{
	String ret;
	ret.string = string;
	ret.length = size;
	return ret;
}

u32 NullTerminatedStringLength(const char* c)
{
	u32 ret = 0;
	while (c[ret] != '\0') 
	{
		ret++;
	}
	return ret;
}

String CreateString(Arena *arena, char *string, u32 size)
{
	return CopyString(arena, CreateString(string, size));
}

String CreateString(char *nullTerminatedString)
{
	return CreateString(nullTerminatedString, NullTerminatedStringLength(nullTerminatedString));
}

String CreateString(Arena *arena, char *nullTerminatedString)
{
	return CreateString(arena, nullTerminatedString, NullTerminatedStringLength(nullTerminatedString));
}

String Append(Arena *arena, String a, String b)
{
	u32 size = a.length + b.length;
	char *newString = PushArray(arena, char, a.length + b.length);
	for (u32 i = 0; i < a.length; i++)
	{
		newString[i] = a.string[i];
	}
	for (u32 i = 0; i < b.length; i++)
	{
		newString[a.length + i] = b.string[i];
	}
	return CreateString(newString, size);
}

String CreateString(Arena *arena, int integer)
{
	int scaler = 1;
	int converter = integer ? integer : 1;

	int counter = 0;
	while (converter != 0)
	{
		scaler *= 10;
		counter++;
		converter /= 10;
	}
	int index = 0;

	String ret;

	if (integer < 0)
	{
		index++;
		ret.string = PushArray(arena, char, counter + 1);
		ret.length = counter + 1;
		ret.string[0] = '-';
	}
	else
	{
		ret.string = PushArray(arena, char, counter);
		ret.length = counter;
	}
	while (index != counter)
	{
		scaler /= 10;

		switch ((integer / scaler) % 10)
		{
		case 0:
		{
			ret.string[index] = '0';
		}break;
		case 1:
		{
			ret.string[index] = '1';
		}break;
		case 2:
		{
			ret.string[index] = '2';
		}break;
		case 3:
		{
			ret.string[index] = '3';
		}break;
		case 4:
		{
			ret.string[index] = '4';
		}break;
		case 5:
		{
			ret.string[index] = '5';
		}break;
		case 6:
		{
			ret.string[index] = '6';
		}break;
		case 7:
		{
			ret.string[index] = '7';
		}break;
		case 8:
		{
			ret.string[index] = '8';
		}break;
		case 9:
		{
			ret.string[index] = '9';
		}break;
		}
		index++;
	}

	return ret;
}


String CreateString(Arena *arena, float rational)
{
	int intPart = (int)rational;
	float ratPart = rational - intPart;
	if (ratPart < 0)
	{
		ratPart *= -1.0f;
	}

	Clear(workingArena);
	String s = Append(workingArena, CreateString(workingArena, intPart), CreateString("."));
	for (u32 i = 0; i < 5; i++)
	{
		ratPart *= 10;
		intPart = (int)ratPart;
		s = Append(workingArena, s, CreateString(workingArena, intPart));
		ratPart -= intPart;
	}
	return CopyString(arena, s);
}

String CreateString(Arena *arena, float rational, u32 numberOfDigits)
{
	int intPart = (int)rational;
	float ratPart = rational - intPart;
	if (ratPart < 0)
	{
		ratPart *= -1.0f;
	}

	String s = Append(workingArena, CreateString(workingArena, intPart), CreateString("."));
	for (u32 i = 0; i < numberOfDigits; i++)
	{
		ratPart *= 10;
		intPart = (int)ratPart;
		s = Append(workingArena, s, CreateString(workingArena, intPart));
		ratPart -= intPart;
	}
	return CopyString(arena, s);
}

String CreateString(Arena *arena, u32 integer)
{
	return CreateString(arena, (int)integer);
}
String CreateString(Arena *arena, char character)
{
	String ret;
	ret.string = PushStruct(arena, char);
	ret.string[0] = character;
	ret.length = 1;
	return ret;
}

String CreateString(Arena *arena, bool val)
{
	if (val)
	{
		return CreateString(arena, "true");
	}
	else
	{
		return CreateString(arena, "false");
	}
}

char* ToZeroTerminated(Arena *arena, String string)
{
	char *ret = PushArray(arena, char, string.length);
	for (u32 i = 0; i < string.length; i++)
	{
		ret[i] = string.string[i];
	}
	ret[string.length] = '\0';
	return ret;
}