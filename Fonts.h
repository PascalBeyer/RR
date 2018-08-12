#ifndef RR_FONTS
#define RR_FONTS


enum FontEnum
{
	Font_A,
	Font_B,
	Font_C,
	Font_D,
	Font_E,
	Font_F,
	Font_G,
	Font_H,
	Font_I,
	Font_J,
	Font_K,
	Font_L,
	Font_M,
	Font_N,
	Font_O,
	Font_P,
	Font_Q,
	Font_R,
	Font_S,
	Font_T,
	Font_U,
	Font_V,
	Font_W,
	Font_X,
	Font_Y,
	Font_Z,

	Font_0,
	Font_1,
	Font_2,
	Font_3,
	Font_4,
	Font_5,
	Font_6,
	Font_7,
	Font_8,
	Font_9,

	Font_Aus,
	Font_Anf,
	Font_DoppelPunkt,
	Font_Fragezeichen,
	Font_Groeser,
	Font_Kleiner,
	Font_Komma,
	Font_Leerzeichen,
	Font_Punkt,
	Font_Stern,
	Font_Minus,

	Font_Count,
	Font_InvalideID
};


inline FontEnum GetCharIndex(char a)
{
	switch (a)
	{

	case 'a' : { return Font_A;			  }break;
	case 'b' : { return Font_B;			  }break;
	case 'c' : { return Font_C;			  }break;
	case 'd' : { return Font_D;			  }break;
	case 'e' : { return Font_E;			  }break;
	case 'f' : { return Font_F;			  }break;
	case 'g' : { return Font_G;			  }break;
	case 'h' : { return Font_H;			  }break;
	case 'i' : { return Font_I;			  }break;
	case 'j' : { return Font_J;			  }break;
	case 'k' : { return Font_K;			  }break;
	case 'l' : { return Font_L;			  }break;
	case 'm' : { return Font_M;			  }break;
	case 'n' : { return Font_N;			  }break;
	case 'o' : { return Font_O;			  }break;
	case 'p' : { return Font_P;			  }break;
	case 'q' : { return Font_Q;			  }break;
	case 'r' : { return Font_R;			  }break;
	case 's' : { return Font_S;			  }break;
	case 't' : { return Font_T;			  }break;
	case 'u' : { return Font_U;			  }break;
	case 'v' : { return Font_V;			  }break;
	case 'w' : { return Font_W;			  }break;
	case 'x' : { return Font_X;			  }break;
	case 'y' : { return Font_Y;			  }break;
	case 'z' : { return Font_Z;			  }break;
	case 'A': { return Font_A;			  }break;
	case 'B': { return Font_B;			  }break;
	case 'C': { return Font_C;			  }break;
	case 'D': { return Font_D;			  }break;
	case 'E': { return Font_E;			  }break;
	case 'F': { return Font_F;			  }break;
	case 'G': { return Font_G;			  }break;
	case 'H': { return Font_H;			  }break;
	case 'I': { return Font_I;			  }break;
	case 'J': { return Font_J;			  }break;
	case 'K': { return Font_K;			  }break;
	case 'L': { return Font_L;			  }break;
	case 'M': { return Font_M;			  }break;
	case 'N': { return Font_N;			  }break;
	case 'O': { return Font_O;			  }break;
	case 'P': { return Font_P;			  }break;
	case 'Q': { return Font_Q;			  }break;
	case 'R': { return Font_R;			  }break;
	case 'S': { return Font_S;			  }break;
	case 'T': { return Font_T;			  }break;
	case 'U': { return Font_U;			  }break;
	case 'V': { return Font_V;			  }break;
	case 'W': { return Font_W;			  }break;
	case 'X': { return Font_X;			  }break;
	case 'Y': { return Font_Y;			  }break;
	case 'Z': { return Font_Z;			  }break;

	case '0': { return Font_0;			  }break;
	case '1': { return Font_1;			  }break;
	case '2': { return Font_2;			  }break;
	case '3': { return Font_3;			  }break;
	case '4': { return Font_4;			  }break;
	case '5': { return Font_5;			  }break;
	case '6': { return Font_6;			  }break;
	case '7': { return Font_7;			  }break;
	case '8': { return Font_8;			  }break;
	case '9': { return Font_9;			  }break;

	case '!': { return Font_Aus;		  }break;
	case '"': { return Font_Anf;		  }break;
	case ':': { return Font_DoppelPunkt; }break;
	case '?': { return Font_Fragezeichen; }break;
	case '>': { return Font_Groeser;	  }break;
	case '<': { return Font_Kleiner;	  }break;
	case ',': { return Font_Komma;		  }break;
	case ' ': { return Font_Leerzeichen; }break;
	case '.': { return Font_Punkt;		  }break;
	case '*': { return Font_Stern;       }break;
	case '-': {return Font_Minus; }break;


	default:
		return Font_InvalideID;
		break;
	}
}

inline float FontGetActualFloatWidth(FontEnum fontIndex, float size)
{
	float ret = 0.0f;
	switch (fontIndex)
	{
	case Font_0:
	{
		ret =  30.0f;
	}break;
	case Font_1:
	{
		ret =  30.0f;
	}break;
	case Font_2:
	{
		ret =  30.0f;
	}break;
	case Font_3:
	{
		ret =  30.0f;
	}break;
	case Font_4:
	{
		ret =  30.0f;
	}break;
	case Font_5:
	{
		ret =  30.0f;
	}break;
	case Font_6:
	{
		ret =  30.0f;
	}break;
	case Font_7:
	{
		ret =  30.0f;
	}break;
	case Font_8:
	{
		ret =  30.0f;
	}break;
	case Font_9:
	{
		ret =  30.0f;
	}break;

	case Font_A:
	{
		ret =  50.0f;
	}break;
	case Font_B:
	{
		ret =  50.0f;
	}break;
	case Font_C:
	{
		ret =  50.0f;
	}break;
	case Font_D:
	{
		ret =  50.0f;
	}break;
	case Font_E:
	{
		ret =  50.0f;
	}break;
	case Font_F:
	{
		ret =  50.0f;
	}break;
	case Font_G:
	{
		ret =  50.0f;
	}break;
	case Font_H:
	{
		ret =  50.0f;
	}break;
	case Font_I:
	{
		ret =  50.0f;
	}break;
	case Font_J:
	{
		ret =  50.0f;
	}break;
	case Font_K:
	{
		ret =  50.0f;
	}break;
	case Font_L:
	{
		ret =  50.0f;
	}break;
	case Font_M:
	{
		ret =  50.0f;
	}break;
	case Font_N:
	{
		ret =  50.0f;
	}break;
	case Font_O:
	{
		ret =  50.0f;
	}break;
	case Font_P:
	{
		ret =  50.0f;
	}break;
	case Font_Q:
	{
		ret =  50.0f;
	}break;
	case Font_R:
	{
		ret =  50.0f;
	}break;
	case Font_S:
	{
		ret =  50.0f;
	}break;
	case Font_T:
	{
		ret =  50.0f;
	}break;
	case Font_U:
	{
		ret =  50.0f;
	}break;
	case Font_V:
	{
		ret =  50.0f;
	}break;
	case Font_W:
	{
		ret =  50.0f;
	}break;
	case Font_X:
	{
		ret =  50.0f;
	}break;
	case Font_Y:
	{
		ret =  50.0f;
	}break;
	case Font_Z:
	{
		ret =  50.0f;
	}break;

	case Font_Aus: 
	{ 
		ret =  50.0f;		  
	}break;
	case Font_Anf: 
	{ 
		ret =  50.0f;		  
	}break;
	case Font_DoppelPunkt: 
	{ 
		ret =  50.0f; 
	}break;
	case Font_Fragezeichen: 
	{ 
		ret =  50.0f; 
	}break;
	case Font_Groeser: 
	{ 
		ret =  50.0f;	  
	}break;
	case Font_Kleiner: 
	{ 
		ret =  50.0f;	  
	}break;
	case Font_Komma: { ret =  50.0f;		  }break;
	case Font_Leerzeichen: { ret =  50.0f; }break;
	case Font_Punkt: { ret =  20.0f;		  }break;
	case Font_Stern: { ret =  50.0f;       }break;


	default:
	{
		ret =  0.0f;
	}break;

	}

	ret = ret / 50.0f * size;

	return ret;
}

#endif
