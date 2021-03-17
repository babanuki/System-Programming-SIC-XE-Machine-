CC = gcc
CFLAGS = -W -Wall
TARGET = 20151602.out
OBJECTS = assemble.o commandfunction_basic.o command.o hashtable.o linkedlist.o commandfunction_mem.o 20151602.o

all : $(TARGET)

$(TARGET): $(OBJECTS)
			 $(CC) $(CFLAGS) -o $@ $^

clean :
	rm *.o 20151602.out
