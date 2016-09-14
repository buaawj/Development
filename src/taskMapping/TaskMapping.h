/*
 * File added by LGGM.
 *
 * This file contains the declaration of the task mapping simulation engine
 * 
 * Task Mapping Module: core of the Task Mapping Engine
 * (C) 2016 by the University of Antioquia, Colombia
 *
 * Noxim - the NoC Simulator
 * 
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 */

#ifndef __NOXIM_TASK_MAPPING__
#define __NOXIM_TASK_MAPPING__

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <iomanip>
#include <time.h>
#include <stdint.h>

#include "../DataStructs.h"

#define TM_NOT_APP			-1
#define TM_NOT_ROOT			-1
#define TM_NOT_TASK			-1
#define TM_NOT_PE			-1
#define TM_START_PBLOCK			-1
#define TM_START_MAPPING		-1

#define TM_APPID_MIN			0
#define TM_TASKID_MIN			0
#define TM_PEID_MIN			0
#define TM_TIMERESTART_MIN		0
#define TM_EXECTIME_MIN			0
#define TM_PAYLOAD_MIN			2	// min: header and tail flits
#define TM_MAPSTART_MIN			0

#define TM_MFILE_SEP			','
#define TM_MFILE_STRAPP			"app:"
#define TM_MFILE_STRTASK		"task:"
#define TM_MFILE_STRMAP			"map:"
#define TM_MFILE_STRSIM			"sim:"

// Timing and Delays (to be included in the mapping file)
#define TM_PE_SCHEDULER_TICK		100
#define TM_PE_SCHEDULER_DELAY		10
#define TM_PE_SCHEDULER_TXSTART		1
#define TM_PE_SCHEDULER_MAPBEFORE	5
#define TM_PE_SCHEDULER_MAPPINGGAP	10
#define TM_PE_SCHEDULER_TICK_MIN	20
#define TM_PE_SCHEDULER_DELAY_MIN	1
#define TM_PE_SCHEDULER_MAPBEFORE_MIN	0
#define TM_PE_SCHEDULER_MAPPINGGAP_MIN	10

// ***************************
// Messages, Warnings and Info
// ***************************

#define _TM_ASSERT(X,MSG,EXIT)  if ( !(X) ) { \
                                  double ccycle = sc_time_stamp().to_double() / GlobalParams::clock_period_ps; \
                                  cout << endl \
                                       << "******************" << endl \
                                       << "ABNORMAL SITUATION : (cycle=" << ccycle << ") " << MSG << endl \
                                       << "******************" << endl; \
                                  if (EXIT) \
                                     assert((X)); \
                                } 

// if DEFINE_TM_ASSERT is defined, tm_engine stops when anormal situations occur
// #define  DEFINE_TM_ASSERT
#ifdef   DEFINE_TM_ASSERT
 #define TM_ASSERT(X,MSG)       _TM_ASSERT(X,MSG,1); 
#else
 #define TM_ASSERT(X,MSG)       _TM_ASSERT(X,MSG,0); 
#endif

// if DEFINE_TM_WARNING is defined, tm_engine shows warnings when they occur
// #define  DEFINE_TM_WARNING
#ifdef   DEFINE_TM_WARNING
 #define TM_WARNING(MSG)         { \
			            double ccycle = sc_time_stamp().to_double() / GlobalParams::clock_period_ps; \
				    cout << endl \
					<< "*******" << endl \
					<< "WARNING : (" << ccycle << ") " << MSG << endl \
					<< "*******" << endl; \
				 }	
#else
 #define TM_WARNING(MSG)
#endif

#define TM_SIMWRITELOG(MSG) \
        { \
          if (tmInstance->taskmappinglog_file.is_open()) { \
            tmInstance->taskmappinglog_file << MSG; \
          } \
          else { \
            cout << MSG; \
          } \
        } 

// if DEFINE_TM_SIMCONFIGLOG is defined, tm_engine writes simulation configuration logs to the specified log file
// #define DEFINE_TM_SIMCONFIGLOG
#ifdef DEFINE_TM_SIMCONFIGLOG
 #define TM_SIMCONFIGLOG(MSG) TM_SIMWRITELOG(MSG)
#else
 #define TM_SIMCONFIGLOG(MSG)
#endif

// if DEFINE_TM_SIMEXECLOG is defined, tm_engine writes simulation execution logs to the specified log file
// if TM_SIMEXECLOG_EXTENDED is defined, tm_engine writes extended logs
// #define  DEFINE_TM_SIMEXECLOG
// #define  TM_SIMEXECLOG_EXTENDED
#define TM_SIMEXECLOG_BEINGMAPPEDSTATE                  0
#define TM_SIMEXECLOG_WAITINGRXSTATE                    1
#define TM_SIMEXECLOG_SLEEPINGSTATE                     2
#define TM_SIMEXECLOG_READYSTATE                        3
#define TM_SIMEXECLOG_READYSTATEDUETOCONTEXTSWITCHING   4
#define TM_SIMEXECLOG_CONTEXTSWITCHINGSTATE             5
#define TM_SIMEXECLOG_RUNNINGSTATE                      6
#define TM_SIMEXECLOG_STOPSANDUNMAPPED                  7
#define TM_SIMEXECLOG_RECEIVESALLANT                    8
#define TM_SIMEXECLOG_STARTSEXEC                        9
#define TM_SIMEXECLOG_CONTINUESEXECAFTERCONEXTSW        10
#define TM_SIMEXECLOG_COMPLETESPB                       11
#define TM_SIMEXECLOG_COMPLETESALLPBS                   12
#define TM_SIMEXECLOG_PAYLOADREADYTOTRANSF              13
#define TM_SIMEXECLOG_CONTINUESEXEC                     14
#define TM_SIMEXECLOG_PLBEINGTRANSTOSAMEPE              15
#define TM_SIMEXECLOG_PLBEINGTRANSTOWNOC                16
#define TM_SIMEXECLOG_PLTOTALLYOUTTOSAMEPE              17
#define TM_SIMEXECLOG_PLTOTALLYOUTTOWNOC                18
#define TM_SIMEXECLOG_RECEIVESPLFROMSAMEPE              19
#define TM_SIMEXECLOG_RECEIVESPLFROMWNOC                20
#define TM_SIMEXECLOG_DESTNOTREACHEABLE                 21

#ifdef   DEFINE_TM_SIMEXECLOG
 #ifdef   TM_SIMEXECLOG_EXTENDED
  #define TM_SIMEXECLOG(CODE,PE,APP,TSK,MSG) \
          { \
            if (tmInstance->taskmappinglog_file.is_open()) { \
              double ccycle = sc_time_stamp().to_double() / GlobalParams::clock_period_ps; \
              tmInstance->taskmappinglog_file << "(" << setfill('0') << setw(2) << CODE << ") " \
                                              << "@" << setfill('0') << setw(6) << ccycle << " " \
                                              << "pe" << setfill('0') << setw(3) << PE << "," \
                                              << "app" << setfill('0') << setw(2) << APP << "," \
                                              << "tsk" << setfill('0') << setw(2) << TSK \
                                              << " -> " << MSG << endl; \
            } \
          } 
 #else
  #define TM_SIMEXECLOG(CODE,PE,APP,TSK,MSG) \
          { \
            if (tmInstance->taskmappinglog_file.is_open()) { \
              double ccycle = sc_time_stamp().to_double() / GlobalParams::clock_period_ps; \
              tmInstance->taskmappinglog_file << "(" << setfill('0') << setw(2) << CODE << ") " \
                                              << "@" << setfill('0') << setw(6) << ccycle << " " \
                                              << "pe" << setfill('0') << setw(3) << PE << "," \
                                              << "app" << setfill('0') << setw(2) << APP << "," \
                                              << "tsk" << setfill('0') << setw(2) << TSK << endl; \
            } \
          } 
 #endif
#else
 #define TM_SIMEXECLOG(CODE,PE,APP,TSK,MSG)
#endif

#define TM_SIMSTATSLOG_STATLATENCYPE2PE                 30
#define TM_SIMSTATSLOG_STATAPPEXECTIME                  31
#define TM_SIMSTATSLOG_STATAPPLATENCY                   32
#define TM_SIMSTATSLOG_STATPECYCLESANDENERGY            33

// if DEFINE_TM_SIMSTATSLOG is defined, tm_engine writes simulation statistics logs to the specified log file
// #define DEFINE_TM_SIMSTATSLOG
#ifdef DEFINE_TM_SIMSTATSLOG
 #define TM_SIMSTATSLOG(MSG) TM_SIMWRITELOG(MSG)
 #define TM_SIMPEMETRICSLOG(CODE,PE,MSG1,MSG2) \
         { \
           double ccycle = sc_time_stamp().to_double() / GlobalParams::clock_period_ps; \
           if (tmInstance->taskmappinglog_file.is_open()) { \
             tmInstance->taskmappinglog_file << "(" << setfill('0') << setw(2) << CODE << ") " \
                                             << "@" << setfill('0') << setw(6) << ccycle << " "\
                                             << "pe" << setfill('0') << setw(3) << PE << "," << MSG1 << " -> " \
                                             << MSG2 << endl; \
           } \
           else \
           { \
             cout << "(" << setfill('0') << setw(2) << CODE << ") " \
                  << "@" << setfill('0') << setw(6) << ccycle << " "\
                  << "pe" << setfill('0') << setw(3) << PE << "," << MSG1 << " -> " \
                  << MSG2 << endl; \
           } \
         } 
 #define TM_SIMAPPMETRICSLOG(CODE,APP,MSG1,MSG2) \
         { \
           double ccycle = sc_time_stamp().to_double() / GlobalParams::clock_period_ps; \
           if (tmInstance->taskmappinglog_file.is_open()) { \
             tmInstance->taskmappinglog_file << "(" << setfill('0') << setw(2) << CODE << ") " \
                                             << "@" << setfill('0') << setw(6) << ccycle << " "\
                                             << "app" << setfill('0') << setw(2) << APP << "," << MSG1 << " -> " \
                                             << MSG2 << endl; \
           } \
           else \
           { \
             cout << "(" << setfill('0') << setw(2) << CODE << ") " \
                  << "@" << setfill('0') << setw(6) << ccycle << " "\
                  << "app" << setfill('0') << setw(2) << APP << "," << MSG1 << " -> " \
                  << MSG2 << endl; \
           } \
         } 
 #define TM_SIMPE2PELATENCYLOG(CODE,APP,TSK1,TSK2,MSG) \
         { \
           double ccycle = sc_time_stamp().to_double() / GlobalParams::clock_period_ps; \
           if (tmInstance->taskmappinglog_file.is_open()) { \
             tmInstance->taskmappinglog_file << "(" << setfill('0') << setw(2) << CODE << ") " \
                                             << "@" << setfill('0') << setw(6) << ccycle << " " \
                                             << "app" << setfill('0') << setw(2) << APP << "," \
                                             << "tsk" << setfill('0') << setw(2) << TSK1 << "->" \
                                             << "tsk" << setfill('0') << setw(2) << TSK2 << "," \
                                             << "latency -> " << MSG << endl; \
           } \
           else \
           { \
             cout << "(" << setfill('0') << setw(2) << CODE << ") " \
                  << "@" << setfill('0') << setw(6) << ccycle << " " \
                  << "app" << setfill('0') << setw(2) << APP << "," \
                  << "tsk" << setfill('0') << setw(2) << TSK1 << "->" \
                  << "tsk" << setfill('0') << setw(2) << TSK2 << "," \
                  << "latency -> " << MSG << endl; \
           } \
         } 
#else
 #define TM_SIMSTATSLOG(MSG)
 #define TM_SIMPEMETRICSLOG(CODE,PE,MSG1,MSG2)
 #define TM_SIMAPPMETRICSLOG(CODE,APP,MSG1,MSG2)
 #define TM_SIMPE2PELATENCYLOG(CODE,APP,TSK1,TSK2,MSG)
#endif

#define TM_STRAPPTASK(T)         "app" << (T)->getApp()->getId() << ",task" << (T)->getId()  

typedef int32_t int_cycles;

enum tm_mfile_app_fsm {
  FSMAPP_APPID,
  FSMAPP_MINRESTART,
  FSMAPP_IGNORE,
};
enum tm_mfile_task_fsm {
  FSMTASK_TASKID,
  FSMTASK_PBLOCK_ET,
  FSMTASK_PBLOCK_ST,
  FSMTASK_PBLOCK_PL
};
enum tm_mfile_map_fsm {
  FSMMAP_START,
  FSMMAP_STOP,
  FSMMAP_TASKID,
  FSMMAP_PEID
};
enum tm_mfile_sim_fsm {
  FSMSIM_SCHEDULER_TICK,
  FSMSIM_SCHEDULER_DELAY,
  FSMSIM_SCHEDULER_MAPBEFORE,
  FSMSIM_SCHEDULER_MAPPINGGAP,
  FSMSIM_SCHEDULER_IGNORE
};

enum tm_task_execstates {
  TM_TASK_EXECSTATE_UNMAPPED = 0,
  TM_TASK_EXECSTATE_BEINGMAPPED,
  TM_TASK_EXECSTATE_WAITINGRX,
  TM_TASK_EXECSTATE_SLEEPING,
  TM_TASK_EXECSTATE_READY,
  TM_TASK_EXECSTATE_SWITCHINGCONTEXT,
  TM_TASK_EXECSTATE_RUNNING
};  

using namespace std;

/*******************************
 * Classes to store statistics *
 *******************************/ 
template <class T> 
class tm_peexec_statistics {
private:
  T         _stat_cswitching_acum;
  T         _stat_tskexec_acum;
  T         _stat_idle_acum;
  
  T         _stat_cycles_total;
  double    _stat_cswitching_energy_d;
  double    _stat_tskexec_energy_d;
  double    _stat_idle_energy_d;
  double    _stat_total_energy_d;
  
public:
  tm_peexec_statistics() {
    _stat_cswitching_acum = 0;
    _stat_tskexec_acum = 0;
    _stat_idle_acum = 0;
    
    _stat_cycles_total = 0;
    _stat_cswitching_energy_d = 0;
    _stat_tskexec_energy_d = 0;
    _stat_idle_energy_d = 0;
    _stat_total_energy_d = 0;
  }
  tm_peexec_statistics<T> operator+(const tm_peexec_statistics<T>& other) {
    tm_peexec_statistics<T> temp;
    temp._stat_cswitching_acum = _stat_cswitching_acum + other._stat_cswitching_acum;
    temp._stat_tskexec_acum = _stat_tskexec_acum + other._stat_tskexec_acum;
    temp._stat_idle_acum = _stat_idle_acum + other._stat_idle_acum;
    
    temp._stat_cycles_total = _stat_cycles_total + other._stat_cycles_total;
    temp._stat_cswitching_energy_d = _stat_cswitching_energy_d + other._stat_cswitching_energy_d;
    temp._stat_tskexec_energy_d = _stat_tskexec_energy_d + other._stat_tskexec_energy_d;
    temp._stat_idle_energy_d = _stat_idle_energy_d + other._stat_idle_energy_d;
    temp._stat_total_energy_d = _stat_total_energy_d + other._stat_total_energy_d;
    
    return temp;
  }
  
  inline void statAddCSwitchingCycles(T cycles) {
    this->_stat_cswitching_acum += cycles;
  }
  inline T statGetCSwitchingCycles(void) {
    return _stat_cswitching_acum;
  }
  inline void statAddTskExecCycles(T cycles) {
    this->_stat_tskexec_acum += cycles;
  }
  inline T statGetTskExecCycles(void) {
    return _stat_tskexec_acum;
  }
  inline void statAddIdleCycles(T cycles) {
    this->_stat_idle_acum += cycles;
  }
  inline T statGetIdleCycles(void) {
    return _stat_idle_acum;
  }
  inline void statComputePeTotalCyclesAndEnergy(void) {
    this->_stat_cycles_total = _stat_cswitching_acum + 
                               _stat_tskexec_acum + 
                               _stat_idle_acum;
    
    _stat_cswitching_energy_d = GlobalParams::power_configuration.pePowerConfig.running_mode * (_stat_cswitching_acum);
    _stat_tskexec_energy_d = GlobalParams::power_configuration.pePowerConfig.running_mode * (_stat_tskexec_acum);
    _stat_idle_energy_d = GlobalParams::power_configuration.pePowerConfig.idle_mode * (_stat_idle_acum);
    _stat_total_energy_d = _stat_cswitching_energy_d + _stat_tskexec_energy_d + _stat_idle_energy_d;
  }
  inline T statGetPeTotalCycles(void) {
    return _stat_cycles_total;
  }
  inline double statGetCSwitchingEnergy_d(void) {
    return this->_stat_cswitching_energy_d;
  }
  inline double statGetTskExecEnergy_d(void) {
    return this->_stat_tskexec_energy_d;
  }
  inline double statGetIdleEnergy_d(void) {
    return this->_stat_idle_energy_d;
  }
  inline double statGetTotalEnergy_d(void) {
    return this->_stat_total_energy_d;
  }
};

template <class T> 
class tm_appexec_statistics {
private:
  int       _stat_exec_number;
  double    _stat_avg;
  T         _stat_last;
  T         _stat_min;
  T         _stat_max;

public:  
  tm_appexec_statistics() {
    _stat_exec_number = 0;
    _stat_avg = 0;
    _stat_last = 0;
    _stat_min = 0;
    _stat_max = 0;
  }
  tm_appexec_statistics<T> operator+(const tm_appexec_statistics<T>& other) {
    tm_appexec_statistics<T> temp;
    temp._stat_exec_number = _stat_exec_number + other._stat_exec_number;
    temp._stat_avg = _stat_avg + other._stat_avg;
    temp._stat_last = _stat_last + other._stat_last;
    temp._stat_min = _stat_min + other._stat_min;
    temp._stat_max = _stat_max + other._stat_max;  
    return temp;
  }
  
  inline int statGetExecNumber(void) { 
    return (_stat_exec_number);
  }
  inline void statSetExecNumber(T _stat_exec_number) {
    this->_stat_exec_number = _stat_exec_number;
  }
  inline double statGetAverage(void) {
    return (_stat_avg);
  }
  inline void statSetAverage(T _stat_avg) {
    this->_stat_avg = _stat_avg;
  }
  inline T statGetLast(void) {
    return (_stat_last);
  }
  inline void statSetLast(T _stat_last) {
    this->_stat_last = _stat_last;
  }
  inline T statGetMin(void) {
    return (_stat_min);
  }
  inline void statSetMin(T _stat_min) {
    this->_stat_min = _stat_min;
  }
  inline T statGetMax(void) {
    return (_stat_max);
  }
  inline void statSetMax(T _stat_max) {
    this->_stat_max = _stat_max;
  }

  void statAddNewValue(T stat_newvalue) {
    _stat_last = stat_newvalue;
    _stat_exec_number++;
    _stat_avg = ((_stat_avg * (_stat_exec_number-1)) + stat_newvalue) / (_stat_exec_number);
    if (stat_newvalue < _stat_min || _stat_exec_number == 1)
      _stat_min = stat_newvalue;
    if (stat_newvalue > _stat_max)
      _stat_max = stat_newvalue;
  }
};

/************************************************
 * Classes to store the information of each app *
 ************************************************/ 
class tm_task;
class tm_appmapping{
private:
  int_cycles		_start;		// time when the root task should start
  int_cycles		_stop;		// time when the root task should stop

  // Statistics
  queue <int_cycles>    _stat_startx;
  bool                  _stat_taskstarted;
    
public:    
  map < int, int > 	task_on_pe;	// Task on which PE? Task id, pe id

  // Statistics
  tm_appexec_statistics <int_cycles>    stat_exectime;
  tm_appexec_statistics <int_cycles>    stat_delay;
  
  // Constructor
  tm_appmapping() {
    _start = 0;
    _stop = 0;
    task_on_pe.clear();
    
    _stat_startx = queue <int_cycles>();
    _stat_taskstarted = false;
  }
  
  inline int_cycles getStartTime(void) {
    return _start;
  }
  inline void setStartTime(int_cycles start) {
    this->_start = start;
  }
  inline int_cycles getStopTime(void) {
    return _stop;
  }
  inline void setStopTime(int_cycles stop) {
    this->_stop = stop;
  }
  
  void statRootExecStarted(void);
  inline void statRootExecFinished(void) {
    _stat_taskstarted = false;
  }
  void statLeafExecFinished(tm_task * t_leaf);
};

class tm_app{
private:    
  int           		_id;		// the app id is also stored as a key in the map object
  int  				_root_taskid;	// id of the root task
  int_cycles			_min_restart;	// minimum time between the last pblock and the first one
  int  				_leaf_taskid;	// id of the leaf task
  
  // Mapping
  int 				_map_currently_running;
  bool                          _currently_mapped;
  int_cycles			_start;		// time when the root task should start
  int_cycles			_stop;		// time when the root task should stop
  
public:    
  map < int, tm_task > 		task_graph;	// KEY: task id, T: tm_task struct
  
  // Mapping
  vector < tm_appmapping > 	app_mappings;
  
  // Structure constructor
  tm_app() {
    _id = TM_NOT_APP;
    _root_taskid = TM_NOT_TASK;
    _min_restart = 0;
    _leaf_taskid = TM_NOT_TASK;
    task_graph.clear();
    
    _map_currently_running = TM_START_MAPPING;
    _currently_mapped = false;
    _start = 0;
    _stop = 0;
    app_mappings.clear();
  }
  
  inline int getId(void) { 
    return _id; 
  }
  inline void setId(int id) { 
    this->_id = id;
  }
  inline int getRootId(void) {
    return _root_taskid; 
  }
  inline void setRootId(int root_taskid) {
    this->_root_taskid = root_taskid; 
  }
  inline int_cycles getMinRestart(void) {
    return _min_restart; 
  }
  inline void setMinRestart(int_cycles min_restart) {
    this->_min_restart = min_restart; 
  }
  inline int getLeafId(void) {
    return _leaf_taskid; 
  }
  inline void setLeafId(int leaf_taskid) {
    this->_leaf_taskid = leaf_taskid; 
  }
  
  inline int getMapCurrentlyRunning(void) {
    return _map_currently_running;  
  }
  inline void setMapCurrentlyRunning(int map_currently_running) {
    this->_map_currently_running = map_currently_running;  
  }
  inline bool getCurrentlyMapped(void) {
    return _currently_mapped;  
  }
  inline void setCurrentlyMapped(bool currently_mapped) {
    this->_currently_mapped = currently_mapped;  
  }
  inline int_cycles getStartTime(void) {
    return _start;  
  }
  inline void setStartTime(int_cycles start) {
    this->_start = start;  
  }
  inline int_cycles getStopTime(void) {
    return _stop;  
  }
  inline void setStopTime(int_cycles stop) {
    this->_stop = stop;  
  }
  
  void appBeingMapped(int_cycles start, int_cycles stop);
  void appBeingUnmapped();
  inline tm_appexec_statistics <int_cycles> * getCurrentStatsExecTime(void) {
    return (&app_mappings[_map_currently_running].stat_exectime);  
  }
  inline tm_appexec_statistics <int> * getCurrentStatsLatency(void) {
    return (&app_mappings[_map_currently_running].stat_delay);  
  }
  inline tm_appmapping * getCurrentMapping(void) {
    return (&app_mappings[_map_currently_running]);  
  }
  inline int getCurrentMappingInt(void) {
    return _map_currently_running;  
  }
  void printSubsequentBranchLatencies(int imap, tm_task * t, string initss = "", tm_appexec_statistics <int> initstat = tm_appexec_statistics <int>());
  void printStats(void);
  
  bool alreadyStopped ();
};

/*************************************************
 * Classes to store the information of each task *
 *************************************************/ 
class tm_pblock_task {
private:    
  int_cycles	_exectime;	// time in cycles for this block after which the payload is sent to the subsequent task
  int  		_staskid;	// task ID of the subsequent task, it could be -1 (No Task Associated)
  int  		_payload;	// packets to be sent to the subsequent task, it could be 0 (No payload)

public:    
  // Statistics
  map < int, tm_appexec_statistics < int > >  stat_txdelay; // mapping ID, statistics
  map < int, tm_appexec_statistics < int > >  stat_txhops; // mapping ID, statistics
  
  tm_pblock_task() {
    _exectime = 0;
    _staskid = TM_NOT_TASK;
    _payload = 0;
  }  
  
  inline int_cycles getExecTime(void) {
    return _exectime;
  }
  inline void setExecTime(int_cycles exectime) {
    this->_exectime = exectime;
  }
  inline int getSTaskId(void) {
    return _staskid;
  }
  inline void setSTaskId(int staskid) {
    this->_staskid = staskid;
  }
  inline int getPayLoad(void) {
    return _payload;  
  }
  inline void setPayLoad(int payload) {
    this->_payload = payload;  
  }
};

class tm_antecedent_task {
private:    
  int 	    _pb;		// index of the pblock in the antecedent task
  tm_task * _at;		// Pointer to the antecedent task
  
  int	    _num_rx;	// Number of packets received without being processed

public:  
  tm_antecedent_task() {
    _pb = 0;
    _at = NULL;
    _num_rx = 0;
  }
  
  inline int getPbId(void) {
    return _pb;
  }
  inline void setPbId(int pb) {
    this->_pb = pb;
  }
  inline tm_task * getATask(void) {
    return _at;
  }
  inline void setATask(tm_task * at) {
    this->_at = at;
  }
  inline int getNumRx(void) {
    return _num_rx;
  }
  inline void incNumRx(void) {
    _num_rx++;
  }
  inline void decNumRx(void) {
    _num_rx--;
  }
  inline void resetNumRx(void) {
    _num_rx=0;
  }
};

class tm_task{
private:
  int    			_id;			// the task id is also stored as a key in the map object

  tm_app * 			_ptr_parentapp; 	// pointer to the parent application
  int	 			_peid_container;	// Container PE id
  
  int 	 			_currentPBlock;		// block being currently executed
  int_cycles 			_exectime_left;		// execution time left in the execution of the current processing block
  tm_task_execstates 		_execution_state;	// indicates the current execution state of the task
    
public:    
  vector < tm_pblock_task > 	pblocks_tasks;  	// vector of processing blocks of each task (related to subsequent tasks)
  vector < tm_antecedent_task > antecedent_tasks; 	// vector of antecedent tasks (pointer to tm_antecedent_task struct)
  
  tm_task() {
    _id = TM_NOT_TASK;
    pblocks_tasks.clear();
    antecedent_tasks.clear();
    
    _ptr_parentapp = NULL;
    _peid_container = TM_NOT_PE;
    
    _currentPBlock = TM_START_PBLOCK;
    _exectime_left = 0;
    _execution_state = TM_TASK_EXECSTATE_UNMAPPED;
  }
  
  inline int getId(void) {
    return _id;
  }
  inline void setId(int id) {
    this->_id = id;
  }
  inline tm_app * getApp(void) {
    return _ptr_parentapp;
  }
  inline void setApp(tm_app * ptr_parentapp) {
    this->_ptr_parentapp = ptr_parentapp;
  }
  inline int getPeId(void) {
    return _peid_container;
  }
  inline void setPeId(int peid_container) {
    this->_peid_container = peid_container;
  }
  inline int getCurrentPb(void) {
    return _currentPBlock;
  }
  inline void setCurrentPb(int currentPBlock) {
    this->_currentPBlock = currentPBlock;
  }
  inline void incCurrentPb(void) {
    this->_currentPBlock++;
  }
  inline void resetCurrentPb(void) {
    this->_currentPBlock = 0;
  }
  inline int_cycles getExectimeLeft(void) {
    return _exectime_left;
  }
  inline void setExectimeLeft(int_cycles exectime_left) {
    this->_exectime_left = exectime_left;
  }
  inline tm_task_execstates getExecState(void) {
    return _execution_state;
  }
  inline void setExecState(tm_task_execstates execution_state) {
    this->_execution_state = execution_state;
  }

  inline bool isInBeingMappedState() { 
    return (_execution_state == TM_TASK_EXECSTATE_BEINGMAPPED); 
  }
  void toBeingMappedState(int peid_container, int_cycles start, int_cycles stop);
  inline bool isInWaitingRxState() { 
    return (_execution_state == TM_TASK_EXECSTATE_WAITINGRX); 
  }
  void toWaitingRxState();
  inline bool isInSleepingState() { 
    return (_execution_state == TM_TASK_EXECSTATE_SLEEPING); 
  }
  void toSleepingState();
  inline bool isInReadyState() { 
    return (_execution_state == TM_TASK_EXECSTATE_READY); 
  }
  void toReadyState();
  void toReadyStateDueToContextSwitching();
  inline bool isInContextSwitchingState() { 
    return (_execution_state == TM_TASK_EXECSTATE_SWITCHINGCONTEXT); 
  };
  void toContextSwitchingState();
  inline bool isInRunningState() { 
    return (_execution_state == TM_TASK_EXECSTATE_RUNNING); 
  }
  void toRunningState();
  void toStopAndUnmapState();
  void taskExecutionRunning();
  void taskExecutionCompleted();
  tm_appexec_statistics <int> getTotalLatencyUptoThis(int imap);
    
  inline bool isRootTask() {
    return (_id == _ptr_parentapp->getRootId());
  }
  inline bool isLeafTask() {
    return (_id == _ptr_parentapp->getLeafId());
  }
};

/**************************************************
 * Structures to store the information of each PE *
 **************************************************/ 
class tm_mappingonpe{
private:
  int_cycles		_stop;		// time when the unmapping must be performed
    
public:    
  vector < tm_task * > 	tasks_on_pe;	// Task in the PE
  
  tm_mappingonpe() {
    _stop = 0;
    tasks_on_pe.clear();
  }
  
  inline int_cycles getStopTime(void) {
    return _stop;
  }
  inline void setStopTime(int_cycles stop) {
    this->_stop = stop;
  }
};

/*
 * Structure to store the PE information
 */ 
class tm_pe{
private:    
  int _id;		// This id is store as a KEY of the object
  
public:
  map < int_cycles, tm_mappingonpe >		mappingsonpe;	// KEY: time event (start), T: structure tm_mappingonpe
  map < int_cycles, tm_mappingonpe >::iterator 	currentmapping;	// iterator

  // Statistics
  tm_peexec_statistics < int_cycles > stats;
  
  tm_pe() {
    _id = TM_NOT_PE;
    mappingsonpe.clear();
    currentmapping = mappingsonpe.end();
  }
  
  inline int getId(void) {
    return _id;
  }
  inline void setId(int id) {
    this->_id = id;
  }
  void printStats(void);
};

// General class for this module: TaskMapping
class TaskMapping {
private:
  map < int, tm_app > tm_apps;	// KEY: App id, T: tm_app struct
  map < int, tm_pe > tm_pes;	// KEY: PE id, T: tm_pe struct
  string objectName;
  
  int_cycles scheduler_tick, scheduler_delay, scheduler_tx_delay;
  int_cycles scheduler_mapbefore, scheduler_mappinggap;
  
  void addApp(int appid, int_cycles min_restart);
  void addTask(int appid, int taskid);
  void addPBlockToTask(int appid, int taskid, int_cycles exectime, int staskid, int payload); 
  void addMap(int appid, int_cycles start, int_cycles stop);
  void addTaskToMap(int appid, int taskid, int peid);
  void deleteCurrentMapping(void);
  void settleRootAndLeafTasks(void);
  void settleAntecedentTasks(void);
  bool checkConsistency(void);
  void performMappingOnPEs(void);
  void showAppInfo(tm_app *app);
  void showTasksPerAppInfo(tm_task *t);
  void showMappingsPerAppInfo(tm_app *app);
  void showAllAppsAndTasksInfo(void);
  void showMappedTasksPerPEInfo(tm_pe *pe);
  void showAllMappedTasksOnPEsInfo(void);
  void showAppStatistics(void);
  void showPEStatistics(void);
  void showPESummaryStatistics(void);
  
public:
  ofstream taskmappinglog_file;
  
  TaskMapping(string objectName) {
    tm_apps.clear();
    tm_pes.clear();
    
    this->objectName = objectName;
    
    scheduler_tick = TM_PE_SCHEDULER_TICK;
    scheduler_delay = TM_PE_SCHEDULER_DELAY;
    scheduler_tx_delay = TM_PE_SCHEDULER_TXSTART;
    scheduler_mapbefore = TM_PE_SCHEDULER_MAPBEFORE;
    scheduler_mappinggap = TM_PE_SCHEDULER_MAPPINGGAP;
    
    if (strcmp(GlobalParams::taskmappinglog_filename.c_str(), TM_NOT_FILE)) {
      taskmappinglog_file.open(GlobalParams::taskmappinglog_filename.c_str(), ofstream::out);
      if (!taskmappinglog_file.is_open()) {
	  cerr << "The TaskMapping Log file " << GlobalParams::taskmappinglog_filename << " could not be created!" << endl;
	  exit(0);
      }
      else {
	time_t rawtime;
	struct tm * timeinfo;
	
	cout << "Task Mapping Log file is: \"" << GlobalParams::taskmappinglog_filename << "\"" << endl;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	taskmappinglog_file << "Task Mapping Simulation File created on: " << asctime(timeinfo);
      }
    }
  }

  ~TaskMapping() {
    if (taskmappinglog_file.is_open()) {
      taskmappinglog_file.close();
    }
  }
  
  bool createTaskGraphFromFile(const char * fname);
  tm_app * getApp(int appid);
  tm_task * getTask(int appid, int taskid);
  tm_pe * getPE(int peid);
  int_cycles getSchedulerTick();
  int_cycles getSchedulerDelay();
  int_cycles getSchedulerTxDelay();
  int_cycles getSchedulerMapBefore();
  int_cycles getSchedulerMappingGAP();
  
  void showTaskMappingConfiguration(void);
  void showTaskMappingStatistics(void);

  inline string name() {
    return objectName;
  }
};

extern TaskMapping * tmInstance;

#endif // __NOXIM_TASK_MAPPING__