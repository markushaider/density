
CC = gcc
CFLAGS= -DH5_USE_16_API

OBJECTS = main.o
TARGET = main
LIBS =  -lm -lhdf5
INCLUDES = 

$(TARGET): $(OBJECTS) 
	$(CC) $(CFLAGS)  $(OBJECTS) -o $(TARGET) $(LIBS)

$(OBJECTS) : $(INCLUDES) Makefile

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)
