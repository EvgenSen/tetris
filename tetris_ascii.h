/*
 * tetris_ascii.h -
 *
 * Author: Evgeniy Sennikov <sennikov.work@ya.ru>
 */

#ifndef TETRIS_ASCII_H
#define TETRIS_ASCII_H

#define CLOCK_ID CLOCK_MONOTONIC // May be: CLOCK_REALTIME, CLOCK_PROCESS_CPUTIME_ID, CLOCK_MONOTONIC or get id by clock_getcpuclockid()

#define FIELD_X_SIZE 10
#define FIELD_Y_SIZE 20
#define WIDE_FILED   1

#define GENERAL_SYMBOL    '%'

#define KEY_ACTION_QUIT   'Q'
#define KEY_ACTION_LEFT   'A'
#define KEY_ACTION_RIGHT  'D'
// #define KEY_ACTION_UP  'W'
#define KEY_ACTION_DOWN   'S'
#define KEY_ACTION_DROP   ' '
#define KEY_ACTION_ROTATE 'W'

#define DEBUG 1

#endif /* TETRIS_ASCII_H */
