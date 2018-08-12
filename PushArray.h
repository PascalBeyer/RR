#ifndef RR_PUSHVECTOR
#define RR_PUSHVECTOR

template<class T>
class PushArray
{
public:
	PushArray<T>()
	{
		maxSize = 0;
		size = 0;
		pushArray = NULL;
	};

	PushArray<T>(unsigned int maxSize)
	{
		this->maxSize = maxSize;
		size = 0;
		pushArray = new T[maxSize];
	};
	PushArray<T>(const PushArray<T> &copy)
	{
		this->maxSize = copy->maxSize;
		this->size = copy->size;
		pushArray = new T[maxSize];
		for (unsigned int i = 0; i < maxSize; i++)
		{
			pushArray[i] = copy->pushArray[i];
		}
	};
	~PushArray<T>()
	{
		delete[] pushArray;
	};

	void Clear()
	{
		size = 0;
	};
	void PushBack(T elem)
	{
		pushArray[size++] = elem;
	};
	T Get(unsigned int i)
	{
		return pushArray[i];
	};
	unsigned int Size()
	{
		return size;
	};
	T *pushArray;
	unsigned int maxSize;
	unsigned int size;
};


#endif // !RR_PUSHVECTOR

