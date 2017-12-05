# location of CMVision
CMVDIR = ../..

CC = g++
CFLAGS = -O3
TARGET = grab
SRC = $(wildcard *.cc) $(wildcard $(CMVDIR)/src/*.cc)
OBJ = $(SRC:%.cc=%.o)
LIB =
INC = -I$(CMVDIR)/include


all: $(TARGET)

.SUFFIXES: .cc
%.o: %.cc
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIB)

clean:
	rm -f *.o $(TARGET) core
