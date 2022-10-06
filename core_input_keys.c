#include <stdlib.h>
#include <assert.h>
#include "raylib.h"

#include <stdio.h>

#define ROWS 30
#define COLUMNS 60
#define SQUARE_SIDE 20
#define STEPS_PER_SQUARE 5

enum direction{Still, Up, Down, Left, Right};
typedef enum direction direction;

struct player {
	int x;
	int y;
	direction head_direction;
	direction next_direction;
	int id; // id field might be replaced by index in "players" list
	Color color_main;
	Color color_second
};
typedef struct player player;

void print_clr(Color clr) {
	printf("r: %d, g: %d, b: %d, a: %d\n", clr.r, clr.g, clr.b, clr.a);
}

void arrow_input (direction *dir) {
	if (IsKeyDown(KEY_RIGHT)) *dir = Right;
	if (IsKeyDown(KEY_LEFT)) *dir = Left;
	if (IsKeyDown(KEY_UP)) *dir = Up;
	if (IsKeyDown(KEY_DOWN)) *dir = Down;
}

void wasd_input (direction *dir) {
	if (IsKeyDown(KEY_D)) *dir = Right;
	if (IsKeyDown(KEY_A)) *dir = Left;
	if (IsKeyDown(KEY_W)) *dir = Up;
	if (IsKeyDown(KEY_S)) *dir = Down;
}

void move_player(player *p, const int step) {
		switch ((*p).head_direction) {
			case Right:
				(*p).x += step;
				break;
			case Left:
				(*p).x -= step;
				break;
			case Up:
				(*p).y -= step;
				break;
			case Down:
				(*p).y += step;
				break;
			case Still:
				break;
		}
}

void check_grid(int grid[ROWS][COLUMNS]) {
	for (int row = 0; row < ROWS; ++row) {
		for (int col = 0; col < COLUMNS; ++col) {
			assert(
				(grid[row][col] == 0)
			);
		}
	}
}

void draw(int **grid, player players[], int players_len) {
	int tile; // tile value
	Color clr;

	BeginDrawing();
		for (int row = 0; row < ROWS; ++row) {
			for (int col = 0; col < COLUMNS; ++col) {
				tile = grid[row][col];
				if (tile == 0) {
					clr = RAYWHITE;
				}
				else if (tile < 0) {
					 clr = players[-tile-1].color_second;
				} else if (tile > 0) {
					 clr = players[tile-1].color_main;
				}
				DrawRectangle(col * SQUARE_SIDE, row * SQUARE_SIDE, SQUARE_SIDE, SQUARE_SIDE, clr);
			}
		}
		for (int i = 0; i<players_len; ++i) {
			DrawRectangle(
				players[i].x,
				players[i].y,
				SQUARE_SIDE,
				SQUARE_SIDE,
				players[i].color_main
			);
		}
	EndDrawing();
}

int main(void)
{
	// dimensions
	int head_step = SQUARE_SIDE / STEPS_PER_SQUARE;
	assert(SQUARE_SIDE % head_step == 0);

	const int screenWidth = COLUMNS * SQUARE_SIDE;
	const int screenHeight = ROWS * SQUARE_SIDE;

	// grid is 2d array
	int** grid = malloc(ROWS * sizeof(int*));
	for (int i = 0; i < ROWS; ++i) {
		grid[i] = calloc(COLUMNS, sizeof(int));
	}

	InitWindow(screenWidth, screenHeight, "raylib [core] example - keyboard input");

	player player_1;
	player_1.x = 0;
	player_1.y = 0;
	player_1.head_direction = Still;
	player_1.next_direction = Still;
	player_1.color_main = MAROON;
	player_1.color_second = PINK;
	player_1.id = 1;

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		arrow_input(&player_1.next_direction);
		
		// only update direction and tile ownership when head is aligned with grid
		if ( player_1.x % SQUARE_SIDE == 0 && player_1.y % SQUARE_SIDE == 0 ) {
			player_1.head_direction = player_1.next_direction;
			grid[player_1.y / SQUARE_SIDE][player_1.x / SQUARE_SIDE] = -player_1.id;
		}
		
		move_player(&player_1, head_step);
		player players[] = {player_1};

		draw(grid, (player[]) {player_1}, 1);
	}
	CloseWindow(); // Close window and OpenGL context

	for (int i = 0; i < ROWS; ++i) {
		free(grid[i]);
	}
	free(grid);
	return 0;
}