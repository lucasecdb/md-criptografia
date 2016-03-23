CC=g++

SRC=pcm.cpp main.cpp
EXE=main

LIBS=
HDRS=pcm.h
OBJ=$(SRC:.cpp=.o)

all: $(OBJ) $(HDRS)
	$(CC) $(FLAGS) -o $(EXE) $(OBJ) $(LIBS)

$(OBJ): $(HDRS)

clean:
	rm -rf *.o core $(EXE)
