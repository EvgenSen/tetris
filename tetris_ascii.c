/*
 * tetris_ascii.c -
 *
 * Author: Evgeniy Sennikov <sennikov.work@ya.ru>
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <time.h>

#include "tetris_ascii.h"

#define clear_screen() printf("\033[H\033[J")

#define THREAD_COUNT 2
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_FIGURE_CNT  7
#define MAX_FIGURE_SIZE 4

enum ret_codes {
	RET_OK        = 0,
	RET_ERR       = 1,
	RET_GAME_OVER = 2,
	RET_CANT_MOVE = 3,
};

/** playfield uses this markup:
 *  0 - empty squares
 *  1 - active squares (current figure)
 *  2 - fixed squares (old figures)
 *
 * Y ^
 * 3 |0111100000|
 * 2 |0000000000|
 * 1 |0200022000|
 * 0 |2220220000|
 *   +----------+->
 *    0123456789  X
 */
unsigned char playfield[FIELD_X_SIZE][FIELD_Y_SIZE];

enum playfield_markup {
	SQUARE_EMPTY   = 0,
	SQUARE_ACTIVE  = 1,
	SQUARE_FIXED   = 2,
};

char key = 0;

int cur_fig_rotate =  0;
int cur_fig_num    = -1;
int cur_pos_x[MAX_FIGURE_SIZE] = {0};
int cur_pos_y[MAX_FIGURE_SIZE] = {0};

typedef struct {
	/* Count of figure positions while rotation, starting from 0 */
	int max_rotate_cnt;
	/* Description offsets relative to the main coordinat
	 * [Max Rotate Position = 4][Max Figure Size = 4][Coordinat count (x y) = 2] */
	int offset[4][MAX_FIGURE_SIZE][2];
} figure_t;

#define NULL_OFFSET  {0, 0}, {0, 0}, {0, 0}, {0, 0}

figure_t figure_list[MAX_FIGURE_CNT] =
{
	// %%
	// %%
	{
		0, // = max_rotate_cnt
		{
			{ {0, 0}, {-1, 0}, {0, -1}, {-1, -1} },
			{ NULL_OFFSET },
			{ NULL_OFFSET },
			{ NULL_OFFSET },
		}
	},

	// %%%
	//  %
	{
		3, // = max_rotate_cnt
		{
			{ {0, 0}, {-1, 0}, {1, 0}, {0, -1} },
			{ {0, 0}, {-1, 0}, {0, 1}, {0, -1} },
			{ {0, 0}, {-1, 0}, {1, 0}, {0,  1} },
			{ {0, 0}, { 0, 1}, {1, 0}, {0, -1} },
		}
	},

	// %%
	//  %%
	{
		1, // = max_rotate_cnt
		{
			{ {0, 0}, {-1, 0}, {0, -1}, { 1, -1} },
			{ {0, 0}, {-1, 0}, {0,  1}, {-1, -1} },
			{ NULL_OFFSET },
			{ NULL_OFFSET },
		}
	},

	//  %%
	// %%
	{
		1, // = max_rotate_cnt
		{
			{ {0, 0}, {1, 0}, {0, -1}, {-1, -1} },
			{ {0, 0}, {1, 0}, {0,  1}, { 1, -1} },
			{ NULL_OFFSET },
			{ NULL_OFFSET },
		}
	},

	// %%%
	// %
	{
		3, // = max_rotate_cnt
		{
			{ {0, 0}, {-1, 0}, { 1,  0}, {-1, -1} },
			{ {0, 0}, { 0, 1}, {-1,  1}, { 0, -1} },
			{ {0, 0}, {-1, 0}, { 1,  0}, { 1,  1} },
			{ {0, 0}, { 0, 1}, { 1, -1}, { 0, -1} },
		}
	},

	// %%%
	//   %
	{
		3, // = max_rotate_cnt
		{
			{ {0, 0}, {-1, 0}, { 1,  0}, { 1, -1} },
			{ {0, 0}, { 0, 1}, {-1, -1}, { 0, -1} },
			{ {0, 0}, {-1, 0}, { 1,  0}, {-1,  1} },
			{ {0, 0}, { 0, 1}, { 1,  1}, { 0, -1} },
		}
	},

	// %%%%
	{
		1, // = max_rotate_cnt
		{
			{ {0, 0}, {-2, 0}, {-1, 0}, {1,  0} },
			{ {0, 0}, { 0, 2}, { 0, 1}, {0, -1} },
			{ NULL_OFFSET },
			{ NULL_OFFSET },
		}
	},

};

/** Create new figure.
 *
 *  Figure is randomly selected, set initial coordinates
 *  and checking the possibility of adding figure
 *
 * @param      no args
 *
 * @return     RET_OK        - Create Ok
 *             RET_GAME_OVER - Can't create, game over
 */
int create_new_figure ( void )
{
	int i = 0;
	int ret = RET_OK;

	cur_fig_rotate = 0;
	cur_fig_num = rand() % MAX_FIGURE_CNT;

	cur_pos_x[0] = FIELD_X_SIZE / 2;
	cur_pos_y[0] = FIELD_Y_SIZE - 1;

	for (i = 1; i < MAX_FIGURE_SIZE; i++)
	{
		cur_pos_x[i] = cur_pos_x[0] + figure_list[cur_fig_num].offset[0][i][0];
		cur_pos_y[i] = cur_pos_y[0] + figure_list[cur_fig_num].offset[0][i][1];
	}

	/* Add current figure to playfield */
	for (i = 0; i < MAX_FIGURE_SIZE; i++)
	{
		if(playfield[cur_pos_x[i]][cur_pos_y[i]])
		{
			ret = RET_GAME_OVER;
		}
		else
		{
			playfield[cur_pos_x[i]][cur_pos_y[i]] = SQUARE_ACTIVE;
		}
	}

	return ret;
}

/** Commit current figure.
 *
 *  Change current position of figure to SQUARE_FIXED
 *
 * @param      no args
 *
 * @return     no returns
 */
void commit_cur_figure ( void )
{
	int i = 0;
	for (i = 0; i < MAX_FIGURE_SIZE; i++)
	{
		playfield[cur_pos_x[i]][cur_pos_y[i]] = SQUARE_FIXED;
	}
}

/** Remove current figure.
 *
 *  Change current position of figure to SQUARE_EMPTY
 *
 * @param      no args
 *
 * @return     no returns
 */
void remove_cur_figure ( void )
{
	int i = 0;
	for (i = 0; i < MAX_FIGURE_SIZE; i++)
	{
		playfield[cur_pos_x[i]][cur_pos_y[i]] = SQUARE_EMPTY;
	}
}

/** Remove line and shift all higher squares
 *
 * @param      line - line number for removing
 *
 * @return     no returns
 */
void remove_line ( int line )
{
	int x = 0;
	int y = 0;

	for (y = line; y < (FIELD_Y_SIZE - 1); y++)
	{
		for (x = 0; x < FIELD_X_SIZE; x++)
		{
			playfield[x][y] = playfield[x][y+1];
		}
	}
}

/** Checks each line for fullness. If the line is full
 *  remove it and shift all higher squares
 *
 * @param      no args
 *
 * @return     no returns
 */
void check_full_line ( void )
{
	int x = 0;
	int y = 0;
	int sum = 0;

	for (y = 0; y < FIELD_Y_SIZE; y++)
	{
		sum = 0;
		for (x = 0; x < FIELD_X_SIZE; x++)
		{
			if(playfield[x][y])
				sum++;
		}
		if(sum >= FIELD_X_SIZE)
		{
			remove_line(y);
			/* After remove and shift line
			 * need recheck current line */
			y--;
		}
	}
}

void print_screen ( int game_over )
{
	int x = 0;
	int y = 0;

	clear_screen();

#ifdef DEBUG
	printf("DBG: rotate=%d\n", cur_fig_rotate);
	printf("DBG: cur=%d\n", cur_fig_num);
	printf("DBG: game_over=%d\n", game_over);
	printf("DBG: x=%d y=%d\n", cur_pos_x[0], cur_pos_y[0]);
#endif

	printf("+");
	for (x = 0; x < FIELD_X_SIZE; x++)
	{
		printf("-%c", WIDE_FILED ? '-' : 0);
	}
	printf("+\n");

	for (y = FIELD_Y_SIZE - 1; y >= 0; y--)
	{
		printf("|");
		for (x = 0; x < FIELD_X_SIZE; x++)
		{
			if(playfield[x][y] == SQUARE_EMPTY)
				printf("%c ", WIDE_FILED ? ' ' : 0);
			else
				printf("%c%c", WIDE_FILED ? GENERAL_SYMBOL : 0, GENERAL_SYMBOL);
		}
		printf("|\n");
	}

	printf("+");
	for (x = 0; x < FIELD_X_SIZE; x++)
	{
		printf("-%c", WIDE_FILED ? '-' : 0);
	}
	printf("+\n");

	if(game_over)
	{
		printf(" GAME OVER\n");
	}

	printf("Press '%c' to quit\n", KEY_ACTION_QUIT);
}

/** Moves figure one square to the left
 *
 * @param      no args
 *
 * @return     RET_OK        - Moved Ok
 *             RET_CANT_MOVE - Can't move
 */
int key_action_left ( void )
{
	int i = 0;

	/* Checking for collisions with wall or other figures */
	for (i = 0; i < MAX_FIGURE_SIZE; i++)
	{
		if((cur_pos_x[i] - 1) < 0 ||
		   playfield[cur_pos_x[i] - 1][cur_pos_y[i]] == SQUARE_FIXED)
		{
			return RET_CANT_MOVE;
		}
	}
	remove_cur_figure();
	/* Add current figure one square to the left */
	for (i = 0; i < MAX_FIGURE_SIZE; i++)
	{
		cur_pos_x[i]--;
		playfield[cur_pos_x[i]][cur_pos_y[i]] = SQUARE_ACTIVE;
	}

	return RET_OK;
}

/** Moves figure one square to the right
 *
 * @param      no args
 *
 * @return     RET_OK        - Moved Ok
 *             RET_CANT_MOVE - Can't move
 */
int key_action_right ( void )
{
	int i = 0;

	/* Checking for collisions with wall or other figures */
	for (i = 0; i < MAX_FIGURE_SIZE; i++)
	{
		if((cur_pos_x[i] + 1) > (FIELD_X_SIZE - 1) ||
		   playfield[cur_pos_x[i] + 1][cur_pos_y[i]] == SQUARE_FIXED)
		{
			return RET_CANT_MOVE;
		}
	}
	remove_cur_figure();
	/* Add current figure one square to the right */
	for (i = 0; i < MAX_FIGURE_SIZE; i++)
	{
		cur_pos_x[i]++;
		playfield[cur_pos_x[i]][cur_pos_y[i]] = SQUARE_ACTIVE;
	}

	return RET_OK;
}

/** Moves figure one square to the down.
 *  If can't move, commit current figure,
 *  check full lines and create new figure.
 *
 * @param      no args
 *
 * @return     RET_OK        - Moved Ok
 *             RET_CANT_MOVE - Can't move
 *             RET_GAME_OVER - Can't move and game over
 */
int key_action_down ( void )
{
	int i = 0;
	int ret = RET_OK;

	/* Checking for collisions with bottom or other figures */
	for (i = 0; i < MAX_FIGURE_SIZE; i++)
	{
		if((cur_pos_y[i] - 1) < 0 ||
		   playfield[cur_pos_x[i]][cur_pos_y[i] - 1] == SQUARE_FIXED)
		{
			commit_cur_figure();
			check_full_line();
			ret = create_new_figure();
			return (( ret == RET_OK ) ? RET_CANT_MOVE : RET_GAME_OVER);
		}
	}
	remove_cur_figure();
	/* Add current figure one square to the down */
	for (i = 0; i < MAX_FIGURE_SIZE; i++)
	{
		cur_pos_y[i]--;
		playfield[cur_pos_x[i]][cur_pos_y[i]] = SQUARE_ACTIVE;
	}
	return ret;
}

/** Moves figure down to the end.
 *
 * @param      no args
 *
 * @return     RET_CANT_MOVE - Figure down Ok
 *             RET_GAME_OVER - Can't move and game over
 */
int key_action_drop ( void )
{
	int ret;
	while ((ret = key_action_down()) == RET_OK);

	return ret;
}

/** Moves figure one square to the down.
 *  If can't move, commit current figure,
 *  check full lines and create new figure.
 *
 * @param      no args
 *
 * @return     RET_OK        - Rotated Ok
 *             RET_CANT_MOVE - Can't rotate
 */
int key_action_rotate ( void )
{
	int i = 0;
	int new_x, new_y, new_fig_rotate;

	if(figure_list[cur_fig_num].max_rotate_cnt == 0)
	{
		return RET_CANT_MOVE;
	}

	/* Set new rotate number */
	if((cur_fig_rotate + 1) <= figure_list[cur_fig_num].max_rotate_cnt)
	{
		new_fig_rotate = cur_fig_rotate + 1;
	}
	else
	{
		new_fig_rotate = 0;
	}

	/* Checking for collisions with bottom, walls or other figures */
	for (i = 1; i < MAX_FIGURE_SIZE; i++)
	{
		new_x = cur_pos_x[0] + figure_list[cur_fig_num].offset[new_fig_rotate][i][0];
		new_y = cur_pos_y[0] + figure_list[cur_fig_num].offset[new_fig_rotate][i][1];

		if(new_x < 0 || new_x >= FIELD_X_SIZE ||
		   new_y < 0 || new_y >= FIELD_Y_SIZE ||
		   playfield[new_x][new_y] == SQUARE_FIXED )
		{
			return RET_CANT_MOVE;
		}
	}

	/* Update figure position */
	remove_cur_figure();
	/* Main coordinate does not change */
	playfield[cur_pos_x[0]][cur_pos_y[0]] = SQUARE_ACTIVE;
	for (i = 1; i < MAX_FIGURE_SIZE; i++)
	{
		cur_pos_x[i] = cur_pos_x[0] + figure_list[cur_fig_num].offset[new_fig_rotate][i][0];
		cur_pos_y[i] = cur_pos_y[0] + figure_list[cur_fig_num].offset[new_fig_rotate][i][1];
		playfield[cur_pos_x[i]][cur_pos_y[i]] = SQUARE_ACTIVE;
	}
	cur_fig_rotate = new_fig_rotate;

	return RET_OK;
}

/** Choose key action based on global var 'key'
 *
 * @param      no args
 *
 * @return     RET_OK        - All Ok
 *             RET_GAME_OVER - Game over
 */
int choose_key_action ( void )
{
	int game_over = RET_OK;

	if(key == KEY_ACTION_LEFT)
	{
		key_action_left();
	}
	else if(key == KEY_ACTION_RIGHT)
	{
		key_action_right();
	}
	else if(key == KEY_ACTION_DOWN)
	{
		if(key_action_down() == RET_GAME_OVER)
			game_over = RET_GAME_OVER;
	}
	else if(key == KEY_ACTION_DROP)
	{
		if(key_action_drop() == RET_GAME_OVER)
			game_over = RET_GAME_OVER;
	}
	else if(key == KEY_ACTION_ROTATE)
	{
		key_action_rotate();
	}

	print_screen(game_over);

#ifdef DEBUG
	printf("DBG: get '%c' (%d)\n", key, key);
#endif

	return game_over;
}

/** The second thread that get keystrokes.
 *
 *  Thread get 1 char from terminal and write this to
 *  global variable. This variable read by main thread.
 *
 * @param      no args
 *
 * @return     no return
 */
void key_loop ( void * arg )
{
	char c = 0;
	do {
		c = getchar();
		c = toupper((unsigned char)c);
		pthread_mutex_lock(&mutex);
		key = c;
		pthread_mutex_unlock(&mutex);
	} while (c != KEY_ACTION_QUIT);
}

/** The main thread that draws the screen.
 *
 *  Todo: Descr
 *
 * @param      no args
 *
 * @return     no return
 */
void main_loop ( void * arg )
{
	struct timespec ts_first, ts_cur;
	unsigned long spent_ms = 0;
	int delay = 1000;
	int ret;

	clock_gettime (CLOCK_ID, &ts_first);
	do {
		pthread_mutex_lock(&mutex);
		if(key != 0)
		{
			ret = choose_key_action();
			if(key == KEY_ACTION_QUIT ||
			   ret == RET_GAME_OVER)
			{
				pthread_mutex_unlock(&mutex);
				break;
			}
			key = 0;
		}
		pthread_mutex_unlock(&mutex);
		clock_gettime (CLOCK_ID, &ts_cur);
		spent_ms = 1000 * (ts_cur.tv_sec - ts_first.tv_sec) + ((ts_cur.tv_nsec - ts_first.tv_nsec) / 1000000 );
		if (spent_ms > delay)
		{
			if(key_action_down() == RET_GAME_OVER)
			{
				print_screen(1);
				break;
			}
			print_screen(0);
			clock_gettime (CLOCK_ID, &ts_first);
		}
	} while (1);
}

int main ( int argc, const char * argv[] )
{
	pthread_t thread_id[THREAD_COUNT];
	int i = 0;

	/* save current terminal setting and disable canonical mode */
	struct termios settings;
	struct termios settings_backup;
	tcgetattr(0, &settings_backup);
	settings = settings_backup;
	settings.c_lflag &= ~ICANON;
	tcsetattr(0, TCSANOW, &settings);

	create_new_figure();
	print_screen(0);

	srand(time(NULL));

	pthread_mutex_init(&mutex, NULL);

	pthread_create( &thread_id[0], NULL, (void *)&main_loop, NULL );
	pthread_create( &thread_id[1], NULL, (void *)&key_loop, NULL );

	for( i=0; i < THREAD_COUNT; i++ )
	{
		pthread_join( thread_id[i], NULL);
	}

	pthread_mutex_destroy(&mutex);

	/* restore terminal settings */
	tcsetattr(0, TCSANOW, &settings_backup);

	return 0;
}
