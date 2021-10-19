CC=gcc
CFLAGS=-c -Wall -Wextra

SOURCES=main.c Cane.c
EXECUTABLE=Cane

OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o main

main.o: main.c
	$(CC) $(FLAGS) main.c

BizzBuzz.o: Cane.c
	$(CC) $(CFLAGS) Cane.c

clean:
	rm -rf *.o Cane

