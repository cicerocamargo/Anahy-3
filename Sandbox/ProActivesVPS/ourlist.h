#ifndef OURLIST_H
#define OURLIST_H

#include "Job.h"
#include "JobId.h"
#include <cassert>
#include <iostream>

template<class Type>
class ListNode {

public:

	ListNode<Type>* next;
	ListNode<Type>* previous;
	Type value;

	ListNode(Type value) {
		this->value = value;
		next = previous = NULL;
	}
};

template<class Type>
class mList {

	int __size;
	ListNode<Type>* first;
	ListNode<Type>* last;

public:
	mList() {
		std::cout << "node size: " << sizeof(ListNode<Type>) << std::endl;
		__size = 0;
		first = last = NULL;
	}

	Type& front() {
		return first->value;
	}

	Type& back() {
		return last->value;
	}

	void pop_front() {
		if (__size == 1) {
			delete first;
			first = last = NULL;
		} else {
			ListNode<Type>* deadNode = first;
			first = first->next;
			first->previous = NULL;
			delete deadNode;
		}
		__size--;
	}

	void pop_back() {
		if (__size == 1) {
			delete first;
			first = last = NULL;
		} else {
			ListNode<Type>* deadNode = last;
			last = last->previous;
			last->next = NULL;
			delete deadNode;
		}
		__size--;
	}

	void push_front(Type elem) {
		ListNode<Type>* newNode = new ListNode<Type>(elem);

		if (__size == 0) {
			first = last = newNode;
		} else  {
			assert(first != NULL);
			first->previous = newNode;
			newNode->next = first;
			first = newNode;
		}
		__size++;
	}

	void push_back(Type elem) {
		ListNode<Type>* newNode = new ListNode<Type>(elem);

		if (__size == 0) {
			first = last = newNode;
		} else {
			assert(last != NULL);
			last->next = newNode;
			newNode->previous = last;
			last = newNode;
		}
		__size++;
	}

	int size() {
		return __size;
	}

	bool empty() {
		return __size == 0 ? true : false;
	}

	void print() {
		std::cout << "[";
		ListNode<Type>* it = first;
		while (it != NULL) {
			std::cout << " " << it->value;
			if (it->next != NULL) {
				std::cout << ",";
			}
			it = it->next;
		}
		std::cout << "]" << std::endl;
		std::cout << __size << " elements." << std::endl;
	}
};

#endif