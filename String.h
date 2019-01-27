#ifndef RR_STRING
#define RR_STRING

typedef unsigned char Char;

struct String
{
	union
	{
		Char *data;
		char *cstr;
	};
	
	union
	{
		u32 amount;
		u32 length;
	};
   
	Char& operator[](u32 i)
	{
		return data[i];
	}
};

DefineArray(String);



static String BeginConcatenate(Arena *arena)
{
	String ret;
	ret = PushArray(arena, Char, 0);
	return ret;
}
static void EndConcatenate(String *s, Arena *arena)
{
	s->length = (u32)((Char *)arena->current - s->data);
}


static void CopyStringToString(String inp, String *out)
{
	memcpy(out->data, inp.data, inp.length * sizeof(Char));
	out->length = inp.length;
}

static String CopyString(String str, Arena *arena = frameArena) // todo memcopy
{
	String ret;
	ret = PushArray(arena, Char, str.length);
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

// note, this does not check for null, right now I think thats good. Se we know if we fucked up
static u32 NullTerminatedStringLength(const char* c)
{
	u32 ret = 0;
	while (c[ret] != '\0')
	{
		ret++;
	}
	return ret;
}

static String CreateString(char *nullTerminatedString) 
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

static String S(char *nullTerminatedString, Arena *arena, bool keepNullterminated = false)
{
	String ret;
	u32 length = NullTerminatedStringLength(nullTerminatedString) + keepNullterminated;
	ret = PushArray(arena, Char, length);
	
	memcpy(ret.data, nullTerminatedString, (length + keepNullterminated) * sizeof(Char));
   
	ret.length -= keepNullterminated;
	return ret;
}

static String CreateString(Char *nullTerminatedString)
{
	return CreateString(nullTerminatedString, NullTerminatedStringLength((char *)nullTerminatedString));
}

static String Append(String a, String b, Arena *arena = frameArena)
{
	u32 size = a.length + b.length;
	String newString = PushArray(arena, Char, a.length + b.length);
	for (u32 i = 0; i < a.length; i++)
	{
		newString[i] = a.data[i];
	}
	for (u32 i = 0; i < b.length; i++)
	{
		newString[a.length + i] = b.data[i];
	}
	return newString;
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
   
	ret = PushArray(arena, Char, counter);
	
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
		String ret = BeginConcatenate(arena);
		S('-', arena);
		UtoS((u32)(-integer), arena);
		EndConcatenate(&ret, arena);
		return ret;
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


struct FloatSpread
{
	u64 mant;
	i32 exp;
};

static FloatSpread SpreadFloat(f64 f)
{
	Assert(f >= 0);
   
	u64 convertedF = *(u64 *)&f;
   
	i32 bias = 1023;
   
	FloatSpread ret;
	ret.exp = ((convertedF & 0xFFF0000000000000) >> 52); // 12 bits sign + exponents, 52  mantisse
	ret.mant = (convertedF & 0x000FFFFFFFFFFFFF) 
      | 0x0010000000000000; // getting the mantisse and the adding the hidden bit
   
	ret.exp = ret.exp - bias; // I think the exponent is rolled such that 0 is actually 0
	ret.exp = ret.exp - 52; // making it such that ret.mant * 2^ret.exp = f again
   
	return ret;
}

static FloatSpread Mult(FloatSpread x, FloatSpread y)
{
	// notes are from the case  that we are doing f32 not f64, so double everything
	u64 ax = x.mant >> 32;		//we use these to have space to multiply accuratly
	u64 bx = x.mant & 0xFFFFFFFF;
	u64 ay = y.mant >> 32;
	u64 by = y.mant & 0xFFFFFFFF;
   
	// multiplication accordint to the distributive law
	u64 upperParts = ax*ay; // should be multiplied by 2^32
	u64 lowerParts = bx*by; // should be multiplied by 2^0
	u64 mixed_axby = ax*by;	// should be multiplied by 2^16
	u64 mixed_bxay = bx*ay; // should be multiplied by 2^16
   
	// we do that at the end by doing exponent stuff and dividing everything else instead
   
	// we throw away the lowest 16 bits of lowerPart, as these would get rounded for floats
	// we are just interested in how lowerparts makes us round.
	u64 temp = (lowerParts >> 32) + (mixed_axby & 0xFFFF) + (mixed_bxay & 0xFFFF); // should be multiplied by 2^16
	u64 roundingNumber = (1U << 31);
	temp += roundingNumber; //this allways rounds towards infinity (everything is still assumed to be positive)
	FloatSpread ret;
	ret.mant = upperParts + (mixed_axby >> 32) + (mixed_bxay >> 32) + (temp >> 32); // should be multiplied by 2^32
	ret.exp = x.exp + y.exp + 64; // is multiplied by 2^32, noice!
	return ret;
}

struct CachedPower { // c = f * 2^e ~= 10^k
	uint64_t f;
	int e; // binary exponent
	int k; // decimal exponent
};

inline int BinaryExponentFromDecimalExponent(int k)
{
	return (k * 108853 - 63 * (1 << 15)) >> 15;
}

constexpr int kAlpha = -60;
constexpr int kGamma = -32;

constexpr int kCachedPowersSize = 85;
constexpr int kCachedPowersMinDecExp = -348;
constexpr int kCachedPowersMaxDecExp = 324;
constexpr int kCachedPowersDecExpStep = 8;

inline CachedPower GetCachedPower(int index)
{
	static constexpr uint64_t kSignificands[/*680 bytes*/] = {
		0xFA8FD5A0081C0288, // e = -1220, k = -348, //*
		0xBAAEE17FA23EBF76, // e = -1193, k = -340, //*
		0x8B16FB203055AC76, // e = -1166, k = -332, //*
		0xCF42894A5DCE35EA, // e = -1140, k = -324, //*
		0x9A6BB0AA55653B2D, // e = -1113, k = -316, //*
		0xE61ACF033D1A45DF, // e = -1087, k = -308, //*
		0xAB70FE17C79AC6CA, // e = -1060, k = -300, // >>> double-precision (-1060 + 960 + 64 = -36)
		0xFF77B1FCBEBCDC4F, // e = -1034, k = -292,
		0xBE5691EF416BD60C, // e = -1007, k = -284,
		0x8DD01FAD907FFC3C, // e =  -980, k = -276,
		0xD3515C2831559A83, // e =  -954, k = -268,
		0x9D71AC8FADA6C9B5, // e =  -927, k = -260,
		0xEA9C227723EE8BCB, // e =  -901, k = -252,
		0xAECC49914078536D, // e =  -874, k = -244,
		0x823C12795DB6CE57, // e =  -847, k = -236,
		0xC21094364DFB5637, // e =  -821, k = -228,
		0x9096EA6F3848984F, // e =  -794, k = -220,
		0xD77485CB25823AC7, // e =  -768, k = -212,
		0xA086CFCD97BF97F4, // e =  -741, k = -204,
		0xEF340A98172AACE5, // e =  -715, k = -196,
		0xB23867FB2A35B28E, // e =  -688, k = -188,
		0x84C8D4DFD2C63F3B, // e =  -661, k = -180,
		0xC5DD44271AD3CDBA, // e =  -635, k = -172,
		0x936B9FCEBB25C996, // e =  -608, k = -164,
		0xDBAC6C247D62A584, // e =  -582, k = -156,
		0xA3AB66580D5FDAF6, // e =  -555, k = -148,
		0xF3E2F893DEC3F126, // e =  -529, k = -140,
		0xB5B5ADA8AAFF80B8, // e =  -502, k = -132,
		0x87625F056C7C4A8B, // e =  -475, k = -124,
		0xC9BCFF6034C13053, // e =  -449, k = -116,
		0x964E858C91BA2655, // e =  -422, k = -108,
		0xDFF9772470297EBD, // e =  -396, k = -100,
		0xA6DFBD9FB8E5B88F, // e =  -369, k =  -92,
		0xF8A95FCF88747D94, // e =  -343, k =  -84,
		0xB94470938FA89BCF, // e =  -316, k =  -76,
		0x8A08F0F8BF0F156B, // e =  -289, k =  -68,
		0xCDB02555653131B6, // e =  -263, k =  -60,
		0x993FE2C6D07B7FAC, // e =  -236, k =  -52,
		0xE45C10C42A2B3B06, // e =  -210, k =  -44,
		0xAA242499697392D3, // e =  -183, k =  -36, // >>> single-precision (-183 + 80 + 64 = -39)
		0xFD87B5F28300CA0E, // e =  -157, k =  -28, //
		0xBCE5086492111AEB, // e =  -130, k =  -20, //
		0x8CBCCC096F5088CC, // e =  -103, k =  -12, //
		0xD1B71758E219652C, // e =   -77, k =   -4, //
		0x9C40000000000000, // e =   -50, k =    4, //
		0xE8D4A51000000000, // e =   -24, k =   12, //
		0xAD78EBC5AC620000, // e =     3, k =   20, //
		0x813F3978F8940984, // e =    30, k =   28, //
		0xC097CE7BC90715B3, // e =    56, k =   36, //
		0x8F7E32CE7BEA5C70, // e =    83, k =   44, // <<< single-precision (83 - 196 + 64 = -49)
		0xD5D238A4ABE98068, // e =   109, k =   52,
		0x9F4F2726179A2245, // e =   136, k =   60,
		0xED63A231D4C4FB27, // e =   162, k =   68,
		0xB0DE65388CC8ADA8, // e =   189, k =   76,
		0x83C7088E1AAB65DB, // e =   216, k =   84,
		0xC45D1DF942711D9A, // e =   242, k =   92,
		0x924D692CA61BE758, // e =   269, k =  100,
		0xDA01EE641A708DEA, // e =   295, k =  108,
		0xA26DA3999AEF774A, // e =   322, k =  116,
		0xF209787BB47D6B85, // e =   348, k =  124,
		0xB454E4A179DD1877, // e =   375, k =  132,
		0x865B86925B9BC5C2, // e =   402, k =  140,
		0xC83553C5C8965D3D, // e =   428, k =  148,
		0x952AB45CFA97A0B3, // e =   455, k =  156,
		0xDE469FBD99A05FE3, // e =   481, k =  164,
		0xA59BC234DB398C25, // e =   508, k =  172,
		0xF6C69A72A3989F5C, // e =   534, k =  180,
		0xB7DCBF5354E9BECE, // e =   561, k =  188,
		0x88FCF317F22241E2, // e =   588, k =  196,
		0xCC20CE9BD35C78A5, // e =   614, k =  204,
		0x98165AF37B2153DF, // e =   641, k =  212,
		0xE2A0B5DC971F303A, // e =   667, k =  220,
		0xA8D9D1535CE3B396, // e =   694, k =  228,
		0xFB9B7CD9A4A7443C, // e =   720, k =  236,
		0xBB764C4CA7A44410, // e =   747, k =  244,
		0x8BAB8EEFB6409C1A, // e =   774, k =  252,
		0xD01FEF10A657842C, // e =   800, k =  260,
		0x9B10A4E5E9913129, // e =   827, k =  268,
		0xE7109BFBA19C0C9D, // e =   853, k =  276,
		0xAC2820D9623BF429, // e =   880, k =  284,
		0x80444B5E7AA7CF85, // e =   907, k =  292,
		0xBF21E44003ACDD2D, // e =   933, k =  300,
		0x8E679C2F5E44FF8F, // e =   960, k =  308,
		0xD433179D9C8CB841, // e =   986, k =  316,
		0x9E19DB92B4E31BA9, // e =  1013, k =  324, // <<< double-precision (1013 - 1137 + 64 = -60)
	};
   
	int const k = kCachedPowersMinDecExp + index * kCachedPowersDecExpStep;
	int const e = BinaryExponentFromDecimalExponent(k);
   
	return { kSignificands[index], e, k };
}

static i32 ComputeExponentBase10(i32 exponentBase2)
{
	//(i32)ceil((31 - exponentBase2) * 0.30102999566398114);
	return (exponentBase2 * -78913 + ( (kAlpha - 1) * 78913 + (1 << 18))) >> 18; 
}

static u32 CountLeadingZeros(u64 mant)
{
	u32 leadingZeros = 0;
	while (!(mant >> 63))
	{
		mant <<= 1;
		++leadingZeros;
	}
	return leadingZeros;
}

static FloatSpread Normalize(FloatSpread f)
{
	u32 leadingZeros = CountLeadingZeros(f.mant);
   
	FloatSpread ret;
	ret.exp = f.exp - leadingZeros;
	ret.mant = (f.mant << leadingZeros);
	return ret;
}

static void Cut(FloatSpread f, u32 *parts)
{
	parts[2] = (f.mant % (10000000 >> f.exp)) << f.exp;
	u64 temp = (f.mant / (10000000 >> f.exp));
	parts[1] = temp % 10000000;
	parts[0] = (u32)(temp / 10000000);
}
static String FormatString(char *format, ...);
static void ConsoleOutput(char* format, ...);


static void Grisu(Arena *arena, f64 f) // todo does not work.
{
	FloatSpread f0 = SpreadFloat(f);
	FloatSpread normalized = Normalize(f0);
	i32 exp10 = ComputeExponentBase10(normalized.exp); // why +32?
	CachedPower pow = GetCachedPower(exp10);
	FloatSpread c = { pow.f, pow.e };
	
	FloatSpread asd = Mult(normalized, c);
   
	u32 parts[3];
	Cut(asd, parts);
	
	ConsoleOutput("%u32%u32%u32e%i32", parts[0], parts[1], parts[2], pow.k);
   
}


// floats : 32bit, 1 sign bit, 8 exponent 23 mantisse (all normalized, i.e there is a "hidden bit" for mantisse)
static String FtoS(float rational, u32 numberOfDigits, Arena *arena = frameArena)
{
	//FloatSpread f0 = SpreadFloat(0.5f);
	//FloatSpread f1 = SpreadFloat(1.0f);
	//FloatSpread f2 = SpreadFloat(11.0f);
	//FloatSpread f3 = SpreadFloat(F32MAX);
	//FloatSpread f4 = SpreadFloat(16.0f);
	//FloatSpread f5 = Mult(f0, f1);
	//FloatSpread f6 = Mult(f0, f2);
	//FloatSpread f7 = Mult(f0, f3);
	//FloatSpread f8 = Mult(f0, f4);
   String ret = PushArray(arena, Char, 0);
   
   if (rational < 0)
	{
      rational *= -1.0f;
      *PushStruct(arena, Char) = '-';
	}
   
   
	int intPart = (int)rational;
	float ratPart = rational - intPart;
	
	ItoS(intPart, arena);
	*PushStruct(arena, Char) = '.';
	for (u32 i = 0; i < numberOfDigits; i++) // todo table lookup?
	{
		ratPart *= 10.0f;
      Char upart = (Char)ratPart;
		*PushStruct(arena, Char) = '0' + upart;
		ratPart -= (f32)upart;
		//if (!ratPart) break;
	}
   
	ret.length = (u32)((Char *)arena->current - ret.data);
	return ret;
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

static bool operator!=(String s, const char *nullTerminated)
{
	return !(s == nullTerminated);
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
	char *ret = PushData(arena, char, string.length + 1);
	for (u32 i = 0; i < string.length; i++)
	{
		ret[i] = string.data[i];
	}
	ret[string.length] = '\0';
	return ret;
}

static char* ToNullTerminated(String string)
{
	char *ret = PushData(frameArena, char, string.length + 1);
	for (u32 i = 0; i < string.length; i++)
	{
		ret[i] = string.data[i];
	}
	ret[string.length] = '\0';
	return ret;
}


static Char Eat1(String *s)
{
	Assert(s->length);
	Char ret = *s->data;
	s->data++;
	s->length--;
	return ret;
}

static void Eat(u32 amount, String *toEat)
{
	Assert(toEat->length >= amount);
	toEat->data += amount;
	toEat->length -= amount;
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

static void EatSpacesFromEnd(String *string)
{
	while (string->length)
	{
		if (!(string->data[(string->length - 1)] == ' '))
		{
			return;
		}
		string->length--;
	}
}

// not including ' '
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


static bool BeginsWith(String toCheck, String desiredBeginning)
{
   if(toCheck.amount < desiredBeginning.amount) return false;
	for (u32 i = 0; i < desiredBeginning.amount; i++)
	{
		if (desiredBeginning[i] != toCheck[i]) return false;
	}
   
	return true;
}


static bool BeginsWith(String toCheck, char *cStr) // todo audit speed
{
	for (u32 i = 0; i < toCheck.amount, *cStr; cStr++, i++)
	{
		if (*cStr != toCheck[i]) return false;
	}
   
	return !(*cStr);
}

static bool BeginsWithEat(String *_toCheck, char *cStr) // todo audit speed
{
	// copy and paste from above
	String toCheck = *_toCheck;
	u32 i = 0;
	for (; i < toCheck.amount, *cStr; cStr++, i++)
	{
		if (*cStr != toCheck[i]) return false;
	}
   
	bool ret = !(*cStr);
   
	if (ret)
	{
		Eat(i, _toCheck);
	}
   
	return ret;
}


//not including char
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

static String GetToChar(String s, Char c)
{
	String ret = s;
   
	for (u32 i = 0; i < ret.length; i++)
	{
		if (ret[i] == c)
		{
			ret.length = i;
			return ret;
		}
	}
	return ret;
}

// does not include the searched char
static String EatToCharFromBackReturnTail(String *string, Char c)
{
	String ret = *string;
   
	for (u32 i = ret.length - 1; i < ret.length; i--)
	{
		if (ret[i] == c)
		{
			ret.data = ret.data + (i + 1);
			ret.length = ret.length - (i + 1);
			return ret;
		}
		string->length--;
	}
   
	return ret;
}

static String GetBackToChar(String string, Char c) // #zerg
{
	String ret = string;
   
	for (u32 i = ret.length - 1; i < ret.length; i--)
	{
		if (ret[i] == c)
		{
			ret.data = ret.data + (i + 1);
			ret.length = ret.length - (i + 1);
			return ret;
		}
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

static i32 StoI(String string, b32 *success)
{
	if (string[0] == '-')
	{
		Eat1(&string);
		return - (i32)StoU(string, success);
	}
   
	return StoU(string, success);
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

static f64 PowerOfTen(i32 exp) // todo speed this is stupid
{
	if (exp < 0) return 1.0 / PowerOfTen(-exp);
   
	f64 ret = 1.0f;
	for (u32 i = 0; i < (u32)exp; i++)
	{
		ret *= 10.0;
	}
	return ret;
}

//float : 1 sign 8 exponent 23 mantisse (+ 1 hidden bit)
static f32 StoF(String string, b32 *success) // todo, gawd this is slow and trashy
{
	if (!string.length)
	{
		*success = false;
		return 0.0f;
	}
	bool negative = false;
   
	if (string[0] == '-')
	{
		Eat1(&string);
		negative = true;
	}
   
	u64 sum = 0u;
	f64 exp = 1.0;
   
	while (string.length && string[0] == '0') 
	{
		Eat1(&string);
	}
   
	while(string.length && string[0] != '.' && string[0] != 'e')
	{
		u32 charToUInt = Eat1(&string) - '0';
		if (charToUInt < 10)
		{
			sum = 10 * sum + charToUInt;
		}
		else
		{
			*success = false;
			return 0.0f;
		}
	}
   
	if (string.length && string[0] != 'e')
	{
		Eat1(&string);
	}
   
	while(string.length && string[0] != 'e')
	{
		u32 charToUInt = Eat1(&string) - '0';
		if (charToUInt < 10)
		{
			sum = 10 * sum + charToUInt;
			exp *= 10;
		}
		else
		{
			*success = false;
			return 0.0f;
		}
	}
   
	if (string[0] == 'e')
	{
		Eat1(&string);
		i32 exponent = StoI(string, success);
		exp /= PowerOfTen(exponent);
	}
   
	f64 mantissa = (f64)sum;
	f32 ret = (f32)(mantissa / exp); // this is exact for any exponents we care about.
   
	if (negative) ret *= -1.0f;
   
	return ret;
}

// todo speed these two seem like they could be joined the other way around
static f32 Eatf32(String *s, b32 *success)
{
	String a = *s;
   
	For(a)
	{
		if (!(*it == '-') && !(*it == '.') && ! ('0' <= *it && *it <= '9') && (*it != 'e'))
		{
			a.length = (u32)(it - a.data);
			break;
		}
	}
   
	if (!a.length)
	{
		*success = false;
		return 0.0f;
	}
   
	Eat(a.length, s);
   
	return StoF(a, success);
}


static u32 Eatu32(String *s, b32 *success)
{
	String a = *s;
   
	For(a)
	{
		if (!(*it == '-') && !('0' <= *it && *it <= '9'))
		{
			a.length = (u32)(it - a.data);
			break;
		}
	}
   
	if (!a.length)
	{
		*success = false;
		return 0u;
	}
   
	Eat(a.length, s);
   
	return StoU(a, success);
}


static String ConsumeNextLine(String *inp) // returns head inclusive of the endline
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
	if (currentOffset)
	{
		s->data[s->length] = '\0';
	}
}

// Warning screws with the string, slow af
String ConsumeNextLineSanitize(String *inp)
{
	String head = ConsumeNextLine(inp);
	Sanitize(&head);
	return head;
}

void EatNewLinesFromEnd(String *s)
{
	while (s->amount && (s->data[s->amount - 1] == '\n' || s->data[s->amount - 1] == '\r'))
	{
		s->amount--;
	}
}

void RemoveNewLinesFromBack(String *s)
{
	while (s->amount && (s->data[s->amount - 1] == '\n' || s->data[s->amount - 1] == '\r'))
	{
		s->amount--;
	}
}

String ConsumeNextLineSanitizeEatSpaces(String *inp)
{
	String head = ConsumeNextLine(inp);
	RemoveNewLinesFromBack(&head);
	EatSpaces(&head);
	return head;
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
	FtoS(vec.a, arena);
	S(",", arena);
	FtoS(vec.r, arena);
	S(",", arena);
	FtoS(vec.g, arena);
	S(",", arena);
	FtoS(vec.b, arena);
	S(")", arena);
   
	EndConcatenate(&ret, arena);
	return ret;
}


static String SpaceString(u32 deltaLength, Arena *arena)
{
	String ret = PushArray(arena, Char, deltaLength);
	memset(ret.data, ' ', deltaLength);
	return ret;
}

//typedef char* va_list; maybe look into all of this stuff, gets pretty system dependend i assume
#define va_start __crt_va_start
#define va_arg   __crt_va_arg
#define va_end   __crt_va_end
//#define va_copy(destination, source) ((destination) = (source))

static String StringFormatHelper(Arena *arena, char *format, va_list args)
{
	String ret = BeginConcatenate(arena);
	String inp = S(format); // todo speed slow, unnecessary
	String head = EatToCharReturnHead(&inp, '%');
   
	CopyString(head, arena);
   
	while (inp.length)
	{
		Assert(inp[0] == '%');
		Eat1(&inp);
      
		b32 hasLength = true;
		u32 desiredLength = Eatu32(&inp, &hasLength);
		b32 rightBound = false;
		if (hasLength)
		{
			if (inp[0] == 'r' && inp[1] == 'b')
			{
				Eat(2, &inp);
				rightBound = true;
			}
			if (inp[0] == 'l' && inp[1] == 'b')
			{
				Eat(2, &inp);
				rightBound = false;
			}
         
		}
      
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
            i64 val = 0;
            if (inp[1] == '3' && inp[2] == '2')
            {
               val = (i64)va_arg(args, i32);
               
            }
            else if(inp[1] == '6' && inp[2] == '4')
            {
               val = va_arg(args, i64);
            }
            else
            {
               Assert(!"Unhandled integer type!");
            }
            
            if (hasLength)
            {
               u32 length = val ? (u32)log10((f64)val) + 1 : 1;
               if (desiredLength > length)
               {
                  u32 delta = desiredLength - length;
                  
                  if (rightBound)
                  {
                     SpaceString(delta, arena);
                     ItoS(val, arena);
                  }
                  else
                  {
                     ItoS(val, arena);
                     SpaceString(delta, arena);
                  }
                  
               }
               else
               {
                  ItoS(val, arena);
               }
            }
            else
            {
               ItoS(val, arena);
            }
            
            Eat(3, &inp);
         }break;
         case 'u':
         {
            u64 val = 0;
            if (inp[1] == '3' && inp[2] == '2')
            {
               val = (u64)va_arg(args, u32);
            }
            else if (inp[1] == '6' && inp[2] == '4')
            {
               val = va_arg(args, u64);
            }
            else
            {
               Assert(!"Unhandled unsigned type!");
            }
            
            if (hasLength)
            {
               u32 length = val ? (u32)log10((f64)val) + 1 : 1;
               if (desiredLength > length)
               {
                  u32 delta = desiredLength - length;
                  
                  if (rightBound)
                  {
                     SpaceString(delta, arena);
                     UtoS(val, arena);
                  }
                  else
                  {
                     UtoS(val, arena);
                     SpaceString(delta, arena);
                  }
                  
               }
               else
               {
                  UtoS(val, arena);
               }
            }
            else
            {
               UtoS(val, arena);
            }
            
            Eat(3, &inp);
         }break;
         case 'f':
         {
            f64 val = 0.0f;
            if (inp[1] == '3' && inp[2] == '2')
            {
               val = va_arg(args, f64);
               
            }
            else if (inp[1] == '6' && inp[2] == '4')
            {
               val = va_arg(args, f64);
            }
            else
            {
               Assert(!"Unhandled floating Point type!");
            }
            
            u32 tailSize = 5;
            
            if (hasLength)
            {
               u32 log = 0;
               if (val)
               {
                  f64 l = log10(Abs(val));
                  log = (l > 0) ? (u32)l : 0;
               }
               u32 length =  (log + 1 + 1 + tailSize);
               if (desiredLength > length)
               {
                  u32 delta = desiredLength - length;
                  
                  if (rightBound)
                  {
                     SpaceString(delta, arena);
                     FtoS((f32)val, tailSize, arena);
                  }
                  else
                  {
                     FtoS((f32)val, tailSize, arena);
                     SpaceString(delta, arena);
                  }
                  
               }
               else
               {
                  FtoS((f32)val, tailSize, arena);
               }
            }
            else
            {
               FtoS((f32)val, tailSize, arena);
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
            
            u32 stringLength = val.length;
            
            if (hasLength)
            {
               i32 deltaLength = (i32)desiredLength - (i32)stringLength;
               if (deltaLength < 0)
               {
                  CopyString(val, arena);
               }
               else
               {
                  if (rightBound)
                  {
                     SpaceString(deltaLength, arena);
                     CopyString(val, arena);
                  }
                  else
                  {
                     CopyString(val, arena);
                     SpaceString(deltaLength, arena);
                  }
               }
            }
            else
            {
               CopyString(val, arena);
            }
            
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
                  char *cval = va_arg(args, char *);
                  String val = S(cval);
                  
                  u32 stringLength = val.length;
                  
                  if (hasLength)
                  {
                     i32 deltaLength = (i32)desiredLength - (i32)stringLength;
                     if (deltaLength < 0)
                     {
                        CopyString(val, arena);
                     }
                     else
                     {
                        if (rightBound)
                        {
                           SpaceString(deltaLength, arena);
                           CopyString(val, arena);
                        }
                        else
                        {
                           CopyString(val, arena);
                           SpaceString(deltaLength, arena);
                        }
                     }
                  }
                  else
                  {
                     CopyString(val, arena);
                  }
                  
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

// % number rb type : makes a string that is number long and right bound / left bound. leftbound on default


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

static char* FormatCString(char *format, ...)
{
	va_list argList;
	va_start(argList, format);
	String ret = StringFormatHelper(frameArena, format, argList);
	PushZeroStruct(frameArena, char);
	va_end(argList);
	return ret.cstr;
}


#endif
