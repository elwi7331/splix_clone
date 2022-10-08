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
	int id;
	Color color_main;
	Color color_second;
	Color color_head;
	
	int x_upper;
	int x_lower;
	int y_upper;
	int y_lower;
};
typedef struct player player;

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
// return 2 if soon in tail (own)
// return 3 if sonn in tail (other)
int check_next_move(int **grid, const player p) {
	direction dir = p.head_direction;
	const int width = COLUMNS * SQUARE_SIDE;
	const int height = ROWS * SQUARE_SIDE;

	// index of tile that player/head is moving towards
	int tile_x = p.x / SQUARE_SIDE;
	int tile_y = p.y / SQUARE_SIDE;
	
	switch (dir) {
		case Left:
			if ( p.x == 0 ) {
				return 1;
			} else {
				--tile_x;
			} 
			break;
		case Right:
			if ( p.x + SQUARE_SIDE == width ) {
				return 1;
			} else {
				++tile_x;
			}
			break;
		case Up:
			if ( p.y == 0 ) {
				return 1;
			} else {
				--tile_y;
			}
			break;
		case Down:
			if ( p.y + SQUARE_SIDE == height ) {
				return 1;
			} else {
				++tile_y;
			}
		case Still:
			return 0;
	}

	// check if player is moving towards own or other's tail
	if ( grid[tile_y][tile_x] < 0 ) {
		if ( grid[tile_y][tile_x] == -p.id ) {
			return 2;
		} else {
			return 3;
		}
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

void draw(int **grid, const player players[], const int players_len) {
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
int** make_grid(const int rows, const int columns) {
	int** grid = malloc(rows * sizeof(int*));
	for (int i = 0; i < rows; ++i) {
		grid[i] = calloc(columns, sizeof(int));
	}
	return grid;
}

void free_grid(int** grid, const int rows) {
	for (int i = 0; i < rows; ++i) {
		free(grid[i]);
	}
	free(grid);
}

void flood_fill(int** grid, const int row, const int col, const int rows, const int columns, const int new, const int empty) {
	if ( grid[row][col] != empty ) {
		return;
	} else {
		grid[row][col] = new;
		if ( row != 0 ) {
			flood_fill(grid, row-1, col, rows, columns, new, empty);
		} if (row != rows-1) {
			flood_fill(grid, row+1, col, rows, columns, new, empty);
		} if ( col != 0 ) {
			flood_fill(grid, row, col-1, rows, columns, new, empty);
		} if (col != columns-1) {
			flood_fill(grid, row, col+1, rows, columns, new, empty);
		}
	}
} 

void resize_bounds( int** grid, player *p ) {
	int x_upper = -1;
	int x_lower = COLUMNS+1;
	int y_upper = -1;
	int y_lower = ROWS+1;

	for (int row = 0; row < ROWS; ++row) {
		for (int col = 0; col < COLUMNS; ++col) {
			if ( grid[row][col] == (*p).id ) {
				if ( x_upper < col ) {
					x_upper = col;
				} else if ( x_lower > col ) {
					x_lower = col;
				} else if ( y_upper < row ) {
					y_upper = row;
				} else if ( y_lower > row ) {
					y_lower = row;
				}
			}
		}
	}

	(*p).x_upper = x_upper;
	(*p).x_lower = x_lower;
	(*p).y_upper = y_upper;
	(*p).y_lower = y_lower;
}

void area_capture(int** grid, const player p) {

	/*
	fill grid is size of the smallest box that contains all of the players
	tiles + one row/column of "empty" tiles in every direction.
	*/

	int fill_grid_rows = p.y_upper - p.y_lower + 3;
	int fill_grid_columns = p.x_upper - p.x_lower + 3;

	// replace trail with owned blocks
	for (int row = 0; row < ROWS; ++row) {
		for (int col = 0; col < COLUMNS; ++col) {
			if ( grid[row][col] == -p.id ) {
				grid[row][col] = p.id;
			}
		}
	}

	int** fill_grid = make_grid(fill_grid_rows, fill_grid_columns);
	
	/*
	We only need to look at the tiles in the smallest box
	that contains all of the players tiles.
	This box is 2 rows / columns smaller than fill_grid.
	This hopefully explains the test expressions in the for loops.

	As fill_grid has a "marigin" of empty rows/columns,
	the indexes are shifted by one. 
	This is why 1 is added to the grid-indexes when
	accessing fill_grid.
	*/
	for ( int row = 0; row < fill_grid_rows-2; ++row ) {
		for (int col = 0; col < fill_grid_columns-2; ++col) {
			if ( grid[ row + p.y_lower ][ col + p.x_lower ] == p.id ) {
				fill_grid[ row+1 ][ col+1 ] = 1;
			} else {
				fill_grid[ row+1 ][ col+1 ] = 0;
			}
		}
	}
	
	// flood fill the 0's with 2, starting in upper left corner
	flood_fill(fill_grid, 0, 0, fill_grid_rows, fill_grid_columns, 2, 0);

	/*
	replace remaining 0's with p.id
	the indexing is explained by comments in the nested loop above.
	*/

	for (int row = 0; row < fill_grid_rows-2; ++row) {
		for (int col = 0; col < fill_grid_columns-2; ++col) {
			if ( fill_grid[ row+1 ][ col+1 ] == 0 ) {
				grid[ row + p.y_lower ][ col + p.x_lower ] = p.id;
			}
		}
	}

	free_grid(fill_grid, fill_grid_rows);
}

int main(void)
{
	// dimensions
	int head_step = SQUARE_SIDE / STEPS_PER_SQUARE;
	assert(SQUARE_SIDE % head_step == 0);

	const int screenWidth = COLUMNS * SQUARE_SIDE;
	const int screenHeight = ROWS * SQUARE_SIDE;

	int** grid = make_grid(ROWS, COLUMNS);

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

	player_1.x_upper = 2;
	player_1.x_lower = 0;
	player_1.y_upper = 2;
	player_1.y_lower = 0;

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		arrow_input(&player_1.next_direction);
		
		// only update direction and tile ownership when head is aligned with grid
		if ( player_1.x % SQUARE_SIDE == 0 && player_1.y % SQUARE_SIDE == 0 ) {
			int aligned_x = player_1.x / SQUARE_SIDE;
			int aligned_y = player_1.y / SQUARE_SIDE;

			player_1.head_direction = player_1.next_direction;

			// check if player is outside or inside
			if ( grid[aligned_y][aligned_x] != player_1.id ) {
	
				// paint trail (-id)
				grid[aligned_y][aligned_x] = -player_1.id;
				
				// update bounds
				if ( player_1.x_upper < aligned_x ) {
					player_1.x_upper = aligned_x;
				} if ( player_1.x_lower > aligned_x ) {
					player_1.x_upper = aligned_x;
				} if ( player_1.y_upper < aligned_y ) {
					player_1.y_upper = aligned_y;
				} if ( player_1.y_lower > aligned_y ) {
					player_1.x_upper = aligned_x;
				}

			} else {
				area_capture(grid, player_1);
			}

			if (check_next_move(grid, player_1)) {
				goto exit_game;
			}
		}
		

		move_player(&player_1, head_step);
		player players[] = {player_1};

		draw(grid, (player[]) {player_1}, 1);
	}

	exit_game:
		CloseWindow(); // Close window and OpenGL context
		free_grid(grid, ROWS);
		return 0;
}