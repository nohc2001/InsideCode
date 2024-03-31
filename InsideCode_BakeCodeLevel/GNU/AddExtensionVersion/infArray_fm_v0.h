#ifndef H_UTILL_FREEMEMORY
#include "Utill_FreeMemory.h"
#endif

#include <math.h>
using namespace std;

template < typename T > class circularArray
{
  public:
	int pivot = 0;
	int maxsiz = 10;
	T *arr = nullptr;
	freemem::FM_System0 * fm;

	circularArray()
	{
	}
	
	circularArray(const circularArray<T>& ref)
	{
		pivot = ref.pivot;
		maxsiz = ref.maxsiz;
		arr = ref.arr;
		fm = ref.fm;
	}
	
	virtual ~ circularArray()
	{
	}

	void Init(int max_siz, freemem::FM_System0 * fms)
	{
		fm = fms;
		maxsiz = max_siz;
		arr = (T *) fm->_New(sizeof(T) * maxsiz, true);
		// cout << arr;
		pivot = 0;
	}

	void Release()
	{
		fm->_Delete((byte8 *) arr, sizeof(T) * maxsiz);
		arr = nullptr;
	}

	void move_pivot(int dist)
	{
		pivot = (maxsiz + pivot + dist) % maxsiz;
	}

	T & operator[](int index)
	{
		int realindex = (index + pivot) % maxsiz;
		return arr[realindex];
	}

	void dbg()
	{
		for (int i = 0; i < maxsiz; ++i)
		{
			cout << this->operator[](i) << " ";
		}
		cout << endl;
	}
};

template < typename T > class infArray
{
  public:
	freemem::FM_System0 * fm;
	int fragment_siz = 10;
	int array_depth = 1;
	int array_capacity = 10;
	int array_siz = 0;			// up
	circularArray < int *>*ptrArray = nullptr;

	infArray()
	{
	}
	virtual ~ infArray()
	{
	}

	void Init(int fmgsiz, freemem::FM_System0 * fms)
	{
		fm = fms;
		fragment_siz = fmgsiz;
		array_depth = 1;
		ptrArray = (circularArray < int *>*)fm->_New(sizeof(circularArray < int *>), true);
		ptrArray->Init(fragment_siz, fm);

		circularArray < T > *arr =
			(circularArray < T > *)fm->_New(sizeof(circularArray < T >), true);
		arr->Init(fragment_siz, fm);
		ptrArray->operator[](0) = (int *)arr;
		for (int i = 1; i < fragment_siz; ++i)
		{
			ptrArray->operator[](i) = nullptr;
		}
		array_capacity = fragment_siz;
		array_siz = 0;
	}

	// must test
	void Release()
	{
		for (int k = 0; k < array_depth; ++k)
		{
			int maxn = pow(fragment_siz, array_depth - k);
			for (int n = 0; n < maxn; ++n)
			{
				int seek = n * pow(fragment_siz, k + 1);
				circularArray < int *>*ptr = ptrArray;
				for (int i = 0; i < array_depth - k; ++i)
				{
					ptr =
						(circularArray <
						 int *>*)ptr->operator[]((int)((seek / pow(fragment_siz, array_depth - i)))
												 % fragment_siz);
				}

				if (ptr == nullptr)
				{
					break;
				}

				if (k == 0)
				{
					// most bottom real array
					circularArray < T > *vptr = (circularArray < T > *)ptr;
					vptr->Release();
					fm->_Delete((byte8 *) vptr, sizeof(circularArray < T >));
					// delete vptr;
				}
				else
				{
					// not bottom ptr array
					circularArray < int *>*vptr = (circularArray < int *>*)ptr;
					vptr->Release();
					// delete[]vptr;
					fm->_Delete((byte8 *) vptr, sizeof(circularArray < int *>));
				}
			}
		}

		ptrArray->Release();
		fm->_Delete((byte8 *) ptrArray, sizeof(circularArray < int *>));
		// delete[]ptrArray;
	}

	int get_max_capacity_inthisArr()
	{
		return pow(fragment_siz, array_depth + 1);
	}

	void set(int index, T value)
	{
		T nullv = 0;
		if (index >= array_capacity)
		{
			return;
		}
		circularArray < int *>*ptr = ptrArray;
		for (int i = 0; i < array_depth; ++i)
		{
			ptr =
				(circularArray <
				 int *>*)ptr->operator[]((int)((index / pow(fragment_siz, array_depth - i))) %
										 fragment_siz);
		}
		circularArray < T > *vptr = (circularArray < T > *)ptr;
		// T *vptr = ptr;
		int inindex = ((int)(index / pow(fragment_siz, 0))) % fragment_siz;
		vptr->operator[](inindex) = value;
	}

	void push(T value)
	{
		if (array_siz + 1 <= array_capacity)
		{
			set(array_siz, value);
			// this[array_siz] = value;
			array_siz += 1;
		}
		else
		{
			if (array_siz + 1 > get_max_capacity_inthisArr())
			{
				// create new parent ptr array
				int *chptr = (int *)ptrArray;
				ptrArray = (circularArray < int *>*)fm->_New(sizeof(circularArray < int *>), true);
				ptrArray->Init(fragment_siz, fm);

				ptrArray->operator[](0) = chptr;
				array_depth += 1;
				for (int i = 1; i < fragment_siz; ++i)
				{
					ptrArray->operator[](i) = nullptr;
				}
			}
			// create child ptr arrays
			int next = array_siz;
			circularArray < int *>*ptr = ptrArray;
			int upcapacity = 0;
			for (int i = 0; i < array_depth; ++i)
			{
				int inindex = ((int)(next / pow(fragment_siz, array_depth - i))) % fragment_siz;

				upcapacity += (inindex) * pow(fragment_siz, array_depth - i);

				circularArray < int *>*tptr = ptr;
				ptr = (circularArray < int *>*)tptr->operator[](inindex);
				if (ptr == nullptr)
				{
					if (i == array_depth - 1)
					{
						circularArray < T > *aptr =
							(circularArray < T > *)fm->_New(sizeof(circularArray < T >), true);
						aptr->Init(fragment_siz, fm);
						tptr->operator[](inindex) = (int *)aptr;
						ptr = (circularArray < int *>*)tptr->operator[](inindex);
					}
					else
					{
						circularArray < int *>*insptr =
							(circularArray < int *>*)fm->_New(sizeof(circularArray < int *>),
															  true);
						insptr->Init(fragment_siz, fm);
						tptr->operator[](inindex) = (int *)insptr;

						ptr = (circularArray < int *>*)tptr->operator[](inindex);
					}
				}
			}
			circularArray < T > *vptr = (circularArray < T > *)ptr;
			// T *vptr = ptr;
			int inindex = ((int)(next / pow(fragment_siz, 0))) % fragment_siz;
			upcapacity += fragment_siz;
			vptr->operator[](inindex) = value;
			// capacity update 
			array_capacity = upcapacity;
			array_siz += 1;
		}
	}

	void pop_back()
	{
		T nullt = 0;
		set(array_siz - 1, nullt);
		array_siz -= 1;
	}

	T & operator[](int index)
	{
		T nullv = 0;
		if (index >= array_capacity)
		{
			cout << "error! array index bigger than capacity!" << endl;
			return nullv;
		}
		circularArray < int *>*ptr = ptrArray;
		for (int i = 0; i < array_depth; ++i)
		{
			ptr =
				(circularArray <
				 int *>*)ptr->operator[]((int)((index / pow(fragment_siz, array_depth - i))) %
										 fragment_siz);
		}
		circularArray < T > *vptr = (circularArray < T > *)ptr;

		// T *vptr = ptr;
		int inindex = ((int)(index / pow(fragment_siz, 0))) % fragment_siz;
		return vptr->operator[](inindex);
	}

	circularArray < T > *get_bottom_array(int index)
	{
		if (index >= array_capacity)
		{
			return nullptr;
		}
		circularArray < int *>*ptr = ptrArray;
		for (int i = 0; i < array_depth; ++i)
		{
			ptr =
				(circularArray <
				 int *>*)ptr->operator[]((int)((index / pow(fragment_siz, array_depth - i))) %
										 fragment_siz);
		}
		circularArray < T > *vptr = (circularArray < T > *)ptr;
		return vptr;
	}

	circularArray < int *>*get_ptr_array(int index, int height)
	{
		if (index >= array_capacity)
		{
			return nullptr;
		}
		circularArray < int *>*ptr = ptrArray;
		for (int i = 0; i < array_depth - height; ++i)
		{
			ptr =
				(circularArray <
				 int *>*)ptr->operator[]((int)((index / pow(fragment_siz, array_depth - i))) %
										 fragment_siz);
		}
		return ptr;
	}

	// direction : -1 or 1
	void move(int index, int direction, bool expend)
	{
		T save;
		circularArray < T > *corearr = get_bottom_array(index);
		int inindex = index % fragment_siz;
		int last = 0;
		if (direction > 0)
		{
			last = fragment_siz - direction;
		}
		save = corearr->operator[](last);

		if (direction > 0)
		{
			for (int i = last; i >= inindex; --i)
			{
				corearr->operator[](i) = corearr->operator[](i - 1);
			}
		}
		else
		{
			for (int i = inindex - 1; i < fragment_siz; ++i)
			{
				corearr->operator[](i) = corearr->operator[](i + 1);
			}
		}


		if (direction > 0)
		{
			int next = index;
			while (true)
			{
				next = ((int)(next / fragment_siz) + 1) * fragment_siz;
				circularArray < T > *temparr = get_bottom_array(next);

				if (temparr == nullptr)
				{
					if (expend)
					{
						push(save);
					}
					break;
				}

				circularArray < T > *nextarr = nullptr;
				nextarr = get_bottom_array(next + fragment_siz);
				T ss;
				if (nextarr == nullptr)
				{
					int ind = (array_siz - 1) % fragment_siz;
					ss = temparr->operator[](ind);
				}
				else
				{
					ss = temparr->operator[](fragment_siz - 1);
				}

				temparr->move_pivot(-direction);
				if (direction > 0)
				{
					temparr->operator[](0) = save;
				}
				else
				{
					temparr->operator[](fragment_siz - 1) = save;
				}

				save = ss;
			}
		}
		else
		{
			int next = array_siz-1 + fragment_siz;
			while (true)
			{
				next = ((int)(next / fragment_siz) - 1) * fragment_siz;
				if (next < 0)
					break;
				circularArray < T > *temparr = get_bottom_array(next);
				
				if(temparr == nullptr){
					continue;
				}
				
				T ss;
				if ((next - fragment_siz) / fragment_siz == index / fragment_siz)
				{
					int ind = (array_siz - 1) % fragment_siz;
					ss = temparr->operator[](0);

					if (next + fragment_siz >= array_siz)
					{
						temparr->move_pivot(-direction);
						array_siz -= 1;
					}
					else
					{
						temparr->move_pivot(-direction);
						temparr->operator[](fragment_siz - 1) = save;
					}

					save = ss;

					corearr->operator[](fragment_siz - 1) = ss;
					break;
				}
				else
				{
					ss = temparr->operator[](0);
				}

				if (next + fragment_siz >= array_siz)
				{
					temparr->move_pivot(-direction);
					array_siz -= 1;
				}
				else
				{
					temparr->move_pivot(-direction);
					temparr->operator[](fragment_siz - 1) = save;
				}

				save = ss;
			}
		}

	}

	void printstate(char sig)
	{
		cout << sig << "_" << "arr siz : " << array_siz << "[ ";
		for (int u = 0; u < array_siz; ++u)
		{
			if (u % fragment_siz == 0)
			{
				int uu = u;
				cout << ">";
				for (int k = 0; k < array_depth; ++k)
				{
					uu /= fragment_siz;
					if (uu % fragment_siz == 0)
					{
						cout << ">";
					}
				}
			}
			cout << this->operator[](u) << ", ";
		}
		cout << "]" << endl;
	}

	void insert(int index, T value, bool expend)
	{
		move(index, 1, expend);
		set(index, value);
	}

	void erase(int index)
	{
		if (index + 1 == array_siz)
		{
			T nullt = 0;
			set(index, nullt);
			array_siz -= 1;
		}
		else
		{
			move(index + 1, -1, false);
		}
	}
};
