#ifndef _ANAHY_SMART_STACK_H
#define _ANAHY_SMART_STACK_H

#include <cstdlib>
#include <cstdio>

template<class T>
class AnahySmartStack {
	unsigned long _current_size;
	unsigned long _upper_bound;
	unsigned long _lower_bound;
	unsigned long _min_size;
	T* _array;

public:
	AnahySmartStack(int size_to_reserve=1024) {
		if (size_to_reserve < 12) {
			size_to_reserve = 12;
		}
		_min_size = size_to_reserve;
		_upper_bound = size_to_reserve;
		_lower_bound = _upper_bound/4;
		_current_size = 0;
		_array = (T*)malloc(_upper_bound*sizeof(T));
	}

	~AnahySmartStack() {
#ifdef DEBUG
		std::cout << "~AnahySmartStack called" << std::endl;
#endif
		free(_array);
	}

	unsigned long size() { return _current_size; }
	unsigned long max_size() { return _upper_bound; }
	unsigned long min_size() { return _lower_bound; }

	bool empty() { return !_current_size; }
	T top() { return _array[_current_size-1]; }

	void push(T value) {
		if (_current_size == _upper_bound) {
			_upper_bound *= 2;
			_lower_bound *= 2;
			T* tmp = (T*)realloc(_array, _upper_bound*sizeof(T));
			if (tmp) {
				_array = tmp;

#ifdef DEBUG

				printf("reallocating: max_size=%lu min_size=%lu\n", _upper_bound, _lower_bound);

#endif
			} else {
				fprintf(stderr, "Error reallocating space for Anahy stack.\n");
				abort();
			}
		}
		_array[_current_size++] = value;
	}



	void pop() {
		if (_current_size == _lower_bound && _lower_bound > _min_size) {
			_upper_bound /= 2;
			_lower_bound /= 2;
			T* tmp = (T*)realloc(_array, _upper_bound*sizeof(T));
			if (tmp) {
				_array = tmp;
#ifdef DEBUG				
				printf("reallocating: max_size=%lu min_size=%lu\n", _upper_bound, _lower_bound);
#endif
			} else {
				fprintf(stderr, "Error reallocating space for Anahy stack.\n");
				abort();
			}
		}
		_current_size--;
	}
};

#endif