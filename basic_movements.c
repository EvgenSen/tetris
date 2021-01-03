/*
 * basic_movements.c - example of moving across the playfield
 *
 * This example shows how to read keystrokes without pressing
 * enter and move point across the playfield.
 *
 * Author: Evgeniy Sennikov <sennikov.work@ya.ru>
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <unistd.h>
#include <termios.h>

// #define USE_NCURSES 1

#define FIELD_X_SIZE 10
#define FIELD_Y_SIZE 20

#define clear_screen() printf("\033[H\033[J")

#ifdef USE_NCURSES
# define PRINT printw
#else
# define PRINT printf
#endif

int cur_pos_x = 0;
int cur_pos_y = 0;

void print_screen(void)
{
	int x = 0;
	int y = 0;
#ifdef USE_NCURSES
	clear();
	PRINT("USE_NCURSES = y\n");
#else
	clear_screen();
	PRINT("USE_NCURSES = n\n");
#endif

	PRINT("+----------+\n");
	for (y = 0; y < FIELD_Y_SIZE; y++)
	{
		PRINT("|");
		for (x = 0; x < FIELD_X_SIZE; x++)
		{
			if(cur_pos_x == x && cur_pos_y == y)
				PRINT("#");
			else
				PRINT(" ");
		}
		PRINT("|\n");
	}
	PRINT("+----------+\n");
	PRINT("Press 'q' to exit\n");
}


int main(int argc, const char *argv[])
{
	char c = 0;
#ifdef USE_NCURSES
	/* puts terminal into curses mode and
	 * change terminal mode to accept a char at a time
	 */
	initscr();
	cbreak();
#else
	/* save current terminal setting and disable canonical mode */
	struct termios settings;
	struct termios settings_backup;
	tcgetattr(0, &settings_backup);
	settings = settings_backup;
	settings.c_lflag &= ~ICANON;
	tcsetattr(0, TCSANOW, &settings);
#endif

	print_screen();

	do {
#ifdef USE_NCURSES
		c = getch();
#else
		c = getchar();
#endif
		c = toupper((unsigned char)c);
		if(c == 'S')
		{
			if(cur_pos_y < (FIELD_Y_SIZE - 1))
				cur_pos_y++;
			else
				cur_pos_y = 0;
		}
		else if(c == 'W')
		{
			if(cur_pos_y > 0)
				cur_pos_y--;
			else
				cur_pos_y = FIELD_Y_SIZE - 1;
		}
		else if(c == 'D')
		{
			if(cur_pos_x < (FIELD_X_SIZE - 1))
				cur_pos_x++;
			else
				cur_pos_x = 0;
		}
		else if(c == 'A')
		{
			if(cur_pos_x > 0)
				cur_pos_x--;
			else
				cur_pos_x = FIELD_X_SIZE - 1;
		}
		print_screen();
	} while (c != 'Q');

	/* restore terminal settings */
#ifdef USE_NCURSES
	endwin();
#else
	tcsetattr(0, TCSANOW, &settings_backup);
#endif

	return 0;
}
