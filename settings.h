/*
 * settings.h - Define all configurable parameters
 *
 * Author: Evgeniy Sennikov <sennikov.work@ya.ru>
 */

#ifndef SETTINGS_H
#define SETTINGS_H

/*
 * System settings
 */
#define CLOCK_ID CLOCK_MONOTONIC // May be: CLOCK_REALTIME, CLOCK_PROCESS_CPUTIME_ID, CLOCK_MONOTONIC or get id by clock_getcpuclockid()
// #define DEBUG 1

/*
 * Field settings
 */
#define FIELD_X_SIZE 10
#define FIELD_Y_SIZE 20
#define WIDE_FILED   1
#define GENERAL_SYMBOL   '%'
#define GAME_OVER_SYMBOL '#'

/*
 * Level and score settings
 */
#define LEVEL_START  0    // Game starts from this level
#define LEVEL_STEP   5    // Remove $(LEVEL_STEP) lines to level up
#define DELAY_START  800  // Game starts from this delay (ms)
#define DELAY_STEP   40   // Each level decreases delay by this value (ms)
// Points for simultaneous removes of lines:
#define RM_1_LINE_SCORE 100
#define RM_2_LINE_SCORE 300 
#define RM_3_LINE_SCORE 700 
#define RM_4_LINE_SCORE 1500

/*
 * Control settings
 */
#define KEY_ACTION_QUIT   'Q'
#define KEY_ACTION_LEFT   'A'
#define KEY_ACTION_RIGHT  'D'
// #define KEY_ACTION_UP  'W'
#define KEY_ACTION_DOWN   'S'
#define KEY_ACTION_DROP   ' '
#define KEY_ACTION_ROTATE 'W'

#endif /* SETTINGS_H */
