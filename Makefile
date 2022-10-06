core_input_keys: core_input_keys.c
	clang core_input_keys.c -o core_input_keys.out `pkg-config --libs --cflags raylib`
