#define checkResults(string, val) {								\
	if (val) {															\
		printf("Failed with code %d at %s", val, string);	\
		exit(1);															\
	}																		\
}
