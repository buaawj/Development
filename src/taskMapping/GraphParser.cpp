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
#include "GraphParser.h"

using namespace std;
extern TaskMapping *tmInstance;
/***************************************************************************************
 * Graph Parser Member Functions 
 ***************************************************************************************/
bool GraphParser::Parse(const char *fname)
{
    ifstream infile(fname);
    FSM_PARSER_DATA_TABLE_STATE state = FSM_PARSER_DATA_TABLE_IDLE;

    cout << endl;
    cout <<"************************" << endl;
    cout <<"Parsing TGFF Graph" << endl;
    cout <<"************************" << endl;
    cout <<endl;

    // Create appliation first
    int appId = 1;
    tmInstance->addApp(appId, 100000);
    if (infile.is_open())
    {
        // Read every line of the file
        while (infile)
        {
            string line;
           
            if (!getline(infile, line))
                break;

            string keyWord = GetFirstString(line);
            cout<<"**"<<keyWord<< "  " <<keyWord.size() <<endl;
            if (keyWord.compare("TASK") == 0)
            {
                // Task Block
                ParseTaskBlock(appId, line);
            }
            else if (keyWord.compare("ARC") == 0)
            {
                // ARC Block
                ParseArcBlock(appId, line);
            }
            else if (keyWord.compare("@COMMUN") == 0 || state != FSM_PARSER_DATA_TABLE_IDLE)
            {
                // Execute time and communication volume table
                switch (state)
                {
                case FSM_PARSER_DATA_TABLE_IDLE:
                    state = FSM_PARSER_DATA_TABLE_HEADER;
                    break;
                case FSM_PARSER_DATA_TABLE_HEADER:
                    if (line.find("Type") != string::npos)
                        state = FSM_PARSER_DATA_TABLE_DATA;
                    break;
                case FSM_PARSER_DATA_TABLE_DATA:
                    ParseDataBlock(line);
                    break;
                default:
                    break;
                }
            }
        } // End while

        TM_ASSERT(infile.eof(), "The task mapping file was not read entirely!");

        /* // Settle Root and Leaf Tasks
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
        */
        return true;
    }
    else
    {
        TM_ASSERT(infile.is_open(), "Error opening graph file.");
        return false;
    }

    return false;
}

string GraphParser::GetFirstString(string s)
{
    string ret;
    if (s.empty())
        return ret;

    int start = 0, end = 0, n = s.length();
    while (start < n && (s[start]==' ' || s[start]=='   '))
        ++start;
    if (start == n)
        return ret;
    for (end = start; end < n && s[end]!=' '; ++end)
        ;
    return s.substr(start, end - start);
}

/*  State Machine Functions */
void GraphParser::ParseTaskBlock(int appId, string line)
{
    string buf;
    stringstream ss(line);
    vector<string> tokens;
    int taskId = 0, taskType = 0;
    cout<< "Parse Task Block"<<endl;
    cout<<line<<endl<<endl;
    while (ss >> buf)
    {
        tokens.push_back(buf);
    }
    cout<<"Debug"<<endl;
    for(int i=0; i<tokens.size(); ++i)
        cout<<tokens[i]<<endl;
    TM_ASSERT(tokens.size() == 4, "Error parsing Task block.");
    taskId = ExtractValue(tokens[1]);
    taskType = atoi(tokens[tokens.size() - 1].c_str());

    // Add task into App
    tmInstance->addTask(appId, taskId);
    taskTable[taskId] = taskType;
}

void GraphParser::ParseArcBlock(int appId, string line)
{
    cout<< "Parse Arc Block"<<endl;
    cout<<line<<endl<<endl;

    string buf;
    stringstream ss(line);
    vector<string> tokens;
    while (ss >> buf)
    {
        tokens.push_back(buf);
    }

    TM_ASSERT(tokens.size() == 8, "Error parsing Task block.");
    int src = ExtractValue(tokens[3]);
    int dst = ExtractValue(tokens[5]);
    int typeId = atoi(tokens[7].c_str());

    // Add edge into App graph
    // For now, put execute time and payload as 0, since
    // we are going to populate this later.
    tmInstance->addPBlockToTask(appId, src, 0, dst, 0);
}

void GraphParser::ParseDataBlock(string line)
{
    cout<< "Parse Data Block"<<endl;
    cout<<line<<endl<<endl;
    string buf;
    stringstream ss(line);
    vector<string> tokens;
    int taskId = 0;

    while (ss >> buf)
    {
        tokens.push_back(buf);
    }

    TM_ASSERT(tokens.size() == 3, "Error parsing Data block.");
    int typeId = atoi(tokens[0].c_str());

    // Update data table
    execTimeTable[typeId] = atoi(tokens[1].c_str());
    commVolumTable[typeId] = atoi(tokens[2].c_str());
}

int GraphParser::ExtractValue(string s)
{
    TM_ASSERT(s.empty() == false, "Error parsing block.");

    int pos = s.find('-');
    TM_ASSERT(pos != string::npos, "Error cannot find _");

    return atoi(s.substr(pos).c_str());
}

int sc_main(int argc, char *argv[])
{
    GraphParser parser;

    srand(time(NULL));
    tmInstance = new TaskMapping("TaskMapping");
    cout << endl
         << "\t\tGraph Parser Testing" << endl;

    parser.Parse("graph.tgff");

    tmInstance->showAllAppsAndTasksInfo();

    return 0;
}