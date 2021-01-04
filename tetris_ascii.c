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

unsigned char playfield[FIELD_X_SIZE][FIELD_Y_SIZE];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int cur_pos_x = FIELD_X_SIZE / 2;
int cur_pos_y = FIELD_Y_SIZE - 1;
char key = 0;

int tmp = 0;

void print_screen(void)
{
	int x = 0;
	int y = 0;

	clear_screen();

#ifdef DEBUG
	printf("x=%d y=%d tmp=%d\n", cur_pos_x, cur_pos_y, ++tmp);
#endif

	printf("+");
	for (x = 0; x < FIELD_X_SIZE; x++)
	{
		printf("-");
	}
	printf("+\n");

	for (y = FIELD_Y_SIZE - 1; y >= 0; y--)
	{
		printf("|");
		for (x = 0; x < FIELD_X_SIZE; x++)
		{
			if(cur_pos_x == x && cur_pos_y == y)
				printf("#");
			else
				printf(" ");
		}
		printf("|\n");
	}

	printf("+");
	for (x = 0; x < FIELD_X_SIZE; x++)
	{
		printf("-");
	}
	printf("+\n");

	printf("Press '%c' to quit\n", KEY_ACTION_QUIT);
}

void key_action_left ( void )
{
	if(cur_pos_x > 0)
		cur_pos_x--;
	else
		cur_pos_x = FIELD_X_SIZE - 1;
}

void key_action_right ( void )
{
	if(cur_pos_x < (FIELD_X_SIZE - 1))
		cur_pos_x++;
	else
		cur_pos_x = 0;
}

// void key_action_up ( void )
// {
// 	if(cur_pos_y < (FIELD_Y_SIZE - 1))
// 		cur_pos_y++;
// 	else
// 		cur_pos_y = 0;
// }

void key_action_down ( void )
{
	if(cur_pos_y > 0)
		cur_pos_y--;
	else
		cur_pos_y = FIELD_Y_SIZE - 1;
}

void key_action_drop ( void )
{
	cur_pos_y = 0;
}

void key_action_rotate ( void )
{

}

/** Choose key action based on global var 'key'
 *
 * @param      no args
 *
 * @return     no return
 */
void choose_key_action ( void )
{
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
		key_action_down();
	}
	else if(key == KEY_ACTION_DROP)
	{
		key_action_drop();
	}
	else if(key == KEY_ACTION_ROTATE)
	{
		key_action_rotate();
	}

	print_screen();

#ifdef DEBUG
	printf("get '%c' (%d)\n", key, key);
#endif

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

	clock_gettime (CLOCK_ID, &ts_first);
	do {
		pthread_mutex_lock(&mutex);
		if(key != 0)
		{
			if(key == KEY_ACTION_QUIT)
			{
				pthread_mutex_unlock(&mutex);
				break;
			}
			choose_key_action();
			key = 0;
		}
		pthread_mutex_unlock(&mutex);
		clock_gettime (CLOCK_ID, &ts_cur);
		spent_ms = 1000 * (ts_cur.tv_sec - ts_first.tv_sec) + ((ts_cur.tv_nsec - ts_first.tv_nsec) / 1000000 );
		if (spent_ms > delay)
		{
			key_action_down();
			print_screen();
			clock_gettime (CLOCK_ID, &ts_first);
		}
	} while (1);
}

int main ( int argc, const char *argv[] )
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

	print_screen();

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
