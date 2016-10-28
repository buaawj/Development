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

#include "TaskMapping.h"
#include "TaskMappingStats.h"

enum FSM_PARSER_DATA_TABLE_STATE
{
    FSM_PARSER_DATA_TABLE_IDLE,
    FSM_PARSER_DATA_TABLE_HEADER,
    FSM_PARSER_DATA_TABLE_DATA
};

/*************************************************
 * TGFF Graph Parser Class *
 ************************************************/
class GraphParser
{
  private:
    map<int, int> taskTable;         // KEY: task id, task type id
    map<int, int> execTimeTable;     // KEY: type id, execute time
    map<int, int> commVolumTable; // KEY: arc id,  communication volum

    FSM_PARSER_DATA_TABLE_STATE state;
    string GetFirstString(string s);
    void ParseTaskBlock(int appId, string line);
    void ParseArcBlock(int appId, string line);
    void ParseDataBlock(string line);
    int ExtractValue(string s);

  public:
    GraphParser(void)
    {
        taskTable.clear();
        execTimeTable.clear();
        commVolumTable.clear();
    }

    ~GraphParser()
    {
    }

    /* Member functions to parser graph */
    bool Parse(const char *fname);
};

#endif // __NOXIM_TGFF_GRPHY_PARSER__