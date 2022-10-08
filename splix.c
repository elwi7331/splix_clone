#include <stdlib.h>
#include <assert.h>
#include "raylib.h"

#include <stdio.h>

#define ROWS 30
#define COLUMNS 60
#define SQUARE_SIDE 20
#define STEPS_PER_SQUARE 10

enum direction{Still, Up, Down, Left, Right};
typedef enum direction direction;

struct player {
	int x;
	int y;
	direction head_direction;
	direction next_direction;
	int id; // id field might be replaced by index in "players" list
	Color color_main;
	Color color_second;
	Color color_head;
};
typedef struct player player;

void print_clr(Color clr) {
	printf("r: %d, g: %d, b: %d, a: %d\n", clr.r, clr.g, clr.b, clr.a);
}

void arrow_input (direction *dir) {
	if ( IsKeyDown(KEY_RIGHT) && *dir != Left ) *dir = Right;
	if ( IsKeyDown(KEY_LEFT) && *dir != Right ) *dir = Left;
	if ( IsKeyDown(KEY_UP) && *dir != Down ) *dir = Up;
	if ( IsKeyDown(KEY_DOWN) && *dir != Up ) *dir = Down;
}

void wasd_input (direction *dir) {
	if ( IsKeyDown(KEY_D) && *dir != Left ) *dir = Right;
	if ( IsKeyDown(KEY_A) && *dir != Right ) *dir = Left;
	if ( IsKeyDown(KEY_W) && *dir != Down ) *dir = Up;
	if ( IsKeyDown(KEY_S) && *dir != Up ) *dir = Down;
}

// return 1 if soon outside border
int check_map_border(player p) {
	direction dir = p.head_direction;
	int width = COLUMNS * SQUARE_SIDE;
	int height = ROWS * SQUARE_SIDE;

	if ( p.x == 0 && dir == Left ) {
		printf("1\n");
		return 1;
	} else if ( p.x + SQUARE_SIDE == width && dir == Right ) {
		printf("2\n");
		return 1;
	} else if ( p.y == 0 && dir == Up ) {
		printf("3\n");
		return 1;
	} else if ( p.y + SQUARE_SIDE == height && dir == Down ) {
		printf("4\n");
		return 1;
	}
	return 0;
}

// move player by step in pointing direction
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
				players[i].color_head
			);
		}
	EndDrawing();
}

// 2d array, WIDTH * HEIGHT
int** make_grid() {
	int** grid = malloc(ROWS * sizeof(int*));
	for (int i = 0; i < ROWS; ++i) {
		grid[i] = calloc(COLUMNS, sizeof(int));
	}
	return grid;
}

void free_grid(int** grid) {
	for (int i = 0; i < ROWS; ++i) {
		free(grid[i]);
	}
	free(grid);
}

void flood_fill(int** grid, int row, int col, int new, int empty) {
	if ( grid[row][col] != empty ) {
		return;
	} else {
		grid[row][col] = new;
		if ( row != 0 ) {
			flood_fill(grid, row-1, col, new, empty);
		} if (row != ROWS-1) {
			flood_fill(grid, row+1, col, new, empty);
		} if ( col != 0 ) {
			flood_fill(grid, row, col-1, new, empty);
		} if (row != ROWS-1) {
			flood_fill(grid, row, col+1, new, empty);
		}
	}
}

void area_capture(int** grid, player p) {

	// replace trail with owned blocks
	for (int row = 0; row < ROWS; ++row) {
		for (int col = 0; col < COLUMNS; ++col) {
			if ( grid[row][col] == -p.id ) {
				grid[row][col] = p.id;
			}
		}
	}

	// grid where owned = 0, otherwise 1
	int** fill_grid = make_grid();
	for (int row = 0; row < ROWS; ++row) {
		for (int col = 0; col < COLUMNS; ++col) {
			if ( grid[row][col] == p.id ) {
				fill_grid[row][col] = 0;
			} else {
				fill_grid[row][col] = 1;
			}
		}
	}
	
	// flood fill the 1's with 2
	// where to start? smaller box fill_grid?
	flood_fill(fill_grid, 20, 20, 2, 1);

	// replace remaining 1's with p.id
	for (int row = 0; row < ROWS; ++row) {
		for (int col = 0; col < COLUMNS; ++col) {
			if ( fill_grid[row][col] == 1 ) {
				grid[row][col] = p.id;
			}
		}
	}

	free_grid(fill_grid);
}

int main(void)
{
	// dimensions
	int head_step = SQUARE_SIDE / STEPS_PER_SQUARE;
	assert(SQUARE_SIDE % head_step == 0);

	const int screenWidth = COLUMNS * SQUARE_SIDE;
	const int screenHeight = ROWS * SQUARE_SIDE;

	int** grid = make_grid();

	grid[0][0] = 1;
	grid[1][0] = 1;
	grid[2][0] = 1;
	grid[0][1] = 1;
	grid[1][1] = 1;
	grid[2][1] = 1;
	grid[0][2] = 1;
	grid[1][2] = 1;
	grid[2][2] = 1;

	InitWindow(screenWidth, screenHeight, "Splix");

	player player_1;
	player_1.x = 1 * SQUARE_SIDE;
	player_1.y = 1 * SQUARE_SIDE;
	player_1.head_direction = Still;
	player_1.next_direction = Still;
	player_1.color_main = MAROON;
	player_1.color_second = PINK;
	player_1.color_head = MAGENTA;
	player_1.id = 1;

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		arrow_input(&player_1.next_direction);
		
		// only update direction and tile ownership when head is aligned with grid
		if ( player_1.x % SQUARE_SIDE == 0 && player_1.y % SQUARE_SIDE == 0 ) {
			player_1.head_direction = player_1.next_direction;

			// check if player is outside or inside
			if ( grid[player_1.y / SQUARE_SIDE][player_1.x / SQUARE_SIDE] != player_1.id ) {
				grid[player_1.y / SQUARE_SIDE][player_1.x / SQUARE_SIDE] = -player_1.id;
			} else {
				area_capture(grid, player_1);
			}
		}
		
		if (check_map_border(player_1)) {
			assert((0));
		}

		move_player(&player_1, head_step);
		player players[] = {player_1};

		draw(grid, (player[]) {player_1}, 1);
	}

	CloseWindow(); // Close window and OpenGL context
	free_grid(grid);
	return 0;
}