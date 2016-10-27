/*
 * File added by ACAG
 *
 * TGFF Random Graph Parser for Task Mapping Module
 * (C) 2016 by the University of California Irvine
 
 * Task Mapping Module:
 * (C) 2016 by the University of Antioquia, Colombia
 *
 * Noxim - the NoC Simulator
 * 
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 */

#ifndef __NOXIM_TGFF_GRPHY_PARSER__
#define __NOXIM_TGFF_GRPHY_PARSER__

#include <vector>
#include <map>
#include <queue>
#include <iomanip>
#include <time.h>
#include <stdint.h>

#include "TaskMappingLogs.h"
#include "TaskMappingStats.h"
#include "../DataStructs.h"

#define TM_NOT_APP -1
#define TM_NOT_ROOT -1
#define TM_NOT_TASK -1
#define TM_NOT_PE -1
#define TM_START_PBLOCK -1
#define TM_START_MAPPING -1

#define TM_APPID_MIN 0
#define TM_TASKID_MIN 0
#define TM_PEID_MIN 0
#define TM_TIMERESTART_MIN 0
#define TM_EXECTIME_MIN 0
#define TM_PAYLOAD_MIN 2 // min: header and tail flits
#define TM_MAPSTART_MIN 0

#define TM_MFILE_SEP ','
#define TM_MFILE_STRAPP "app:"
#define TM_MFILE_STRTASK "task:"
#define TM_MFILE_STRMAP "map:"
#define TM_MFILE_STRSIM "sim:"

// Timing and Delays (to be included in the mapping file)
#define TM_PE_SCHEDULER_TICK 100
#define TM_PE_SCHEDULER_DELAY 10
#define TM_PE_SCHEDULER_TXSTART 1
#define TM_PE_SCHEDULER_MAPBEFORE 5
#define TM_PE_SCHEDULER_MAPPINGGAP 10
#define TM_PE_SCHEDULER_TICK_MIN 20
#define TM_PE_SCHEDULER_DELAY_MIN 1
#define TM_PE_SCHEDULER_MAPBEFORE_MIN 0
#define TM_PE_SCHEDULER_MAPPINGGAP_MIN 10

typedef int32_t int_cycles;

enum tm_mfile_app_fsm
{
    FSMAPP_APPID,
    FSMAPP_MINRESTART,
    FSMAPP_IGNORE,
};
enum tm_mfile_task_fsm
{
    FSMTASK_TASKID,
    FSMTASK_PBLOCK_ET,
    FSMTASK_PBLOCK_ST,
    FSMTASK_PBLOCK_PL
};
enum tm_mfile_map_fsm
{
    FSMMAP_START,
    FSMMAP_STOP,
    FSMMAP_TASKID,
    FSMMAP_PEID
};
enum tm_mfile_sim_fsm
{
    FSMSIM_SCHEDULER_TICK,
    FSMSIM_SCHEDULER_DELAY,
    FSMSIM_SCHEDULER_MAPBEFORE,
    FSMSIM_SCHEDULER_MAPPINGGAP,
    FSMSIM_SCHEDULER_IGNORE
};

/*************************************************
 * TGFF Graph Parser Class *
 ************************************************/
class GraphParser
{
  private:
    unordered_map<int, int> taskTable;         // KEY: task id, task type id
    unordered_map<int, int> execTimeTable;     // KEY: type id, execute time
    unordered_map<int, tm_app> commVolumTable; // KEY: arc id,  communication volum

  public:
    ofstream graphFile;

    GraphParser(string objectName)
    {
        taskTable.clear();
        execTimeTable.clear();
        commVolumTable.clear();
    }

    ~TaskMapping()
    {
        if (graphFile.is_open())
        {
            graphFile.close();
        }
    }

    /* Member functions to parser graph */
    bool parser(const char *fname);

};

#endif // __NOXIM_TGFF_GRPHY_PARSER__