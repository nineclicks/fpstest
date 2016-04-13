# cs335 fpstest
# to compile your project, type make and press enter

all: fpstest

fpstest: fpstest.cpp
	g++ fpstest.cpp libggfonts.a -Wall -ofpstest -lX11 -lGL -lGLU -lm

clean:
	rm -f fpstest
	rm -f *.o

