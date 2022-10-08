splix: splix.c
	cc splix.c -o splix.out `pkg-config --libs --cflags raylib`
	./splix.out
