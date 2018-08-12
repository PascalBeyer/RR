#ifndef RR_TEMPLATES
#define RR_TEMPLATES
#include "BasicTypes.h"

template <class T>
T* map(T* arr, T(*funct)(T inp), u32 size)
{
	for(u32 i = 0;  i < size; i++)
	{
		arr[i] = funct(arr[i]);
	}
}

#endif // !RR_ARRAYTRANSFORMS

