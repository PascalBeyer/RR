#ifndef RR_STRING
#define RR_STRING

//todo : remove this
#include <string.h>

typedef unsigned char Char;

struct String
{
	Char *data;
	u32 length;
	Char& operator[](u32 i)
	{
		return data[i];
	}
};

DefineArray(String);

static void CopyStringToString(String inp, String *out)
{
	memcpy(out->data, inp.data, inp.length * sizeof(Char));
	out->length = inp.length;
}

static String CopyString(Arena *arena, String str)
{
	String ret;
	ret.data = PushArray(arena, Char, str.length);
	ret.length = str.length;
	for (u32 i = 0; i < str.length; i++)
	{
		ret.data[i] = str.data[i];
	}
	return ret;
}

static String CreateString(Char *string, u32 size)
{
	String ret;
	ret.data = string;
	ret.length = size;
	return ret;
}

static String CreateString(char *string, u32 size)
{
	String ret;
	ret.data = (Char *)string;
	ret.length = size;
	return ret;
}

static u32 NullTerminatedStringLength(const char* c)
{
	u32 ret = 0;
	while (c[ret] != '\0')
	{
		ret++;
	}
	return ret;
}

static String CreateString(Arena *arena, char *string, u32 size)
{
	return CopyString(arena, CreateString((Char *)string, size));
}

static String CreateString(char *nullTerminatedString)
{
	return CreateString((Char *)nullTerminatedString, NullTerminatedStringLength(nullTerminatedString));
}

static String CreateString(Char *nullTerminatedString)
{
	return CreateString(nullTerminatedString, NullTerminatedStringLength((char *)nullTerminatedString));
}

static String CreateString(Arena *arena, char *nullTerminatedString)
{
	return CreateString(arena, nullTerminatedString, NullTerminatedStringLength(nullTerminatedString));
}

static String Append(Arena *arena, String a, String b)
{
	u32 size = a.length + b.length;
	Char *newString = PushArray(arena, Char, a.length + b.length);
	for (u32 i = 0; i < a.length; i++)
	{
		newString[i] = a.data[i];
	}
	for (u32 i = 0; i < b.length; i++)
	{
		newString[a.length + i] = b.data[i];
	}
	return CreateString(newString, size);
}

static String CreateString(Arena *arena, int integer)
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
		ret.data = PushArray(arena, unsigned char, counter + 1);
		ret.length = counter + 1;
		ret.data[0] = '-';
	}
	else
	{
		ret.data = PushArray(arena, unsigned char, counter);
		ret.length = counter;
	}
	while (index != counter)
	{
		scaler /= 10;

		switch ((integer / scaler) % 10)
		{
		case 0:
		{
			ret.data[index] = '0';
		}break;
		case 1:
		{
			ret.data[index] = '1';
		}break;
		case 2:
		{
			ret.data[index] = '2';
		}break;
		case 3:
		{
			ret.data[index] = '3';
		}break;
		case 4:
		{
			ret.data[index] = '4';
		}break;
		case 5:
		{
			ret.data[index] = '5';
		}break;
		case 6:
		{
			ret.data[index] = '6';
		}break;
		case 7:
		{
			ret.data[index] = '7';
		}break;
		case 8:
		{
			ret.data[index] = '8';
		}break;
		case 9:
		{
			ret.data[index] = '9';
		}break;
		}
		index++;
	}

	return ret;
}

static String CreateString(Arena *arena, float rational)
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

static String CreateString(Arena *arena, float rational, u32 numberOfDigits)
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

static String CreateString(Arena *arena, u32 integer)
{
	return CreateString(arena, (int)integer);
}

static String CreateString(Arena *arena, char character)
{
	String ret;
	ret.data = PushStruct(arena, unsigned char);
	ret.data[0] = character;
	ret.length = 1;
	return ret;
}

static bool operator==(String a, String b)
{
	if (a.length != b.length) return false;

	for (u32 i = 0; i < a.length; i++)
	{
		if (a[i] != b[i]) return false;
	}
	return true;
}

static String CreateString(Arena *arena, bool val)
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

static char* ToNullTerminated(Arena *arena, String string)
{
	char *ret = PushArray(arena, char, string.length);
	for (u32 i = 0; i < string.length; i++)
	{
		ret[i] = string.data[i];
	}
	ret[string.length] = '\0';
	return ret;
}
#endif
