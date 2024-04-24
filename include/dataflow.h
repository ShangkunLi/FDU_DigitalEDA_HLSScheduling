#ifndef DATAFLOWGRAPH_H
#define DATAFLOWGRAPH_H

#include "parser.h"
#include "unordered_map"

using namespace std;

// Define the number of cycles each
constexpr auto T_ASSIGN = 1; // assignment
constexpr auto T_ADD = 2;    // add
constexpr auto T_SUB = 2;    // subtraction
constexpr auto T_MUL = 5;    // multiplication
constexpr auto T_DIV = 40;   // division
constexpr auto T_LOAD = 3;   // load
constexpr auto T_STORE = 3;  // store
constexpr auto T_BR = 1;     // branch
constexpr auto T_LT = 2;     // <
constexpr auto T_LE = 2;     // <=
constexpr auto T_GT = 2;     // >
constexpr auto T_GE = 2;     // >=
constexpr auto T_EQ = 2;     // ==
constexpr auto T_PHI = 2;    // phi
constexpr auto T_RET = 1;    // return

enum CONDTYPE
{
    IfFalse,
    IfTrue,
    Unconditional
};

enum ISVISITED
{
    UNVISITED,
    VISITED,
    SCHEDULED
};

// Input edge
struct InputEdge
{
    // The input edge data structure
    string sourceBlock;   // The source block of this input
    string inputEdgeName; // The name of this input
    InputEdge(string &_sourceBlock, string &_inputEdgeName)
    {
        sourceBlock = _sourceBlock;
        inputEdgeName = _inputEdgeName;
    }
    InputEdge(string &_inputEdgeName)
    {
        inputEdgeName = _inputEdgeName;
    }
};

// Output edge
struct OutputEdge
{
    // The output edge data structure
    string desBlock;       // The destination block of this output
    string outputEdgeName; // The name of this output
    OutputEdge(string &_desBlock, string &_outputEdgeName)
    {
        desBlock = _desBlock;
        outputEdgeName = _outputEdgeName;
    }
};

// Branch edge
struct BranchEdge
{
    int cond; // branch condition
    string sourceBlock;
    string desBlock;
    bool isRet;
    // construct function
    BranchEdge(string _sourceBlock, string _desBlock, int _cond)
    {
        cond = _cond;
        isRet = false;
        sourceBlock = _sourceBlock;
        desBlock = _desBlock;
    }
    BranchEdge()
    {
        cond = 2;
        isRet = false;
    }
};

// Memory Access
struct Memaccess
{
    string sourceLabel;
    int sourceopIndex;
    string arrayName;
    string bias;
    // construct function
    Memaccess(string _sourceLabel, int _sourceopIndex, string _arrayName, string _bias) : sourceLabel(_sourceLabel), sourceopIndex(_sourceopIndex), arrayName(_arrayName), bias(_bias) {}
};

// Operation class
class op
{
private:
    // operation variables
    vector<string> inputVars;
    // output variable
    string outputVar;
    // operation type
    int opType;
    // operation cycles
    int T;
    string strType;
    int set_T(int type, string &strType);

public:
    vector<string> &get_inputVars()
    {
        return inputVars;
    }
    string &get_outputVar()
    {
        return outputVar;
    }
    int get_opType() const
    {
        return opType;
    }
    int get_latency() const
    {
        return T;
    }
    string get_strType() const{
        return strType;
    }

    // construc function
    op(statement &_statement)
    {
        T = set_T(_statement.get_type(), strType);
        opType = _statement.get_type();
        for (int k = 0; k < _statement.get_num_oprands(); ++k)
        {
            inputVars.push_back(_statement.get_oprand(k));
        }
        outputVar = _statement.get_var();
    }
    op() : T(0){};
};

// Node in this block
class Node
{
public:
    // Operation in this node
    op operation;

    // The input variable of this node
    vector<string> inputVar;
    // The next node of this node
    vector<int> next;

    // Strat time
    int T_start;
    // End time
    int T_end;

    // construct function
    Node(const op &_operation, vector<int> &_next)
    {
        operation = _operation;
        next = _next;
        T_start = 0;
        T_end = 0;
    }
    Node(const op &_operation)
    {
        operation = _operation;
        T_start = 0;
        T_end = 0;
    }
    Node()
    {
        T_start = 0;
        T_end = 0;
    }

    // get the start time
    int get_Tstart() const
    {
        return T_start;
    }
    // get the end time
    int get_Tend() const
    {
        return T_end;
    }

    // set the start time
    void set_Tstart(int currentT)
    {
        T_start = currentT;
    }
    // set the end time
    void set_Tend(int currentT)
    {
        T_end = currentT;
    }
};

// Data flow graph
class DataFlowGraph
{
private:
    // name of this module
    string label;

    // input element list
    vector<InputEdge> inputList;

    // output element list
    vector<OutputEdge> outputList;

    // operation node list
    vector<Node> nodeList;

    // branch edge list
    vector<BranchEdge> branches;

    // build a hash table for data accessing
    unordered_map<string, int> varTable;

    // memory accessing
    vector<Memaccess> memory;

    int period = 0;

public:
    // mark if the node is visited or not
    vector<int> Mark;
    // In-degree of the node
    vector<int> Indegree;
    // Out-degree of the node
    vector<int> Outdegree;

    DataFlowGraph(basic_block &bb);

    DataFlowGraph()
    {
        nodeList.push_back(op());
        BranchEdge out;
        label = "fiction_head";
        out.sourceBlock = label;
        branches.push_back(out);
        nodeList.push_back(op());
    }

    // Create edge & operation
    void CreateEdge(int source, int des);
    bool IsPureNumber(string var);
    void CreateEdge(string inputVar, int des);
    void CreateEdge(Node &CurNode, int i, int des);
    void CreateEdges(Node &CurNode, int des);

    // Initialization
    void Initialize();

    // Get the values in DFG
    // get node list
    vector<Node> &get_nodeList()
    {
        return nodeList;
    }
    // get branch edges
    vector<BranchEdge> &get_branches()
    {
        return branches;
    }
    // get input edges
    vector<InputEdge> &get_inputList()
    {
        return inputList;
    }
    // get output edges
    vector<OutputEdge> &get_outputList()
    {
        return outputList;
    }
    // get the name of this module
    string &get_label()
    {
        return label;
    }
    // get the next node of current node
    vector<int> &get_desNode(int source)
    {
        return nodeList[source].next;
    }
    // get the hash table
    unordered_map<string, int> &get_varTable()
    {
        return varTable;
    }
    // get the period of this graph
    int get_Period() const
    {
        return period;
    }
    // change the period
    void set_Period(int T)
    {
        period = T;
    }
};

#endif