splix: splix.c
	clang splix.c -o splix.out `pkg-config --libs --cflags raylib`
