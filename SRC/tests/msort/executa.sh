gcc makeEntries.c -o makeEntries
gcc mergeAnahy.c -o mergeAnahy -lathread
./makeEntries 500000 > input
./mergeAnahy 16 < input
