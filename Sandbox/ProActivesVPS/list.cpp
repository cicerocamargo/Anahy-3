#include <stdio.h>

template<class Type>
class Elem {
public:
	Elem(Type __element, Elem<Type>* __next) {//, Elem<Type>* __previous) {
		element = __element;
		next = __next;
		//previous = __previous;
	}

	Elem<Type>* next;
	//Elem<Type>* previous;
	Type element;
};

template<class Type>
class list {
private:
	int __size;
	Elem<Type>* first;
	Elem<Type>* last;

public:
	list() {
		__size = 0;
		first = last = NULL;
	}

	Type& front() {
		return first->element;
	}

	void pop_front() {
		Elem<Type>* e;
		if (__size == 0) {
			return;
		} else {
			e = first;
			first = first->next;
			//first->previous = first;
			if (__size == 1) {
				last = NULL;
			}
			delete e;
		}
		__size--;
	}

	void push_back(Type elem) {
		Elem<Type>* e = new Elem<Type>(elem, NULL);//, NULL);

		if (__size == 0) {
			first = e;
			last = e;
		} else if (__size == 1) {
			last = e;
			first->next = last;
			//last->previous = first;
		} else {
			//e->previous = last;
			last->next = e;
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