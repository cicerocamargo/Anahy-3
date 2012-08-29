#include <stdio.h>

template<class T>
class Elem {
public:
	Elem(T& __element, Elem __next, Elem __previous) {
		element = __element;
		next = __next;
		previous = __previous;
	}

	Elem* next;
	Elem* previous;
	T& element;
};

template<class T>
class list {
	int __size;
	Elem<T> *first, *last;

public:
	list() {
		__size = 0;
		first = last = NULL;
	}

	T& front() {
		return first->element;
	}

	void pop_front() {
		Elem<T>* e = first;
		first = first->next;
		first->previous = NULL;
		delete e;
		__size--;
	}

	void push_back(T& elem) {
		Elem<T>* e = new Elem<T>(elem, NULL, NULL);

		if (__size == 0) {
			first = e;
			last = e;
		} else if (__size == 1) {
			last = e;
			first->next = last;
			last->previous = first;
		} else {
			last->next = e;
			e->previous = last;
			last = e;
		}
		__size++;
	}

	int size() {
		return __size;
	}

	bool empty() {
		return __size == 0 ? true : false;
	}

};

int main() {
	int x = 1, y = 2, z = 3;
	list<int> l;
	l.push_back(x);
	l.push_back(y);
	l.push_back(z);
	printf("front: %d\n", l.front());
	printf("size: %d\n", l.size());
	printf("empty? %s\n", l.empty() ? "yes" : "no" );
	l.pop_front();
	printf("front: %d\n", l.front());
	printf("size: %d\n", l.size());
	printf("empty? %s\n", l.empty() ? "yes" : "no" );
	l.pop_front();
	l.pop_front();

	printf("size: %d\n", l.size());
	printf("empty? %s\n", l.empty() ? "yes" : "no" );
	return 0;
}