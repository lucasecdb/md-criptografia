CC=g++

SRC=pcm.cpp main.cpp
EXE=main

LIBS=$(shell pkg-config --libs libcrypto++)
CFLAGS=$(shell pkg-config --cflags libcrypto++) -std=c++11
HDRS=pcm.h
OBJS=$(SRC:.cpp=.o)

all: $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LIBS)

$(OBJS): $(HDRS)

clean:
	rm -rf *.o core $(EXE)
