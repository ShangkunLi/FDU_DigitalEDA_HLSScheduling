#ifndef HLS_H
#define HLS_H
#include <iostream>
#include "parser.h"
#include "dataflow.h"
#include "controlflow.h"

using namespace std;

class HLS
{
private:
    // Parser for the IR
    parser ir_parser;
    // Reg
    vector<vector<pair<string, int>>> REG; 
    // Control Flow Graph
    ControlFlowGraph CFG;


public:
    HLS(parser &p): ir_parser(p) {}

    ControlFlowGraph &getCFG(){
        return CFG;
    }

    // generate the control flow graph
    void generate_CFG();

    // perform scheduling
    void scheduling();
};

#endif