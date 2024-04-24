#include "dataflow.h"

int op::set_T(int type, string& strType)
{
    int ret = 0;
    switch (type)
    {
    case OP_ASSIGN:
        ret = T_ASSIGN;
        strType = "Assign";
        break;
    case OP_SUB:
        ret = T_SUB;
        strType = "Sub";
        break;
    case OP_ADD:
        ret = T_ADD;
        strType = "Add";
        break;
    case OP_MUL:
        ret = T_MUL;
        strType = "Mul";
        break;
    case OP_DIV:
        ret = T_DIV;
        strType = "Div";
        break;
    case OP_LOAD:
        ret = T_LOAD;
        strType = "Load";
        break;
    case OP_STORE:
        ret = T_STORE;
        strType = "Store";
        break;
    case OP_BR:
        ret = T_BR;
        strType = "Br";
        break;
    case OP_LT:
        ret = T_LT;
        strType = "LT";
        break;
    case OP_LE:
        ret = T_LE;
        strType = "LE";
        break;
    case OP_GT:
        ret = T_GT;
        strType = "GT";
        break;
    case OP_GE:
        ret = T_GE;
        strType = "GE";
        break;
    case OP_EQ:
        ret = T_EQ;
        strType = "EQ";
        break;
    case OP_PHI:
        ret = T_PHI;
        strType = "Phi";
        break;
    case OP_RET:
        ret = T_RET;
        strType = "Ret";
        break;
    default:
        ret = 0;
        break;
    }
    return ret;
}

void DataFlowGraph::CreateEdge(int source, int des)
{
    if (source < nodeList.size())
    {
        nodeList[source].next.push_back(des);
    }
}

bool DataFlowGraph::IsPureNumber(string var)
{
    bool isPureNumber = true;
    for (char c : var)
    {
        if (!isdigit(c))
        {
            isPureNumber = false;
            break;
        }
    }
    return isPureNumber;
}

void DataFlowGraph::CreateEdge(string inputVar, int des)
{
    CreateEdge(varTable[inputVar], des);
}

void DataFlowGraph::CreateEdge(Node &CurNode, int i, int des)
{
    string inputVar = CurNode.operation.get_inputVars()[i];
    bool isPureNumber = IsPureNumber(inputVar);
    if (isPureNumber)
    {
        return;
    }
    bool AlreadyIn = (varTable.find(inputVar) != varTable.end());
    if (AlreadyIn && varTable[inputVar] != 0)
    {
        CreateEdge(varTable[inputVar], des);
    }
    else
    {
        if (!AlreadyIn)
        {
            inputList.push_back(InputEdge(inputVar));
            varTable[inputVar] = 0;
        }
        CreateEdge(0, des);
    }
}

void DataFlowGraph::CreateEdges(Node &CurNode, int des)
{
    for (int i = 0; i < CurNode.operation.get_inputVars().size(); i++)
    {
        CreateEdge(CurNode, i, des);
    }
}

DataFlowGraph::DataFlowGraph(basic_block &bb)
{
    label = bb.get_label_name();
    int node_num = bb.get_statements().size();
    // add a null node
    nodeList.push_back(op());

    for (int i = 0; i < node_num; i++)
    {
        int currentNode = i + 1;
        Node CurNode(op(bb.get_statements()[i]));

        // add the output variables of current node into the variable table
        if (!CurNode.operation.get_outputVar().empty())
        {
            varTable[CurNode.operation.get_outputVar()] = currentNode;
        }

        // Branch
        if (CurNode.operation.get_opType() == OP_BR)
        {
            if (CurNode.operation.get_inputVars().size() == 1)
            {
                branches.push_back(BranchEdge(label, CurNode.operation.get_inputVars()[0], Unconditional));
            }
            else if (CurNode.operation.get_inputVars().size() == 3)
            {
                CurNode.inputVar.push_back(CurNode.operation.get_inputVars()[0]);
                CreateEdge(CurNode, 0, currentNode);
                branches.push_back(BranchEdge(label, CurNode.operation.get_inputVars()[1], IfTrue));
                branches.push_back(BranchEdge(label, CurNode.operation.get_inputVars()[2], IfFalse));
            }
        }

        // Return
        else if (CurNode.operation.get_opType() == OP_RET)
        {
            if (!CurNode.operation.get_inputVars().empty())
            {
                CurNode.inputVar.push_back(CurNode.operation.get_inputVars()[0]);
                CreateEdge(CurNode, 0, currentNode);
            }
            BranchEdge returnEdge(label, CurNode.operation.get_inputVars()[0], Unconditional);
            returnEdge.isRet = true;
            branches.push_back(returnEdge);
        }

        // Phi
        else if (CurNode.operation.get_opType() == OP_PHI)
        {
            for (int k = 0; k < CurNode.operation.get_inputVars().size(); k++)
            {
                if (k % 2 == 0)
                {
                    CurNode.inputVar.push_back(CurNode.operation.get_inputVars()[k]);
                    CreateEdge(CurNode, k, currentNode);
                }
            }
        }

        // Load
        else if (CurNode.operation.get_opType() == OP_LOAD)
        {
            // load operation need to interact with memory
            memory.push_back(Memaccess(label, currentNode, CurNode.operation.get_inputVars()[0], CurNode.operation.get_inputVars()[1]));
            for (int i = 0; i < CurNode.operation.get_inputVars().size(); i++)
                CurNode.inputVar.push_back(CurNode.operation.get_inputVars()[i]);
            CreateEdges(CurNode, currentNode);
        }
        else
        {
            // creat regular edges
            for (int i = 0; i < CurNode.operation.get_inputVars().size(); i++)
                CurNode.inputVar.push_back(CurNode.operation.get_inputVars()[i]);
            CreateEdges(CurNode, currentNode);
        }

        nodeList.push_back(CurNode);
    }
}

void DataFlowGraph::Initialize()
{
    Indegree = vector<int>(get_nodeList().size());
    Outdegree = vector<int>(get_nodeList().size());
    Mark = vector<int>(get_nodeList().size(), UNVISITED);
    for (int i = 0; i < get_nodeList().size(); i++)
    {
        if (get_nodeList()[i].operation.get_opType() != OP_PHI)
        {
            for (auto Var : get_nodeList()[i].inputVar)
            {
                if (IsPureNumber(Var))
                    continue;
                Indegree[i]++;
            }
        }
        else
        {
            Indegree[i] = 1;
        }
        Outdegree[i] = get_desNode(i).size();
    }
}
