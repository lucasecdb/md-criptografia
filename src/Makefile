CC=g++

SRC=pcm.cpp main.cpp
EXE=main

LIBS=$(pkg-config --libs libcrypto++)
CFLAGS=$(pkg-config --cflags libcrypto++)
HDRS=pcm.h
OBJS=$(SRC:.cpp=.o)

all: $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LIBS)

$(OBJS): $(HDRS)

clean:
	rm -rf *.o core $(EXE)
