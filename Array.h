#ifndef RR_ARRAY
#define RR_ARRAY



typedef u32* u32Ptr;
DefineDynamicArray(u32Ptr);


typedef v3* v3Ptr;
DefineDynamicArray(v3Ptr);

DefineArray(u16);
typedef u16* u16Ptr;
DefineDynamicArray(u16Ptr);


DefineArray(v3);
DefineArray(v2);
DefineArray(v4);

DefineDynamicArray(v2);
DefineDynamicArray(v3);
DefineDynamicArray(v4);


#endif // !RR_ARRAY

