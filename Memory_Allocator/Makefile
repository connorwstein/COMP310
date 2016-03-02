# set the compiler to gcc
CC=gcc 
# include directory will have all the .h files
IDIR=./include

SDIR=./src

#-I$(IDIR) says to look in the IDIR directory for the header files
CFLAGS=-I$(IDIR) -Wall   
#general structure is target: prerequisites
							#commands

#typing make will check the first rule and if the prerequistes are also targets it will execute those first
#$^ refers to the prequistes and $@ refers to the target
#object file contains machine code of header and c file, but is not linked with library files yet
OBJ=my_malloc.o my_malloc_test.o


my_malloc: $(OBJ) 					
	$(CC) -o $@ $(OBJ) $(CFLAGS)

my_malloc.o: $(SDIR)/my_malloc.c $(IDIR)/my_malloc.h
	$(CC) -c $(SDIR)/my_malloc.c $(CFLAGS)

my_malloc_test.o: $(SDIR)/my_malloc_test.c $(IDIR)/my_malloc.h
	$(CC) -c $(SDIR)/my_malloc_test.c $(CFLAGS)

clean:
	rm *.o my_malloc
