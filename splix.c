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

enum input_mode{wasd, arrows};
typedef enum input_mode input_mode;

struct player {
	int x;
	int y;
	int prev_x_aligned;
	int prev_y_aligned;
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
	
	input_mode input;
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
// negative other.id if soon in other's tail
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
				return p.id;
			} else {
				--tile_x;
			} 
			break;
		case Right:
			if ( p.x + SQUARE_SIDE == width ) {
				return p.id;
			} else {
				++tile_x;
			}
			break;
		case Up:
			if ( p.y == 0 ) {
				return p.id;
			} else {
				--tile_y;
			}
			break;
		case Down:
			if ( p.y + SQUARE_SIDE == height ) {
				return p.id;
			} else {
				++tile_y;
			}
			break;
		case Still:
			return 0;
	}

	// check if player is moving towards own or other's tail
	if ( grid[tile_y][tile_x] < 0 ) {
		if ( grid[tile_y][tile_x] == -p.id ) {
			return p.id;
		} else {
			return -grid[tile_y][tile_x];
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

void draw(int **grid, player *players[], const int players_len) {
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
					 clr = (*players[-tile-1]).color_second;
				} else if (tile > 0) {
					 clr = (*players[tile-1]).color_main;
				}
				DrawRectangle(col * SQUARE_SIDE, row * SQUARE_SIDE, SQUARE_SIDE, SQUARE_SIDE, clr);
			}
		}
		for (int i = 0; i<players_len; ++i) {
			DrawRectangle(
				(*players[i]).x,
				(*players[i]).y,
				SQUARE_SIDE,
				SQUARE_SIDE,
				(*players[i]).color_head
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

// replace all tiles = a -> b
void replace(int **grid, int a, int b, int rows, int columns) {
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < columns; ++col) {
			if ( grid[row][col] == a ) {
				grid[row][col] = b;
			}
		}
	}
}

void area_capture(int** grid, const player p) {

	/*
	fill grid is size of the smallest box that contains all of the players
	tiles + one row/column of "empty" tiles in every direction.
	*/

	int fill_grid_rows = p.y_upper - p.y_lower + 3;
	int fill_grid_columns = p.x_upper - p.x_lower + 3;

	// fill trail
	replace(grid, -p.id, p.id, ROWS, COLUMNS);

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

void spawn(int **grid, player *p) {
	int x = rand() % (COLUMNS-1) + 2;
	int y = rand() % (ROWS-1)+2;

	grid[y-1][x-1] = (*p).id;
	grid[y][x-1]   = (*p).id;
	grid[y+1][x-1] = (*p).id;
	grid[y-1][x]   = (*p).id;
	grid[y][x]     = (*p).id;
	grid[y+1][x]   = (*p).id;
	grid[y-1][x+1] = (*p).id;
	grid[y][x+1]  = (*p).id;
	grid[y+1][x+1] = (*p).id;
	
	(*p).x = x * SQUARE_SIDE;
	(*p).y = y * SQUARE_SIDE;
	(*p).prev_x_aligned = x;
	(*p).prev_y_aligned = y;

	(*p).head_direction = Still;
	(*p).next_direction = Still;
	
	(*p).x_lower = x-1;
	(*p).x_upper = x+1;
	(*p).y_lower = y-1;
	(*p).y_upper = y+1;
}

int main(void)
{
	// dimensions
	int head_step = SQUARE_SIDE / STEPS_PER_SQUARE;
	assert(SQUARE_SIDE % head_step == 0);
	const int screenWidth = COLUMNS * SQUARE_SIDE;
	const int screenHeight = ROWS * SQUARE_SIDE;

	// the grid on which the players exist
	int** grid = make_grid(ROWS, COLUMNS);

	// boiler stuffz
	InitWindow(screenWidth, screenHeight, "Splix");
	SetTargetFPS(60);

	// player stuffs
	player player_1;
	player_1.color_main = MAROON;
	player_1.color_second = PINK;
	player_1.color_head = MAGENTA;
	player_1.id = 1;
	player_1.input = arrows;
	spawn(grid, &player_1);
	
	player player_2;
	player_2.color_main = DARKGREEN;
	player_2.color_second = GREEN;
	player_2.color_head = LIME;
	player_2.id = 2;
	player_2.input = wasd;
	spawn(grid, &player_2);

	player *players[] = { &player_1, &player_2 };
	int players_len = 2;
	
	int move_status; // used when calling check_next_move

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		
		for ( int i = 0; i < players_len; ++i ) {

			switch ( (*players[i]).input ) {
				case arrows:
					arrow_input(&(*players[i]).next_direction);
					break;
				case wasd:
					wasd_input(&(*players[i]).next_direction);
					break;
			}
			
			// only update direction and tile ownership when head is aligned with grid
			if ( (*players[i]).x % SQUARE_SIDE == 0 && (*players[i]).y % SQUARE_SIDE == 0 ) {
				int aligned_x = (*players[i]).x / SQUARE_SIDE;
				int aligned_y = (*players[i]).y / SQUARE_SIDE;

				(*players[i]).head_direction = (*players[i]).next_direction;

				// check if player is outside or inside
				if ( grid[aligned_y][aligned_x] != (*players[i]).id ) {
	
					// paint trail (-id)
					grid[aligned_y][aligned_x] = -(*players[i]).id;
					
					// update bounds
					if ( (*players[i]).x_upper < aligned_x ) {
						(*players[i]).x_upper = aligned_x;
					} if ( (*players[i]).x_lower > aligned_x ) {
						(*players[i]).x_lower = aligned_x;
					} if ( (*players[i]).y_upper < aligned_y ) {
						(*players[i]).y_upper = aligned_y;
					} if ( (*players[i]).y_lower > aligned_y ) {
						(*players[i]).y_lower = aligned_y;
					}

				} else if ( grid[(*players[i]).prev_y_aligned][(*players[i]).prev_x_aligned] != (*players[i]).id ) {
					area_capture(grid, (*players[i]) );
				}

				move_status = check_next_move(grid, (*players[i]));
					if ( move_status != 0 ) {
						replace(grid, move_status, 0, ROWS, COLUMNS);
						replace(grid, -move_status, 0, ROWS, COLUMNS);
						spawn(grid, players[move_status-1]);
					}
					

				(*players[i]).prev_y_aligned = aligned_y;
				(*players[i]).prev_x_aligned = aligned_x;
			}
			

			move_player(players[i], head_step);
		}
		draw(grid, players, players_len);

	}

	exit_game:
		CloseWindow(); // Close window and OpenGL context
		free_grid(grid, ROWS);
		return 0;
}


// TODO resize when area has been captured