#include "PushArray.h"
/*
template<class T>
PushArray<T>::PushArray()
{
	this->maxSize = 0;
	size = 0;
	pushArray = NULL;
}

template<class T>
PushArray<T>::PushArray(unsigned int maxSize)
{
	this->maxSize = maxSize;
	size = 0;
	pushArray = new T[maxSize];
}

template<class T>
PushArray<T>::PushArray(const PushArray<T> &copy)
{
	this->maxSize = copy->maxSize;
	this->size = copy->size;
	pushArray = new T[maxSize];
	for (unsigned int i = 0; i < maxSize; i++)
	{
		pushArray[i] = copy->pushArray[i];
	}
}

template<class T>
PushArray<T>::~PushArray()
{
	delete[] pushArray;
}

template<class T>
void PushArray<T>::Clear()
{
	size = 0;
}

template<class T>
T PushArray<T>::Get(unsigned int i)
{
	return pushArray[i];
}
template<class T>
unsigned int PushArray<T>::Size()
{
	return size;
}

template<class T>
void PushArray<T>::PushBack(T elem)
{
	pushArray[size++] = elem;
}
*/