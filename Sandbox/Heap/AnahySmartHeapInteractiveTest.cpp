#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <list>
#include "AnahyJob.h"
#include "AnahySmartHeap.h"
#define NUM_OPERATIONS 10000

typedef enum {
    Insert,
    ExtractBack,
    ExtractFront 
} HeapOperation;

void* foo(void* args) {
    return NULL;
}

int main() {
   AnahySmartHeap<AnahyJob*> heap(4);
   std::list<AnahyJob*> l;
   AnahyJob* j;
   AnahyJob* k;

   int match_counter = 0, mismatch_counter = 0;

   srand(time(NULL));
   int op = rand() % 3; 
   for (int i = 0; i < NUM_OPERATIONS; ++i, op = rand() % 3) {
   
           switch (op) {
            case Insert: 
                j = new AnahyJob(foo, 0, 0);
                heap.insert(j);
                l.push_back(j);
                break;
            case ExtractFront:
                j = heap.extract_min();
                if (j == 0) {
                    break;
                }
                k = l.front();
                l.pop_front();
                if (j == k) {
                    match_counter++;
                } else {
                    mismatch_counter++;
                }
                break;
            case ExtractBack:
                j = heap.extract_last();
                if (j == 0) {
                    break;
                }
                k = l.back();
                l.pop_back();
                if (j == k) {
                    match_counter++;
                } else {
                    mismatch_counter++;
                }
                break;
            default:
                std::cout << "Unknown option\n";
                break;
        }
   }
   std::cout << match_counter << " matches; " << mismatch_counter << " mismatches\n";
   return 0;
}
