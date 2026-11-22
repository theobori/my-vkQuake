/*
 * statistics.h -- producing statistics for each singleplayer map played
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#include <time.h>

#define STATISTICS_JSON_FILENAME_PREFIX "statistics_"
#define STATISTICS_VER					"1.0"

typedef struct
{
	unsigned int jump;
	unsigned int forward;
	unsigned int moveleft;
	unsigned int moveright;
	unsigned int back;
	unsigned int attack;
} statistics_inputs_counters_t;

typedef struct
{
	int							 killed_monsters;
	int							 total_monsters;
	int							 found_secrets;
	int							 total_secrets;
	char						 mapname[128];
	char						 levelname[128];
	const char					*gamename;
	float						 map_time;
	int							 current_skill;
	char						 demoname[256];
	char						 engine_name_and_ver[32];
	float						 timestamp;
	char						 statistics_ver[4];
	qboolean					 is_completed;
	unsigned int				 deaths;
	statistics_inputs_counters_t inputs_counters;
} statistics_t;

typedef struct
{
	char						 demoname[256];
	statistics_inputs_counters_t inputs_counters;
	qboolean					 is_completed;
	qboolean					 is_recording_demo;
	// used to avoid duplicates when incrementing the deaths counter
	qboolean					 is_dead;
	unsigned int				 deaths;

} statistics_current_ctx_t;

void					  CL_InitStatistics (void);
statistics_current_ctx_t *Statistics_CurrentCtx_GetPtr (void);
void					  Statistics_AutoTryStart (void);
void					  Statistics_AutoTryStop (void);

#endif
