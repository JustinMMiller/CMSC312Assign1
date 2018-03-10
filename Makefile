CC=gcc
LINKER=gcc
INCLUDES=-I.
CFLAGS=-I. -c -g -Wall $(INCLUDES)
LINKARGS=-lm -g
LIBS=-lm

OBJECT_FILES= printsim.o

printsim : $(OBJECT_FILES)
	$(LINKER) $(LINKARGS) -g $^ -o $@ $(LIBS) -pthread

printsim.o : printsim.c
	$(CC) $(CFLAGS) $< -o $@ -pthread

clean : 
	rm printsim.o
