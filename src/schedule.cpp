#include "schedule.h"
using namespace std;

// consider we only have 2 adder, 1 mul, 1 div, 1 sram, 1 branch
bool meet_resources_constraint(HardwareRes &res, int i, DataFlowGraph &DFG)
{
    switch (DFG.get_nodeList()[i].operation.get_opType())
    {
    case OP_ASSIGN:
    case OP_BR:
        return true;
        break;
    case OP_ADD:
    case OP_SUB:
    case OP_LT:
    case OP_GT:
    case OP_LE:
        if (res.get_adder_aval() < 1) // don't have enough adders
        {
            return false;
        }
        else // have enough adders
        {
            res.set_adder_aval(res.get_adder_aval() - 1);
            return true;
        }
        break;
    case OP_MUL:
        if (res.get_mul_aval() < 1)
        {
            return false;
        }
        else
        {
            res.set_mul_aval(res.get_mul_aval() - 1);
            return true;
        }
        break;
    case OP_DIV:
        if (res.get_div_aval() < 1)
        {
            return false;
        }
        else
        {
            res.set_div_aval(res.get_div_aval() - 1);
            return true;
        }
        break;
    case OP_LOAD:
    case OP_STORE:
        if (res.get_sram_aval() < 1)
        {
            return false;
        }
        else
        {
            res.set_sram_aval(res.get_sram_aval() - 1);
            return true;
        }
        break;
    default:
        return true;
    }
}

// release the resource configured
void reset(HardwareRes &hardware, int i, DataFlowGraph &DFG)
{
    switch (DFG.get_nodeList()[i].operation.get_opType())
    {
    case OP_ADD:
    case OP_SUB:
    case OP_LT:
    case OP_GT:
    case OP_LE:
        hardware.set_adder_aval(hardware.get_adder_aval() + 1);
        break;
    case OP_MUL:
        hardware.set_mul_aval(hardware.get_mul_aval() + 1);
        break;
    case OP_DIV:
        hardware.set_div_aval(hardware.get_div_aval() + 1);
        break;
    case OP_LOAD:
    case OP_STORE:
        hardware.set_sram_aval(hardware.get_sram_aval() + 1);
        break;
    default:;
    }
}

int max(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

// perform the ASAP scheduling and store its results in a InitSchedule class
void ASAP(DataFlowGraph &DFG, InitSchedule &InitResult)
{
    int lambda = 1; // mark the longest path cycles
    DFG.Initialize();
    int CurrentNode;
    queue<int> tq;

    // perform scheduling for the first node
    InitResult.get_ASAP()[0].first = 0;
    InitResult.get_ASAP()[0].second = 0;
    DFG.Mark[0] = VISITED;

    // decrease the indegree of the first node's next nodes
    for (int i = 0; i < DFG.get_desNode(0).size(); i++)
    {
        int nextNode = DFG.get_desNode(0)[i];
        DFG.Indegree[nextNode]--;
    }

    // add the indegree = 0 nodes into the queue
    for (int i = 0; i < DFG.get_nodeList().size(); i++)
    {
        if (DFG.Indegree[i] == 0)
        {
            tq.push(i);
            // the start time of schduling is 1
            InitResult.get_ASAP()[i].first = 1;
            InitResult.get_ASAP()[i].second = DFG.get_nodeList()[i].operation.get_latency();
        }
    }

    // topological order
    while (!tq.empty())
    {
        CurrentNode = tq.front();
        int period = InitResult.get_ASAP()[CurrentNode].first; // get the start time of this node
        tq.pop();
        DFG.Mark[CurrentNode] = VISITED;

        for (int i = 0; i < DFG.get_desNode(CurrentNode).size(); i++)
        {
            int nextNode = DFG.get_desNode(CurrentNode)[i];
            DFG.Indegree[nextNode]--;
            // the start time of the next node should greater than the former one
            InitResult.get_ASAP()[nextNode].first = max(InitResult.get_ASAP()[nextNode].first, InitResult.get_ASAP()[CurrentNode].second + 1);
            InitResult.get_ASAP()[nextNode].second = InitResult.get_ASAP()[nextNode].first + DFG.get_nodeList()[nextNode].operation.get_latency() - 1;
            // add more nodes in the queue
            if (DFG.Indegree[nextNode] <= 0 && DFG.Mark[nextNode] == UNVISITED)
            {
                tq.push(nextNode);
                // add the node in the queue
                // update the longest path cycles
                lambda = max(lambda, InitResult.get_ASAP()[nextNode].second);
            }
        }
    }

    DFG.set_Period(lambda);

    // perform scheduling for the last node
    InitResult.get_ASAP()[DFG.get_nodeList().size() - 1].first = lambda + 1;
    InitResult.get_ALAP()[DFG.get_nodeList().size() - 1].second = lambda + 1;
    DFG.Mark[DFG.get_nodeList().size() - 1] = VISITED;
}

// perform the ASAP scheduling and store its results in a InitSchedule class
void ALAP(DataFlowGraph &DFG, InitSchedule &InitResult)
{
    // get the initial longest path cycles of ALAP scheduling
    int lamda = DFG.get_Period();

    // initialization
    DFG.Initialize();

    int CurrentNode;
    queue<int> tq;

    // perform scheduling for the last node
    InitResult.get_ALAP()[DFG.get_nodeList().size() - 1].first = lamda + 1;
    InitResult.get_ALAP()[DFG.get_nodeList().size() - 1].second = lamda + 1;
    DFG.Mark[DFG.get_nodeList().size() - 1] = VISITED;

    Node &Tail = DFG.get_nodeList()[DFG.get_nodeList().size() - 1];
    for (int i = 0; i < Tail.inputVar.size(); i++)
    {
        int formerNode;
        if (DFG.get_varTable().find(Tail.inputVar[i]) != DFG.get_varTable().end())
        {
            formerNode = DFG.get_varTable()[Tail.inputVar[i]];
        }
        else
            break;
        DFG.Outdegree[formerNode]--;
    }

    for (int i = 0; i < DFG.get_nodeList().size(); i++)
    {
        if (DFG.Outdegree[i] <= 0)
        {
            tq.push(i);
            InitResult.get_ALAP()[i].second = lamda;
            InitResult.get_ALAP()[i].first = lamda - DFG.get_nodeList()[i].operation.get_latency() + 1;
        }
    }

    while (!tq.empty())
    {
        CurrentNode = tq.front();
        int period = DFG.get_nodeList()[CurrentNode].get_Tend();
        tq.pop();
        DFG.Mark[CurrentNode] = VISITED;
        Node &node = DFG.get_nodeList()[CurrentNode];
        for (int i = 0; i < node.inputVar.size(); i++)
        {
            int formerNode;
            if (DFG.get_varTable().find(node.inputVar[i]) != DFG.get_varTable().end())
            {
                formerNode = DFG.get_varTable()[node.inputVar[i]];
            }
            else
                break;

            DFG.Outdegree[formerNode]--;
            InitResult.get_ALAP()[formerNode].second = min(InitResult.get_ALAP()[formerNode].second, InitResult.get_ALAP()[CurrentNode].first - 1);
            InitResult.get_ALAP()[formerNode].first = InitResult.get_ALAP()[formerNode].second - DFG.get_nodeList()[formerNode].operation.get_latency() + 1;

            if (DFG.Outdegree[formerNode] <= 0 && DFG.Mark[formerNode] == UNVISITED)
            {
                tq.push(formerNode);
            }
        }
    }

    // perform scheduling for the first node
    InitResult.get_ASAP()[0].first = 0;
    InitResult.get_ASAP()[0].second = 0;
    DFG.Mark[0] = VISITED;
}

// if a.second < b.second, return true
bool cmp(const pair<int, int> &a, const pair<int, int> &b)
{
    return a.second < b.second;
}

// Perfrom the list scheduling for DFG
void ListSchedule_for_DFG(DataFlowGraph &DFG)
{
    InitSchedule InitResult(DFG);
    // perform ASAP and ALAP scheduling
    ASAP(DFG, InitResult);
    ALAP(DFG, InitResult);

    HardwareRes hardware;

    vector<int> Exe_Time_L;
    Exe_Time_L.push_back(0);
    for (int i = 1; i < DFG.get_nodeList().size() - 1; i++)
    {
        Exe_Time_L.push_back(DFG.get_nodeList()[i].operation.get_latency());
    }
    Exe_Time_L.push_back(0);

    // set the start time of scheduling to 1
    int current_period = 1;

    DFG.Initialize();
    // add the indegree = 0 nodes
    int CurrentNode;
    map<int, int> tq_m; // the first item stores the index of node, the second item stores the difference between ASAP & ALAP scheduling

    // perform schduling for the first node
    DFG.get_nodeList()[0].set_Tstart(0);
    DFG.get_nodeList()[0].set_Tend(0);
    DFG.Mark[0] = VISITED;
    // decrease the indegree of next nodes
    for (int i = 0; i < DFG.get_desNode(0).size(); i++)
    {
        int nextNode = DFG.get_desNode(0)[i];
        DFG.Indegree[nextNode]--;
    }

    // add node with 0 indegree to the queue
    for (int i = 1; i < DFG.get_nodeList().size(); i++)
    {
        if (DFG.Indegree[i] <= 0)
        {
            tq_m.insert(std::pair<int, int>(i, InitResult.get_ALAP()[i].first - InitResult.get_ASAP()[i].first));
            DFG.Mark[i] = VISITED;
        }
    }

    int lambda = DFG.get_Period();

    // topological order
    while (!tq_m.empty())
    {
        // each iteration of a while loop, stands for a scheduling for a cycle

        // sorted the item in queue by the difference between ASAP & ALAP result
        vector<pair<int, int>> tq(tq_m.begin(), tq_m.end());
        sort(tq.begin(), tq.end(), cmp);

        // perform schduling
        for (auto itera = tq.begin(); itera != tq.end(); itera++)
        {
            if (DFG.Mark[itera->first] == SCHEDULED)
            {
            }
            else
            {
                if (meet_resources_constraint(hardware, itera->first, DFG))
                {
                    DFG.get_nodeList()[itera->first].T_start = current_period;
                    DFG.get_nodeList()[itera->first].T_end = current_period + DFG.get_nodeList()[itera->first].operation.get_latency() - 1;
                    lambda = max(lambda, DFG.get_nodeList()[itera->first].T_end);
                    DFG.Mark[itera->first] = SCHEDULED;
                }
            }
        }

        // find the visited node, and decrease the next nodes' indrgree
        for (auto iter = tq.begin(); iter != tq.end(); iter++)
        {
            int index1 = iter->first;
            if (DFG.Mark[index1] == SCHEDULED)
            {
                Exe_Time_L[index1]--;
            }

            if (DFG.Mark[index1] == SCHEDULED && Exe_Time_L[index1] <= 0)
            {
                // decrease the indegree of the next nodes'
                for (int i = 0; i < DFG.get_desNode(index1).size(); i++)
                {
                    int nextNode = DFG.get_desNode(index1)[i];
                    Node &NextNode = DFG.get_nodeList()[nextNode];
                    DFG.Indegree[nextNode]--;

                    // add node with 0 indegree to the queue
                    if (DFG.Indegree[nextNode] <= 0 && DFG.Mark[nextNode] == UNVISITED)
                    {
                        tq_m.insert(std::pair<int, int>(nextNode, InitResult.get_ALAP()[nextNode].first - InitResult.get_ASAP()[nextNode].first));
                    }
                }
                // update the resource condition
                reset(hardware, index1, DFG);

                tq_m.erase(index1);
            }
        }
        current_period++;
    }

    DFG.get_nodeList()[DFG.get_nodeList().size() - 1].set_Tstart(lambda + 1);
    DFG.get_nodeList()[DFG.get_nodeList().size() - 1].set_Tend(lambda + 1);
    DFG.Mark[DFG.get_nodeList().size() - 1] = VISITED;
    DFG.set_Period(lambda);
}

// Perform scheduling for each DFG in CFG
void Schedule_for_CFG(ControlFlowGraph &CFG)
{
    for (int DFGNode = 1; DFGNode < CFG.get_DFGNodes().size(); DFGNode++)
    {
        // get each DFG node in CFG
        DataFlowGraph &DFG = CFG.get_DFGNodes()[DFGNode].DFG;
        // perform scheduling for this DFG
        ListSchedule_for_DFG(DFG);
    }

    DataFlowGraph &DFG = CFG.get_DFGNodes()[0].DFG;
    DFG.set_Period(0);
    DFG.get_nodeList()[DFG.get_nodeList().size() - 1].set_Tstart(1);
    DFG.get_nodeList()[DFG.get_nodeList().size() - 1].set_Tend(1);
}