all: trace

trace: trace.c LC4.c ObjectFiles.c LC4.h ObjectFiles.h
	clang -Wall -o trace trace.c LC4.c ObjectFiles.c

clean:
	rm *.o trace

.PHONY: all clean