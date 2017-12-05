# location of CMVision
CMVDIR = ../..

CC = g++
CFLAGS = -O3 -DUSE_METEOR
TARGET = mtest
SRC = $(wildcard *.cc) $(wildcard $(CMVDIR)/src/*.cc)
OBJ = $(SRC:%.cc=%.o)
LIB = -L/usr/X11/lib -lX11
INC = -I$(CMVDIR)/include -I/usr/include/X11


all: $(TARGET)

.SUFFIXES: .cc
%.o: %.cc
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIB)

clean:
	rm -f *.o $(TARGET) $(CMVDIR)/src/*.o core
