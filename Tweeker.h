


enum TweekerType
{
	Tweeker_Invalid,
	Tweeker_b32,
	Tweeker_u32,
	Tweeker_f32,
	Tweeker_v2,
	Tweeker_v3,
	Tweeker_v4,
   
	Tweeker_v3i,
	Tweeker_EulerAngle,
	Tweeker_String,
	Tweeker_EntityType,
};

union TweekerValue
{
	u32 u;
	f32 f;
	v2 vec2;
	v3 vec3;
	v4 vec4;
	b32 b;
};

static TweekerValue CreateTweekerValue()
{
	return {};
}
static TweekerValue CreateTweekerValue(u32 u)
{
	TweekerValue ret = {};
	ret.u = u;
	return ret;
}
static TweekerValue CreateTweekerValue(f32 f)
{
	TweekerValue ret = {};
	ret.f = f;
	return ret;
}
static TweekerValue CreateTweekerValue(v2 vec2)
{
	TweekerValue ret = {};
	ret.vec2 = vec2;
	return ret;
}
static TweekerValue CreateTweekerValue(v3 vec3)
{
	TweekerValue ret = {};
	ret.vec3 = vec3;
	return ret;
}
static TweekerValue CreateTweekerValue(v4 vec4)
{
	TweekerValue ret = {};
	ret.vec4 = vec4;
	return ret;
}
static TweekerValue CreateTweekerValue(b32 b)
{
	TweekerValue ret = {};
	ret.b = b;
	return ret;
}


struct Tweeker //todo stupid way for now, if this is to slow hash table. @scope speedup, part3 minute 30
{
	TweekerType type;
	String name; // not sure, we could make these char* as they do not change after compile time... and its more speedy
	String function;
   
	union // could make this not a union, but its okay like this we can have an array and maybe hash easier later
	{
		TweekerValue value;
		u32 u;
		f32 f;
		v2 vec2;
		v3 vec3;
		v4 vec4;
		b32 b;
	};
};

static Tweeker CreateTweeker(TweekerType type, char *name, TweekerValue value)
{
	Tweeker ret;
	//ret.function;
	ret.name = CreateString(name);
	ret.type = type;
	ret.value = value;
   
	return ret;
}

DefineDynamicArray(Tweeker);


#define Tweekable1(type, name, initalValue) type name = *(type *)MaybeAddTweekerReturnValue(#name, Tweeker_##type, __FUNCTION__, CreateTweekerValue(initalValue));
#define Tweekable2(type, name) type name = *(type *)MaybeAddTweekerReturnValue(#name, Tweeker_##type, __FUNCTION__);


#define Tweekable(...) Expand(GET_MACRO3(__VA_ARGS__, Tweekable1, Tweekable2, Die)(__VA_ARGS__))

static void *MaybeAddTweekerReturnValue(char *_name, TweekerType type, char *function);
static void *MaybeAddTweekerReturnValue(char *_name, TweekerType type, char *function, TweekerValue value);


struct TweekerData
{
	File *tweekerFile;
	TweekerDynamicArray tweekers;
};

static TweekerData globalTweekers;

// needed for draw tweekers
struct TweekerRenderList
{
	Tweeker *tweeker;
	TweekerRenderList *next;
};

struct DebugUITweekerFile
{
	Char *function; // we do pointer compare
	TweekerRenderList *first;
	DebugUITweekerFile *next;
};


static Tweeker *GetTweeker(String name)
{
	For(globalTweekers.tweekers)
	{
		if (name == it->name)
		{
			return it;
		}
	}
	return NULL;
}

static void Tweek(Tweeker t)
{
	Tweeker *toAlter = GetTweeker(t.name);
	if (toAlter)
	{
		toAlter->vec4 = t.vec4;
	}
	else
	{
		Die;
	}
}

static String TweekerToString(Tweeker t, Arena *arena = frameArena)
{
	switch (t.type)
	{
      case Tweeker_b32:
      {
         if (t.b)
         {
            return S("true", arena);
         }
         return S("false", arena);
         
      }break;
      case Tweeker_u32:
      {
         return UtoS(t.u, arena);
      }break;
      case Tweeker_f32:
      {
         return FtoS(t.f, arena);
      }break;
      
      case Tweeker_v2:
      {
         return V2toS(t.vec2, arena);
      }break;
      
      case Tweeker_v3:
      {
         return V3toS(t.vec3, arena);
      }break;
      case Tweeker_v4:
      {
         return V4toS(t.vec4, arena);
      }break;
      default:
		break;
	}
	return S("");
}
