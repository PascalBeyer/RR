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

static String CopyString(String str, Arena *arena = frameArena) // todo memcopy
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

static String CreateString(char *nullTerminatedString) // todo fast
{
	return CreateString((Char *)nullTerminatedString, NullTerminatedStringLength(nullTerminatedString));
}

static String S(char *nullTerminatedString)
{
	return CreateString((Char *)nullTerminatedString, NullTerminatedStringLength(nullTerminatedString));
}

static String S(char c, Arena *arena = frameArena)
{
	String ret;
	ret.length = 1;
	ret.data = PushStruct(arena, Char);
	ret.data[0] = (Char)c;
	return ret;
}

static String S(char *nullTerminatedString, Arena *arena)
{
	String ret;
	u32 length = NullTerminatedStringLength(nullTerminatedString);
	ret.data = PushArray(arena, Char, length);
	ret.length = length;
	memcpy(ret.data, nullTerminatedString, length * sizeof(Char));

	return ret;
}

static String CreateString(Char *nullTerminatedString)
{
	return CreateString(nullTerminatedString, NullTerminatedStringLength((char *)nullTerminatedString));
}

static String Append(String a, String b, Arena *arena = frameArena)
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

/*
static String operator+(String a, String b)
{
	return Append(a, b, frameArena);
}
*/

static String UtoS(u64 integer, Arena *arena = frameArena)
{
	String ret;

	u64 scaler = 1;
	u64 converter = integer ? (u64)integer : 1;

	u32 counter = 0;
	while (converter != 0)
	{
		scaler *= 10;
		counter++;
		converter /= 10;
	}
	u32 index = 0;

	ret.data = PushArray(arena, Char, counter);
	ret.length = counter;
	
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

static String UtoS(u32 integer, Arena *arena = frameArena)
{
	return UtoS((u64)integer, arena);
}

static String ItoS(i32 integer, Arena *arena = frameArena)
{
	if (integer < 0)
	{
		return Append(CreateString("-"), UtoS((u32)(-integer), arena));
	}
	else
	{
		return UtoS((u32)integer, arena);
	}
}

static String ItoS(i64 integer, Arena *arena = frameArena)
{
	if (integer < 0)
	{
		return Append(CreateString("-"), UtoS((u64)(-integer), arena));
	}
	else
	{
		return UtoS((u64)integer);
	}
}

static String FtoS(float rational, u32 numberOfDigits, Arena *arena = frameArena) // todo : maybe make these good or use ryans stuff?
{
	int intPart = (int)rational;
	float ratPart = rational - intPart;
	if (ratPart < 0)
	{
		ratPart *= -1.0f;
	}

	String s = Append(ItoS(intPart, workingArena), CreateString("."), workingArena);
	for (u32 i = 0; i < numberOfDigits; i++)
	{
		ratPart *= 10;
		intPart = (int)ratPart;
		s = Append(s, ItoS(intPart, workingArena), workingArena);
		ratPart -= intPart;
	}
	return CopyString(s, arena);
}

static String FtoS(float rational, Arena *arena = frameArena)
{
	return FtoS(rational, 5u, arena);
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

static bool operator==(String s, const char *nullTerminated)
{
	Assert(nullTerminated);
	Char *a = (Char *)nullTerminated;
	for (u32 i = 0; i < s.length; i++, a++)
	{
		if (!(*a) || (s[i] != *a)) return false;
	}
	
	return !(*a);
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

static String BeginConcatenate(Arena *arena)
{
	String ret;
	ret.data = PushArray(arena, Char, 0); //todo make begin and end concatenate ?
	return ret;
}
static void EndConcatenate(String *s, Arena *arena)
{
	s->length = (u32)(arena->current - s->data);
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

static String EatToNextSpaceReturnHead(String *string)
{
	String ret = *string;

	for (u32 i = 0; i < ret.length; i++)
	{
		if (ret[i] == ' ')
		{
			ret.length = i;
			return ret;
		}
		string->data++;
		string->length--;
	}
	return ret;
}

static String EatToCharReturnHead(String *string, Char c)
{
	String ret = *string;

	for (u32 i = 0; i < ret.length; i++)
	{
		if (ret[i] == c)
		{
			ret.length = i;
			return ret;
		}
		string->data++;
		string->length--;
	}
	return ret;
}

static String EatToCharReturnHead(String *string, Char c1, Char c2)
{
	String ret = *string;

	for (u32 i = 0; i < ret.length; i++)
	{
		if (ret[i] == c1 || ret[i] == c2)
		{
			ret.length = i;
			return ret;
		}
		string->data++;
		string->length--;
	}
	return ret;
}

static u32 StoU(String string, b32 *success)
{
	u32 sum = 0;
	u32 exp = 0;
	for (u32 i = 0; i < string.length; i++)
	{
		u32 charToUInt = string[i] - '0';
		if (charToUInt < 10)
		{
			sum = 10 * sum + charToUInt;
		}
		else
		{
			*success = false;
		}
	}
	return sum;
}

static b32 StoB(String string, b32 *success)
{
	if (string == "false") return 0;
	if (string == "true") return 1;
	if (string == "0") return 0;
	if (string == "1") return 1;

	*success = false;
	return -1;
}

static f32 StoF(String string, b32 *success) // todo : maybe make these good or use ryans stuff?
{
	f32 sum = 0;
	f32 exp = 1.0f;

	u32 j = string.length;

	for (u32 i = 0; i < string.length; i++)
	{
		u32 charToUInt = string[i] - '0';
		if (charToUInt < 10)
		{
			sum = 10 * sum + charToUInt;
		}
		else
		{
			if (string[i] == '.')
			{
				j = i + 1;
				break;
			}
			*success = false;
			return 0.0f;
		}
	}

	for (u32 i = j; i < string.length; i++)
	{
		u32 charToUInt = string[i] - '0';
		if (charToUInt < 10)
		{
			exp /= 10.0f;
			sum = sum + (f32)charToUInt * exp;
		}
		else
		{
			*success = false;
			return 0.0f;
		}
	}

	return sum;
}


String ConsumeNextLine(String *inp) // returns head inclusive of the endline
{
	for (u32 i = 0; i < inp->length; i++)
	{
		if (inp->data[i] == '\n')
		{
			i++;
			String ret;
			ret.data = inp->data;
			ret.length = i;

			inp->length -= i;
			inp->data += i;

			return ret;
		}
	}

	String ret = *inp;

	inp->length = 0;
	inp->data = NULL; // not sure if this should be one past the end, but this seems "safer".

	return ret;
}

static void Sanitize(String *s) //todo: this maybe should be "RemoveChars" and get passed a c-string? which would be slower but more convinient
{
	u32 currentOffset = 0;
	for (u32 i = 0; i < s->length; i++)
	{
		switch (s->data[i])
		{
		case '\n':
		case '\r':
		{
			currentOffset++;
			continue;
		}break;
		}

		s->data[i - currentOffset] = s->data[i];
	}
	s->length -= currentOffset;
}

static void Eat1(String *s)
{
	Assert(s->length);
	s->data++;
	s->length--;
}

static void EatSpaces(String *string)
{
	while (string->length)
	{
		if (!(string->data[0] == ' '))
		{
			return;
		}
		string->data++;
		string->length--;
	}
}


static v2 StoV2(String string, b32 *success)
{
	String s = string;

	EatSpaces(&s);
	if (!s.length || s[0] != '(')
	{
		*success = false;
		return V2();
	}
	Eat1(&s);

	EatSpaces(&s);
	String val1 = EatToCharReturnHead(&s, ',');
	f32 a1 = StoF(val1, success);

	if (!s.length || !success)
	{
		*success = false;
		return V2();
	}
	Eat1(&s);

	EatSpaces(&s);
	String val2 = EatToCharReturnHead(&s, ')');
	f32 a2 = StoF(val2, success);

	if (!success) return V2();

	//sanity check
	Eat1(&s);
	EatSpaces(&s);
	if (s.length != 0)
	{
		*success = false;
		return V2(a1, a2);
	}

	return V2(a1, a2);
}


static v3 StoV3(String string, b32 *success)
{
	String s = string;

	EatSpaces(&s);
	if (!s.length || s[0] != '(')
	{
		*success = false;
		return V3();
	}
	Eat1(&s);

	EatSpaces(&s);
	String val1 = EatToCharReturnHead(&s, ',');
	f32 a1 = StoF(val1, success);

	if (!s.length || !success)
	{
		*success = false;
		return V3();
	}
	Eat1(&s);

	EatSpaces(&s);
	String val2 = EatToCharReturnHead(&s, ',');
	f32 a2 = StoF(val2, success);

	if (!s.length || !success)
	{
		*success = false;
		return V3();
	}
	Eat1(&s);
	EatSpaces(&s);
	String val3 = EatToCharReturnHead(&s, ')');
	f32 a3 = StoF(val3, success);

	if (!success) return V3();

	//sanity check
	Eat1(&s);
	EatSpaces(&s);
	if (s.length != 0)
	{
		*success = -1;
		return V3(a1, a2, a3); // todo maybe just warn that there was junk
	}

	return V3(a1, a2, a3);
}


static v4 StoV4(String string, b32 *success)
{
	String s = string;

	EatSpaces(&s);
	if (!s.length || s[0] != '(')
	{
		*success = false;
		return V4();
	}
	Eat1(&s);

	EatSpaces(&s);
	String val1 = EatToCharReturnHead(&s, ',');
	f32 a1 = StoF(val1, success);

	if (!s.length || !success)
	{
		*success = false;
		return V4();
	}
	Eat1(&s);

	EatSpaces(&s);
	String val2 = EatToCharReturnHead(&s, ',');
	f32 a2 = StoF(val2, success);

	if (!s.length || !success)
	{
		*success = false;
		return V4();
	}
	Eat1(&s);

	EatSpaces(&s);
	String val3 = EatToCharReturnHead(&s, ',');
	f32 a3 = StoF(val3, success);

	if (!s.length || !success)
	{
		*success = false;
		return V4();
	}
	Eat1(&s);

	EatSpaces(&s);
	String val4 = EatToCharReturnHead(&s, ')');
	f32 a4 = StoF(val4, success);

	if (!success) return V4();

	//sanity check
	Eat1(&s);
	EatSpaces(&s);
	if (s.length != 0)
	{
		*success = false;
		return V4(a1, a2, a3, a4);
	}

	return V4(a1, a2, a3, a4);
}


static String V2toS(v2 vec, Arena *arena = frameArena)
{
	String ret = BeginConcatenate(arena);

	S("(", arena);
	FtoS(vec.x, arena);
	S(",", arena);
	FtoS(vec.y, arena);
	S(")", arena);
	EndConcatenate(&ret, arena);

	return ret;
}

static String V3toS(v3 vec, Arena *arena = frameArena)
{
	String ret = BeginConcatenate(arena);

	S("(", arena);
	FtoS(vec.x, arena);
	S(",", arena);
	FtoS(vec.y, arena);
	S(",", arena);
	FtoS(vec.z, arena);
	S(")", arena);

	EndConcatenate(&ret, arena);
	return ret;
}

static String V4toS(v4 vec, Arena *arena = frameArena)
{
	String ret = BeginConcatenate(arena);

	S("(", arena);
	FtoS(vec.w, arena);
	S(",", arena);
	FtoS(vec.x, arena);
	S(",", arena);
	FtoS(vec.y, arena);
	S(",", arena);
	FtoS(vec.z, arena);
	S(")", arena);

	EndConcatenate(&ret, arena);
	return ret;
}



//typedef char* va_list; maybe look into all of this stuff, gets pretty system dependend i assume
#define va_start __crt_va_start
#define va_arg   __crt_va_arg
#define va_end   __crt_va_end
//#define va_copy(destination, source) ((destination) = (source))

static void Eat(u32 amount, String *toEat)
{
	Assert(toEat->length >= amount);
	toEat->data   += amount;
	toEat->length -= amount;
}

static String StringFormatHelper(Arena *arena, char *format, va_list args)
{
	String ret = BeginConcatenate(arena);
	String inp = S(format);
	String head = EatToCharReturnHead(&inp, '%');

	CopyString(head, arena);

	while (inp.length)
	{
		Assert(inp[0] == '%');
		Eat1(&inp);

		switch (inp[0])
		{
		case '%':
		{
			Char *percent = PushStruct(arena, Char);
			*percent = '%';
			Eat1(&inp);
		}break;
		case 'i':
		{
			if (inp[1] == '3')
			{
				Assert(inp[2] == '2');
				i32 val = va_arg(args, i32);
				ItoS(val, arena);
				
			}
			else if(inp[1] == '6')
			{
				Assert(inp[2] == '4');
				i64 val = va_arg(args, i64);
				ItoS(val, arena);
			}
			else
			{
				Assert(!"Unhandled integer type!");
			}
			Eat(3, &inp);
		}break;
		case 'u':
		{
			if (inp[1] == '3')
			{
				Assert(inp[2] == '2');
				u32 val = va_arg(args, u32);
				UtoS(val, arena);
			}
			else if (inp[1] == '6')
			{
				Assert(inp[2] == '4');
				u64 val = va_arg(args, u64);
				UtoS(val, arena);
			}
			else
			{
				Assert(!"Unhandled unsigned type!");
			}
			Eat(3, &inp);
		}break;
		case 'f':
		{
			if (inp[1] == '3')
			{
				Assert(inp[2] == '2');
				f32 val = va_arg(args, f32);
				FtoS(val, arena);
			}
			else if (inp[1] == '6')
			{
				Assert(inp[2] == '4');
				f64 val = va_arg(args, f64);
				FtoS((f32)val, arena);
			}
			else
			{
				Assert(!"Unhandled integer type!");
			}
			Eat(3, &inp);
		}break;
		case 'v':
		{
			switch (inp[1])
			{
			case '2':
			{
				v2 val = va_arg(args, v2);
				V2toS(val, arena);

			}break;
			case '3':
			{
				v3 val = va_arg(args, v3);
				V3toS(val, arena);

			}break;

			case '4':
			{
				v4 val = va_arg(args, v4);
				V4toS(val, arena);

			}break;
			default:
			{
				Assert(!"Unhandled vector size!")
			}break;
			}
			Eat(2, &inp);
		}break;
		case 's':
		{
			String val = va_arg(args, String);
			CopyString(val, arena);
			Eat1(&inp);
		}break;
		case 'c':
		{
			switch (inp[1])
			{
			case '1':
			{
				char val = va_arg(args, char);
				S(val, arena);
			}break;
			case '*':
			{
				char *val = va_arg(args, char *);
				S(val, arena);
			}break;
			default:
			{
				Assert(!"Unhandled cstyle input");
			}break;
			}
			Eat(2, &inp);
		}break;
		default:
		{
			Assert(!"Unhandled % input");
		}break;
		}

		head = EatToCharReturnHead(&inp, '%');
		CopyString(head, arena);
	}

	EndConcatenate(&ret, arena);
	return ret;
}

//ArgList:
// %% = %
// %i32 = s32
// %u32 = u32
// %f32 = f32
// %i64 = s64
// %u64 = u64
// %f64 = f64
// %v2 = v2
// %v3 = v3
// %v4 = v4
// %s = string
// %c1 = Char			// not sure if we need this
// %c* = char* (c-string)

// integer situation sux... can't even have it be %s32 because we could not handle %string32 anymore.
//todo  handle percision?
//todo  b32

static String FormatString(Arena *arena, char *format, ...)
{
	va_list argList;
	va_start(argList, format);
	String ret = StringFormatHelper(arena, format, argList);
	va_end(argList);
	return ret;
}


static String FormatString(char *format, ...)
{
	va_list argList;
	va_start(argList, format);
	String ret = StringFormatHelper(frameArena, format, argList);
	va_end(argList);
	return ret;
}

#endif
