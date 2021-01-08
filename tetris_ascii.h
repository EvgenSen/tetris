/*
 * tetris_ascii.h - Description of structures, defines
 *
 * Author: Evgeniy Sennikov <sennikov.work@ya.ru>
 */

#ifndef TETRIS_ASCII_H
#define TETRIS_ASCII_H

#include "settings.h"

#define clear_screen() printf("\033[H\033[J")

#define THREAD_COUNT 2

#define X_INDEX 0
#define Y_INDEX 1

#define MAX_FIGURE_CNT  7
#define MAX_FIGURE_SIZE 4

#define HEADER_X_SIZE   FIELD_X_SIZE
#define HEADER_Y_SIZE   2

#define NULL_OFFSET  {0, 0}, {0, 0}, {0, 0}, {0, 0}

/* return playfield value for i coordinates */
#define CUR_FIELD_VAL(i) playfield[gen.cur_pos_x[i]][gen.cur_pos_y[i]]
/* return current figure from figure_list */
#define CUR_FIGURE()     figure_list[gen.cur_fig_num]
/* return next figure from figure_list */
#define NEXT_FIGURE()    figure_list[gen.next_fig_num]

enum ret_codes {
	RET_OK        = 0,
	RET_ERR       = 1,
	RET_GAME_OVER = 2,
	RET_CANT_MOVE = 3,
};

enum playfield_markup {
	SQUARE_EMPTY   = 0,
	SQUARE_ACTIVE  = 1,
	SQUARE_FIXED   = 2,
};

typedef struct {
	/* Count of figure positions while rotation, starting from 0 */
	int max_rotate_cnt;
	/* Description offsets relative to the main coordinat
	 * [Max Rotate Position = 4][Max Figure Size = 4][Coordinat count (x y) = 2] */
	int offset[4][MAX_FIGURE_SIZE][2];
} figure_t;

typedef struct {
	int delay;
	int score;
	int level;
	int line;
	int tetris;
} stats_t;

typedef struct {
	int cur_fig_rotate;
	int cur_fig_num;
	int next_fig_num;
	int cur_pos_x[MAX_FIGURE_SIZE];
	int cur_pos_y[MAX_FIGURE_SIZE];
} general_t;

#endif /* TETRIS_ASCII_H */
