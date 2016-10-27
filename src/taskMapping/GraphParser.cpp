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

#include <string>
#include <sstream>
#include <cstdlib>
#include "TaskMapping.h"
#include "../GlobalParams.h"

#include <time.h>

using namespace std;

/***************************************************************************************
 * Graph Parser Member Functions 
 ***************************************************************************************/

void TaskMapping::showPEStatistics(void)
{
    tm_peexec_statistics<int_cycles> stats_acum;

    for (map<int, tm_pe>::iterator it = tm_pes.begin(); it != tm_pes.end(); ++it)
    {
        it->second.printStats();

        stats_acum = stats_acum + it->second.stats;
    }

    TM_SIMSTATSLOG("*** pe overall ***" << endl);
    TM_SIMSTATSLOG("Total FLITs Transmitted=" << stats_acum.statGetFlitsTx() << endl
                                              << "Total FLITs Received=" << stats_acum.statGetFlitsRx() << endl
                                              << "Total Context Switching Cycles=" << stats_acum.statGetCSwitchingCycles() << endl
                                              << "Total Task Execution Cycles=" << stats_acum.statGetTskExecCycles() << endl
                                              << "Total PE Idle Cycles=" << stats_acum.statGetIdleCycles() << endl
                                              << "Total Mapped Task Cycles=" << stats_acum.statGetPeTotalCycles() << endl
                                              << "Total Energy (J)=" << stats_acum.statGetTotalEnergy_d() << endl
                                              << "      Context Switching Energy (J)=" << stats_acum.statGetCSwitchingEnergy_d() << endl
                                              << "      Task Execution Energy (J)=" << stats_acum.statGetTskExecEnergy_d() << endl
                                              << "      Idle Energy (J)=" << stats_acum.statGetIdleEnergy_d() << endl);
    TM_SIMSTATSLOG(endl);
}

/*
 * Public functions 
 */

bool GraphParser::Parser(const char *fname)
{
    ifstream infile(fname);
    const string strapp(TM_MFILE_STRAPP);
    const string strtask(TM_MFILE_STRTASK);
    const string strmap(TM_MFILE_STRMAP);
    const string strsim(TM_MFILE_STRSIM);
    char *nextChar;
    long int valueRead;
    int appid = TM_NOT_APP;

    TM_SIMCONFIGLOG(endl
                    << "************************" << endl
                    << "Parsing TGFF Graph" << endl
                    << "************************" << endl
                    << endl);

    if (infile.is_open())
    {
        // Read every line of the file
        while (infile)
        {
            string line;

            if (!getline(infile, line))
                break;

            // Parser Task Table
            if ( GetFirstString(line).compare("TASK") == 0)
            {
                // Task Table
                line = line.substr(strapp.length(), line.length() - strapp.length());
                TM_SIMCONFIGLOG("TASK Table: " <<endl << line << endl);

                istringstream segline(line);
                tm_mfile_app_fsm app_fsm = FSMAPP_APPID;
                int_cycles min_restart = 0;

                while (segline)
                {
                    string seg;

                    if (!getline(segline, seg, TM_MFILE_SEP))
                        break;

                    valueRead = strtol(seg.c_str(), &nextChar, 10);
                    TM_ASSERT(*nextChar == 0, "Characters in the file are wrong!");

                    switch (app_fsm)
                    {
                    case FSMAPP_APPID:
                        appid = valueRead;
                        TM_ASSERT(appid >= TM_APPID_MIN, "App identifier (" << appid << ") must be greater than or equal to " << TM_APPID_MIN << "!");
                        app_fsm = FSMAPP_MINRESTART;
                        break;
                    case FSMAPP_MINRESTART:
                        min_restart = valueRead;
                        TM_ASSERT(min_restart >= TM_TIMERESTART_MIN, "Restart time (" << min_restart << ") must be greater or equal to " << TM_TIMERESTART_MIN << "!");
                        addApp(appid, min_restart);
                        app_fsm = FSMAPP_IGNORE;
                        break;
                    default:
                        break;
                    }
                }
            } // End if App
            else if (line.length() >= strtask.length() && line.compare(0, strtask.length(), strtask) == 0)
            {
                // Task
                line = line.substr(strtask.length(), line.length() - strtask.length());
                TM_SIMCONFIGLOG("Task: " << line << endl);

                istringstream segline(line);
                tm_mfile_task_fsm task_fsm = FSMTASK_TASKID;
                int id = TM_NOT_TASK, staskid = TM_NOT_TASK, payload = 0;
                int_cycles exectime = 0;

                while (segline)
                {
                    string seg;

                    if (!getline(segline, seg, TM_MFILE_SEP))
                        break;

                    valueRead = strtol(seg.c_str(), &nextChar, 10);
                    TM_ASSERT(*nextChar == 0, "Characters in the file are wrong!");

                    if (appid == TM_NOT_APP)
                        break;

                    switch (task_fsm)
                    {
                    case FSMTASK_TASKID:
                        id = valueRead;
                        TM_ASSERT(id >= TM_TASKID_MIN, "Task identifier (" << id << ") must be greater than or equal to " << TM_TASKID_MIN << "!");
                        addTask(appid, id);
                        task_fsm = FSMTASK_PBLOCK_ET;
                        break;
                    case FSMTASK_PBLOCK_ET:
                        exectime = valueRead;
                        TM_ASSERT(exectime >= TM_EXECTIME_MIN, "Execution Time (" << exectime << ") cannot be less than " << TM_EXECTIME_MIN << "!");
                        task_fsm = FSMTASK_PBLOCK_ST;
                        break;
                    case FSMTASK_PBLOCK_ST:
                        staskid = valueRead;
                        TM_ASSERT(staskid >= TM_NOT_TASK, "Subsequent Task ID (" << staskid << ") shouldn't be less than " << TM_NOT_TASK << "!");
                        task_fsm = FSMTASK_PBLOCK_PL;
                        break;
                    case FSMTASK_PBLOCK_PL:
                        if (staskid != TM_NOT_TASK)
                        {
                            payload = valueRead;
                            TM_ASSERT(payload >= TM_PAYLOAD_MIN, "Payload (" << payload << ") cannot be less than " << TM_PAYLOAD_MIN << "!");
                        }
                        else
                            payload = 0;
                        addPBlockToTask(appid, id, exectime, staskid, payload);
                        task_fsm = FSMTASK_PBLOCK_ET;
                        break;
                    }
                }
            } // End if Task
            else if (line.length() >= strmap.length() && line.compare(0, strmap.length(), strmap) == 0)
            {
                // MAP
                line = line.substr(strmap.length(), line.length() - strmap.length());
                TM_SIMCONFIGLOG("MAP: " << line << endl);
                istringstream segline(line);
                tm_mfile_map_fsm map_fsm = FSMMAP_START;
                int taskid = TM_NOT_TASK, peid = TM_NOT_PE;
                int_cycles start = 0, stop = 0;
                int nmapping = tm_apps[appid].app_mappings.size();

                while (segline)
                {
                    string seg;

                    if (!getline(segline, seg, TM_MFILE_SEP))
                        break;

                    valueRead = strtol(seg.c_str(), &nextChar, 10);
                    TM_ASSERT(*nextChar == 0, "Characters in the file are wrong!");

                    switch (map_fsm)
                    {
                    case FSMMAP_START:
                        start = valueRead;
                        TM_ASSERT(start >= TM_MAPSTART_MIN, "Start Time (" << start << ") cannot be less than " << TM_MAPSTART_MIN << " in app" << appid << ", mapping" << nmapping << "!");
                        map_fsm = FSMMAP_STOP;
                        break;
                    case FSMMAP_STOP:
                        stop = valueRead;
                        TM_ASSERT(stop > start, "Stop Time (" << stop << ") must be greater than Start Time (" << start << ") in app" << appid << " mapping" << nmapping << "!");
                        addMap(appid, start, stop);
                        map_fsm = FSMMAP_TASKID;
                        break;
                    case FSMMAP_TASKID:
                        taskid = valueRead;
                        TM_ASSERT(taskid >= TM_TASKID_MIN, "Task identifier (" << taskid << ") must be greater than or equal to " << TM_TASKID_MIN << " in app" << appid << ", mapping" << nmapping - 1 << "!");
                        map_fsm = FSMMAP_PEID;
                        break;
                    case FSMMAP_PEID:
                        peid = valueRead;
                        TM_ASSERT(peid >= TM_PEID_MIN, "PE identifier (" << peid << ") must be greater than or equal to " << TM_PEID_MIN << " in app" << appid << ", mapping" << nmapping - 1 << "!");
                        addTaskToMap(appid, taskid, peid);
                        map_fsm = FSMMAP_TASKID;
                        break;
                    }
                }
            } // End if MAP
            else if (line.length() >= strsim.length() && line.compare(0, strsim.length(), strsim) == 0)
            {
                // SIM
                line = line.substr(strsim.length(), line.length() - strsim.length());
                TM_SIMCONFIGLOG("SIM: " << line << endl);

                istringstream segline(line);
                tm_mfile_sim_fsm sim_fsm = FSMSIM_SCHEDULER_TICK;
                int sch_tick = TM_PE_SCHEDULER_TICK, sch_delay = TM_PE_SCHEDULER_DELAY, sch_mapbefore = TM_PE_SCHEDULER_MAPBEFORE, sch_mappinggap = TM_PE_SCHEDULER_MAPPINGGAP;

                while (segline)
                {
                    string seg;

                    if (!getline(segline, seg, TM_MFILE_SEP))
                        break;

                    valueRead = strtol(seg.c_str(), &nextChar, 10);
                    TM_ASSERT(*nextChar == 0, "Characters in the file are wrong!");

                    switch (sim_fsm)
                    {
                    case FSMSIM_SCHEDULER_TICK:
                        sch_tick = valueRead;
                        TM_ASSERT(sch_tick >= TM_PE_SCHEDULER_TICK_MIN, "Scheduler Tick (" << sch_tick << ") must be greater than or equal to " << TM_PE_SCHEDULER_TICK_MIN << "!");
                        sim_fsm = FSMSIM_SCHEDULER_DELAY;
                        break;
                    case FSMSIM_SCHEDULER_DELAY:
                        sch_delay = valueRead;
                        TM_ASSERT(sch_delay >= TM_PE_SCHEDULER_DELAY_MIN, "Scheduler Delay (" << sch_delay << ") must be greater than or equal to " << TM_PE_SCHEDULER_DELAY_MIN << "!");
                        sim_fsm = FSMSIM_SCHEDULER_MAPBEFORE;
                        break;
                    case FSMSIM_SCHEDULER_MAPBEFORE:
                        sch_mapbefore = valueRead;
                        TM_ASSERT(sch_mapbefore >= TM_PE_SCHEDULER_MAPBEFORE_MIN, "Scheduler Map Before cycles (" << sch_mapbefore << ") must be greater than or equal to " << TM_PE_SCHEDULER_MAPBEFORE_MIN << "!");
                        sim_fsm = FSMSIM_SCHEDULER_MAPPINGGAP;
                        break;
                    case FSMSIM_SCHEDULER_MAPPINGGAP:
                        sch_mappinggap = valueRead;
                        TM_ASSERT(sch_mappinggap >= TM_PE_SCHEDULER_MAPPINGGAP_MIN, "Scheduler Mapping Gap cycles (" << sch_mappinggap << ") must be greater than " << TM_PE_SCHEDULER_MAPPINGGAP_MIN << "!");
                        scheduler_tick = sch_tick;
                        scheduler_delay = sch_delay;
                        scheduler_mapbefore = sch_mapbefore;
                        scheduler_mappinggap = sch_mappinggap;
                        sim_fsm = FSMSIM_SCHEDULER_IGNORE;
                        break;
                    default:
                        break;
                    }
                }
            } // End if SIM
        }

        TM_ASSERT(infile.eof(), "The task mapping file was not read entirely!");

        // Settle Root and Leaf Tasks
        settleRootAndLeafTasks();

        // Settle Antecedent Tasks
        settleAntecedentTasks();

        // Check Consistency
        bool cc = checkConsistency();
        TM_ASSERT(cc, "Check consistency failed!");

        // Perform the task mapping on PEs
        if (cc)
        {
            performMappingOnPEs();
        }

        return cc;
    }
    else
    {
        TM_ASSERT(infile.is_open(), "Error opening graph file.");
        return false;
    }

    return false;
}

tm_app *TaskMapping::getApp(int appid)
{
    tm_app *app = NULL;

    TM_ASSERT(tm_apps.count(appid) > 0, "App has not been added yet!");
    app = &tm_apps[appid];

    return app;
}

tm_task *TaskMapping::getTask(int appid, int taskid)
{
    tm_app *app = NULL;
    tm_task *t = NULL;

    TM_ASSERT(tm_apps.count(appid) > 0, "App has not been added yet!");
    app = &tm_apps[appid];
    TM_ASSERT(app->task_graph.count(taskid) > 0, "Task has not been added yet!");
    t = &app->task_graph[taskid];

    return t;
}

tm_pe *TaskMapping::getPE(int peid)
{
    tm_pe *pe = NULL;

    if (tm_pes.count(peid) > 0)
    {
        pe = &tm_pes[peid];
    }

    return pe;
}

int_cycles TaskMapping::getSchedulerTick()
{
    return scheduler_tick;
}

int_cycles TaskMapping::getSchedulerDelay()
{
    return scheduler_delay;
}

int_cycles TaskMapping::getSchedulerTxDelay()
{
    return scheduler_tx_delay;
}

int_cycles TaskMapping::getSchedulerMapBefore()
{
    return scheduler_mapbefore;
}

int_cycles TaskMapping::getSchedulerMappingGAP()
{
    return scheduler_mappinggap;
}

void TaskMapping::showTaskMappingConfiguration(void)
{
    TM_SIMCONFIGLOG(endl
                    << "**************************" << endl
                    << "Task Mapping Configuration" << endl
                    << "**************************" << endl
                    << endl);

    showAllAppsAndTasksInfo();
    showAllMappedTasksOnPEsInfo();
}

void TaskMapping::showTaskMappingStatistics(void)
{
    TM_SIMSTATSLOG(endl
                   << "***********************" << endl
                   << "Task Mapping Statistics" << endl
                   << "***********************" << endl
                   << endl);

    showAppStatistics();
    showPEStatistics();
}

/*

int sc_main(int argc, char* argv[]) {
  TaskMapping taskMap;
  
  srand(time(NULL));
  
  cout << endl << "\t\tNoxim - the NoC Simulator" << endl;
  cout << "\t\t(C) University of Catania" << endl << endl;

  taskMap.createTaskGraphFromFile("TaskMappingExample.txt");
  
  taskMap.showAllAppsAndTasksInfo();

  return 0;
}

*/