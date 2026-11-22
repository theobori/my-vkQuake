/*
Copyright (C) 2026 Théo Bori

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"
#include "json.h"

static cvar_t					cl_statistics_auto = {"cl_statistics_auto", "0", CVAR_ARCHIVE_GAME}; // enable or disable the statistics production
static statistics_current_ctx_t statistics_current_ctx;
static UUID4_STATE_T			state;
static UUID4_T					uuid;

statistics_current_ctx_t *Statistics_CurrentCtx_GetPtr ()
{
	return &statistics_current_ctx;
}

static qboolean Statistics_Com_Writefile (const char *filename, const void *data, int len)
{
	int handle;

	handle = Sys_FileOpenWrite (filename);
	if (handle == -1)
	{
		Con_DWarning ("(statistics_com_writefile): Can't open '%s'\n", filename);
		return false;
	}

	Sys_FileWrite (handle, data, len);
	Sys_FileClose (handle);

	return true;
}

static qboolean Statistics_Init (statistics_t *statistics)
{
	globalvars_t *globalvars = (globalvars_t *)sv.qcvm.globals;
	if (!globalvars)
	{
		Con_DWarning ("(statistics_create): The globalvars are not initialized\n");
		return false;
	}

	statistics->killed_monsters = (int)globalvars->killed_monsters;
	statistics->total_monsters = (int)globalvars->total_monsters;
	statistics->found_secrets = (int)globalvars->found_secrets;
	statistics->total_secrets = (int)globalvars->total_secrets;
	statistics->map_time = globalvars->time;
	statistics->current_skill = current_skill;
	statistics->timestamp = time (NULL);
	statistics->is_completed = statistics_current_ctx.is_completed;
	statistics->deaths = statistics_current_ctx.deaths;
	statistics->inputs_counters = statistics_current_ctx.inputs_counters;

	const char *gamename = COM_GetGameNames (false);
	if (gamename[0] == 0)
	{
		statistics->gamename = GAMENAME;
	}
	else
	{
		statistics->gamename = gamename;
	}

	q_strlcpy (statistics->levelname, cl.levelname, sizeof (cl.levelname));
	q_strlcpy (statistics->mapname, cl.mapname, sizeof (cl.mapname));
	q_strlcpy (statistics->demoname, statistics_current_ctx.demoname, sizeof (statistics_current_ctx.demoname));
	q_strlcpy (statistics->engine_name_and_ver, ENGINE_NAME_AND_VER, strlen (ENGINE_NAME_AND_VER) + 1);
	q_strlcpy (statistics->statistics_ver, STATISTICS_VER, sizeof (statistics->statistics_ver));

	return true;
}

static qboolean Statistics_IsAutoEnabled (void)
{
	return cl_statistics_auto.value != 0;
}

static qboolean Statistics_IsAutoDisabled (void)
{
	return !Statistics_IsAutoEnabled ();
}

static qboolean Statistics_Demo_IsRecording (void)
{
	return statistics_current_ctx.is_recording_demo;
}

static char *Statistics_Demo_GetFilePath (void)
{
	return CL_GetDemoFilePath ();
}

static qboolean Statistics_SetUniqueName (char *out, int capaciy)
{
	uuid4_gen (&state, &uuid);

	if (!uuid4_to_s (uuid, out, capaciy))
	{
		return false;
	}

	return true;
}

static qboolean Statistics_Demo_SetUniqueName (char *out, int capaciy)
{
	return Statistics_SetUniqueName (out, capaciy);
}

static qboolean Statistics_Demo_StartRecord (char *demoname)
{
	if (cls.demoplayback)
	{
		Con_DWarning ("(statistics_demo_startrecord): Can't record during demo playback\n");
		return false;
	}

	q_strlcpy (statistics_current_ctx.demoname, demoname, strlen (demoname) + 1);

	char command_text[256];
	if (sprintf (command_text, "record %s", demoname) < 0)
	{
		Con_DWarning ("(statistics_demo_startrecord): Can't build demo name\n");
		return false;
	}

	if (!Cmd_ExecuteString (command_text, src_command))
	{

		Con_DWarning ("(statistics_demo_startrecord): Can't start the demo recording\n");
		return false;
	}

	statistics_current_ctx.is_recording_demo = true;

	return true;
}

static qboolean Statistics_Demo_StartRecordWithUniqueName ()
{
	char demoname[128];

	if (!Statistics_Demo_SetUniqueName (demoname, sizeof (demoname)))
	{
		Con_DWarning ("(statistics_demo_startrecordwithuniquename): Can't set a unique demo name\n");
		return false;
	}

	COM_AddExtension (demoname, ".dem", sizeof (demoname));

	if (!Statistics_Demo_StartRecord (demoname))
	{
		return false;
	}

	return true;
}

static qboolean Statistics_Demo_StopRecord (void)
{
	if (!Statistics_Demo_IsRecording ())
	{
		Con_DWarning ("(statistics_demo_stoprecord): There are no statistics demo recording\n");
		return false;
	}
	if (!Cmd_ExecuteString ("stop", src_command))
	{
		Con_DWarning ("(statistics_demo_stoprecord): Can't stop the demo recording\n");
		return false;
	}

	statistics_current_ctx.is_recording_demo = false;

	return true;
}

static qboolean Statistics_GetDirectoryPath (statistics_t *statistics, char *out, int capacity)
{
	size_t i = 0;

	i = q_strlcat (out, com_statisticsdir, capacity);
	if (i && out[i - 1] != '/')
	{
		out[i++] = '/';
		out[i] = 0;
	}

	i = q_strlcat (out, statistics->gamename, capacity);
	if (i && out[i - 1] != '/')
	{
		out[i++] = '/';
		out[i] = 0;
	}

	return true;
}

static void Statistics_Json_GetAsString (statistics_t *statistics, char *out, int capacity)
{
	char *mem_string = NULL;

	mem_string = q_strcatf (
		mem_string,
		"{"
		"\"killed_monsters\":%d,"
		"\"total_monsters\":%d,"
		"\"found_secrets\":%d,"
		"\"total_secrets\":%d,"
		"\"mapname\":\"%s\","
		"\"levelname\":\"%s\","
		"\"gamename\":\"%s\","
		"\"map_time\":%f,"
		"\"current_skill\":%d,"
		"\"demoname\":\"%s\","
		"\"timestamp\":%f,"
		"\"engine_name_and_ver\":\"%s\","
		"\"statistics_ver\":\"%s\","
		"\"is_completed\":%s,"
		"\"deaths\":%u,"
		"\"inputs_counters\":{"
		"\"jump\":%u,"
		"\"forward\":%u,"
		"\"moveleft\":%u,"
		"\"moveright\":%u,"
		"\"back\":%u,"
		"\"attack\":%u"
		"}}",
		statistics->killed_monsters, statistics->total_monsters, statistics->found_secrets, statistics->total_secrets, statistics->mapname,
		statistics->levelname, statistics->gamename, statistics->map_time, statistics->current_skill, statistics->demoname, statistics->timestamp,
		statistics->engine_name_and_ver, statistics->statistics_ver, statistics->is_completed ? "true" : "false", statistics->deaths,
		statistics->inputs_counters.jump, statistics->inputs_counters.forward, statistics->inputs_counters.moveleft, statistics->inputs_counters.moveright,
		statistics->inputs_counters.back, statistics->inputs_counters.attack);

	q_strlcpy (out, mem_string, capacity);
	Mem_Free (mem_string);
}

static qboolean Statistics_IsSingleplayer ()
{
	return !isDedicated && cl.gametype == GAME_COOP && cl.maxclients == 1;
}

static qboolean Statistics_IsValidMap ()
{
	return strlen (cl.levelname) != 0 && strcmp (cl.mapname, "start") != 0;
}

static qboolean Statistics_IsPlayerAlive ()
{
	return cl.stats[STAT_HEALTH] > 0;
}

static qboolean Statistics_IsPlayerDead ()
{
	return !Statistics_IsPlayerAlive ();
}

static qboolean Statistics_Start (void)
{
	if (!Statistics_IsSingleplayer () || !Statistics_IsValidMap ())
	{
		Con_DWarning ("(statistics_start): Join a valid singleplayer map\n");
		return false;
	}

	if (Statistics_IsPlayerDead ())
	{
		Con_DWarning ("(statistics_start): You must be alive\n");
		return false;
	}

	if (Statistics_Demo_IsRecording ())
	{
		Con_DWarning ("(statistics_start): The statistics production is already recording\n");
		return false;
	}

	statistics_current_ctx.is_completed = false;
	memset (&statistics_current_ctx.inputs_counters, 0, sizeof (statistics_inputs_counters_t));
	statistics_current_ctx.deaths = 0;
	statistics_current_ctx.is_dead = false;

	if (!Statistics_Demo_StartRecordWithUniqueName ())
	{
		Con_DWarning ("(statistics_start): Can't record\n");
		return false;
	}

	return true;
}

static qboolean Statistics_Json_Export (statistics_t *statistics, char *path)
{
	char statistics_json_as_string[4096] = {0};
	Statistics_Json_GetAsString (statistics, statistics_json_as_string, sizeof (statistics_json_as_string));

	char *json_content = (char *)COM_LoadMallocFile_TextMode_OSPath (path, NULL);
	if (!json_content || *json_content == 0)
	{
		json_content = Mem_Alloc (3);
		q_strlcpy (json_content, "[]", 3);
	}
	else
	{
		json_t *json = JSON_Parse (json_content);
		if (!json)
		{
			Con_DWarning ("(statistics_json_export): Can't parse the JSON file content\n");
			return false;
		}
		if (json->root->type != JSON_ARRAY)
		{
			Con_DWarning ("(statistics_json_export): The JSON root node must be of type JSON_ARRAY\n");
			return false;
		}
		JSON_Free (json);
	}

	size_t n = strlen (json_content);
	size_t json_content_size = n + sizeof (statistics_json_as_string);
	json_content = Mem_Realloc (json_content, json_content_size + 1);

	char *end = json_content + n;
	while (end > json_content + 1 && *end != '}')
		end--;

	switch (*end)
	{
	case '}':
		end++;
		*end++ = ',';
		break;
	case ']':
		break;
	default:
		Con_DWarning ("(statistics_json_export): Malformed JSON data\n");
		return false;
	}

	q_strlcpy (end, statistics_json_as_string, json_content_size);
	q_strlcat (end, "]", json_content_size);

	if (!Statistics_Com_Writefile (path, json_content, strlen (json_content)))
	{
		Con_DWarning ("(statistics_json_export): Can't write JSON to the file '%s'\n", path);
		return false;
	}

	Con_DPrintf ("The statistics JSON file has been written into '%s'\n", path);

	return true;
}

static qboolean Statistics_Demo_Export (statistics_t *statistics, char *path)
{
	if (Sys_rename (Statistics_Demo_GetFilePath (), path) != 0)
	{
		Con_DWarning ("(statistics_json_export): Can't move the statistics demo file\n");
		return false;
	}

	Con_DPrintf ("The statistics demo file has been moved to '%s'\n", path);

	return true;
}

static qboolean Statistics_Stop (void)
{
	if (!Statistics_Demo_StopRecord ())
	{
		Con_DWarning ("(statistics_stop): Can't stop the demo recording\n");
		return false;
	}

	statistics_t statistics;
	if (!Statistics_Init (&statistics))
	{
		Con_DWarning ("(statistics_stop): Can't initialize the statistics\n");
		return false;
	}

	char path[MAX_OSPATH] = {0};
	if (!Statistics_GetDirectoryPath (&statistics, path, sizeof (path)))
	{
		Con_DWarning ("(statistics_stop): Can't get the statistics directory path\n");
		return false;
	}

	Sys_mkdir (path);
	char *file_offset = path + strlen (path);
	q_strlcpy (file_offset, statistics_current_ctx.demoname, strlen (statistics_current_ctx.demoname) + 1);
	if (!Statistics_Demo_Export (&statistics, path))
	{
		Con_DWarning ("(statistics_stop): Can't export the demo file\n");
		return false;
	}

	file_offset += q_strlcpy (file_offset, STATISTICS_JSON_FILENAME_PREFIX, strlen (STATISTICS_JSON_FILENAME_PREFIX) + 1);
	file_offset += q_strlcpy (file_offset, statistics.mapname, strlen (statistics.mapname) + 1);
	file_offset += q_strlcpy (file_offset, ".json", strlen (".json") + 1);

	if (!Statistics_Json_Export (&statistics, path))
	{
		Con_DWarning ("(statistics_stop): Can't export the JSON file\n");
		return false;
	}

	return true;
}

static qboolean Statistics_Cancel (void)
{
	if (!Statistics_Demo_StopRecord ())
	{
		Con_DWarning ("(statistics_cancel): Can't stop the demo recording\n");
		return false;
	}

	if (Sys_remove (Statistics_Demo_GetFilePath ()) != 0)
	{
		Con_DWarning ("(statistics_cancel): Can't remove the demo file\n");
		return false;
	}

	return true;
}

static void Statistics_Cancel_f (void)
{
	if (cmd_source != src_command)
		return;

	if (!Statistics_Cancel ())
		Con_Printf ("The statistics production could not been cancel properly\n");
	else
		Con_Printf ("The statistics production has been cancel\n");
}

static void CL_Statistics_Auto_f (cvar_t *var)
{
	if (Statistics_IsAutoEnabled ())
	{
		Con_Printf ("The statistics production has been enabled, it will take effect on the next map\n");
	}
	else
	{
		if (Statistics_Demo_IsRecording ())
		{
			if (!Statistics_Cancel ())
			{
				// unable to cancel the demo for some reason
				return;
			}
		}

		Con_Printf ("The statistics production has been disabled\n");
	}
}

static void Statistics_Start_f (void)
{
	if (cmd_source != src_command)
		return;

	if (!Statistics_Start ())
		Con_Printf ("The statistics production could not been started properly\n");
	else
		Con_Printf ("The statistics production has been started\n");
}
static void Statistics_Stop_f (void)
{
	if (cmd_source != src_command)
		return;

	if (!Statistics_Stop ())
		Con_Printf ("The statistics production could not been stopped properly\n");
	else
		Con_Printf ("The statistics production has been stopped\n");
}

void Statistics_AutoTryStop (void)
{
	if (Statistics_IsAutoDisabled () || !Statistics_Demo_IsRecording ())
		return;

	if (!Statistics_Stop ())
		Sys_Printf ("The statistics production could not been automatically stopped properly\n");
	else
		Sys_Printf ("The statistics production has automatically been stopped\n");
}

void Statistics_AutoTryStart (void)
{
	if (Statistics_IsAutoDisabled () || Statistics_Demo_IsRecording ())
		return;

	if (!Statistics_Start ())
		Sys_Printf ("The statistics production could not been automatically started properly\n");
	else
		Sys_Printf ("The statistics production has automatically been started\n");
}

#if defined(STATISTICS_DEBUG)
static void Statistics_Json_Print (statistics_t *statistics)
{
	char statistics_json_as_string[4096] = {0};
	Statistics_Json_GetAsString (statistics, statistics_json_as_string, sizeof (statistics_json_as_string));

	Con_Printf ("%s\n", statistics_json_as_string);
}

static void Statistics_Json_Print_f (void)
{
	statistics_t statistics;
	if (!Statistics_Init (&statistics))
		return;

	Statistics_Json_Print (&statistics);
}

static void Statistics_Debug_f (void)
{
	Con_Printf ("mapname '%s', strlen: %ld\n", cl.mapname, strlen (cl.mapname));
	Con_Printf ("levelname '%s', strlen: %ld\n", cl.levelname, strlen (cl.levelname));
	Con_Printf ("gametype '%d'\n", cl.gametype);
}
#endif

void CL_InitStatistics (void)
{
	uuid4_seed (&state);

	Cvar_RegisterVariable (&cl_statistics_auto);
	Cvar_SetCallback (&cl_statistics_auto, CL_Statistics_Auto_f);
	Cmd_AddCommand ("statistics_cancel", Statistics_Cancel_f);
	Cmd_AddCommand ("statistics_start", Statistics_Start_f);
	Cmd_AddCommand ("statistics_stop", Statistics_Stop_f);

#if defined(STATISTICS_DEBUG)
	Cmd_AddCommand ("statistics_json_print", Statistics_Json_Print_f);
	Cmd_AddCommand ("statistics_debug", Statistics_Debug_f);
#endif
}
