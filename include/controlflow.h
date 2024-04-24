#ifndef CONTROLFLOWGRAPH_H
#define CONTROLFLOWGRAPH_H
#include "dataflow.h"

using namespace std;

struct graph_node
{
    DataFlowGraph DFG;
    vector<BranchEdge> controlEdges; // Edges related to control
    graph_node(DataFlowGraph &_DFG)
    {
        DFG = _DFG;
        controlEdges = DFG.get_branches();
    };
};

class ControlFlowGraph
{
private:
    string func_name;
    int ret_type;
    vector<var> vars;
    // map each DFG to a number
    unordered_map<string, int> IndexDFG;
    vector<graph_node> DFGs;
    DataFlowGraph *findSourceVar(string VarName);
    // The regesters
    unordered_map<string, int> MemMap;
    vector<string> Data;

public:
    // construct function of CFG
    ControlFlowGraph(){};
    ControlFlowGraph(parser &p);

    // get the DFG node in CFG
    vector<graph_node> &get_DFGNodes()
    {
        return DFGs;
    }

    // get the index of DFG
    int get_Index(string label);

    // get the next CFG's index
    vector<int> NextNode(std::string label);
    vector<int> NextNode(int index);

    // get the CFG's basic information
    string get_Funcname()
    {
        return func_name;
    }
    vector<var> &get_Var()
    {
        return vars;
    }
    int get_RetType() const
    {
        return ret_type;
    }
    unordered_map<string, int> &MemTable()
    {
        return MemMap;
    }
    vector<string> &get_DataSet()
    {
        return Data;
    }
};

#endif