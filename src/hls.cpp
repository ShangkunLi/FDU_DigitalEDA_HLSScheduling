#include "hls.h"
#include "schedule.h"
#include <iostream>
#include <queue>
#include <fstream>
using namespace std;

void HLS::generate_CFG()
{
    ofstream fout;
    fout.open("../test/cdfg.txt");
    CFG = ControlFlowGraph(ir_parser);
    fout << "CDFG Begins" << endl;
    fout << endl;
    for (int DFGNode = 0; DFGNode < getCFG().get_DFGNodes().size(); DFGNode++)
    {
        // get each DFG node in CFG
        DataFlowGraph &DFG = this->getCFG().get_DFGNodes()[DFGNode].DFG;
        fout << endl;
        fout << "****************   "
             << "DFG Name: " << DFG.get_label() << "    ******************" << endl;
        fout << endl;
        fout << "Input List of DFG: " << endl;
        for (int i = 0; i < DFG.get_inputList().size(); i++)
        {
            fout << "\t| Source DFG: " + DFG.get_inputList()[i].sourceBlock << endl;
            fout << "\t\t| Input Variabl: " + DFG.get_inputList()[i].inputEdgeName << endl;
        }
        fout << endl;

        // Initialization
        DFG.Initialize();

        int CurrentNode;
        std::queue<int> tq;
        for (int i = 0; i < DFG.get_nodeList().size(); i++)
        {
            if (DFG.Indegree[i] == 0)
            {
                tq.push(i);
            }
        }
        // topological order
        while (!tq.empty())
        {
            CurrentNode = tq.front();
            tq.pop();
            DFG.Mark[CurrentNode] = VISITED;

            fout << "--------------------------------------" << endl;
            fout << endl;
            fout << "Current Node index in DFG: " << CurrentNode << endl;
            fout << endl;
            fout << "Current Node Type: " << DFG.get_nodeList()[CurrentNode].operation.get_strType()<<endl;
            fout << endl;
            fout << "Input Variable of Current Node:" << endl;
            for (int i = 0; i < DFG.get_nodeList()[CurrentNode].inputVar.size(); i++)
                fout << "\t| Variable: " << DFG.get_nodeList()[CurrentNode].inputVar[i] << endl;
            fout << endl;
            fout << "Output Node Index of Current Node: " << endl;
            for (int i = 0; i < DFG.get_desNode(CurrentNode).size(); i++)
                fout << "\t| Node Index: " << DFG.get_desNode(CurrentNode)[i] << endl;
            fout << endl;
            for (int i = 0; i < DFG.get_desNode(CurrentNode).size(); i++)
            {
                // 得到第i个下一节点，如果这个节点这某个输入变量依赖于当前节点
                int nextNodeIndex = DFG.get_desNode(CurrentNode)[i];
                Node &NextNode = DFG.get_nodeList()[nextNodeIndex];
                // 根据第i个节点的输入变量情况，减少入度
                DFG.Indegree[nextNodeIndex]--;
                // 入度小于等于0则进入队列
                if (DFG.Indegree[DFG.get_desNode(CurrentNode)[i]] == 0)
                {
                    tq.push(DFG.get_desNode(CurrentNode)[i]);
                }
            }
        }
        fout << endl;
        fout << "--------------------------------------" << endl;
        fout << "Output List of DFG:" << endl;
        for (int i = 0; i < DFG.get_outputList().size(); i++)
        {
            fout << "\t| Destination DFG: " + DFG.get_outputList()[i].desBlock << endl;
            fout << "\t\t| Output Variable: " + DFG.get_outputList()[i].outputEdgeName << endl;
        }
        fout << endl;
        fout << "--------------------------------------" << endl;
        fout << "Branch of DFG:" << endl;
        for (int i = 0; i < DFG.get_branches().size(); i++)
        {
            fout << "\t| With Condition: " << DFG.get_branches()[i].cond << endl;
            fout << "\t\t| Branch: " + DFG.get_branches()[i].sourceBlock + "->" + DFG.get_branches()[i].desBlock << endl;
        }
        fout << endl;
        fout << "--------------------------------------" << endl;
    }
    fout << endl;
    fout << flush;
    fout.close();
    std::cout << endl;
    std::cout << "Finish CDFG Generation" << endl;
    std::cout << endl;
}

void HLS::scheduling()
{
    Schedule_for_CFG(CFG);
    ofstream fout;
    fout.open("../test/scheduled_CDFG.txt");
    fout << "Schduled CDFG Begins" << endl;
    fout << endl;
    for (int DFGNode = 0; DFGNode < getCFG().get_DFGNodes().size(); DFGNode++)
    {
        // get each DFG node in CFG
        DataFlowGraph &DFG = this->getCFG().get_DFGNodes()[DFGNode].DFG;
        fout << endl;
        fout << "****************   "
             << "DFG Name: " << DFG.get_label() << "    ******************" << endl;
        fout << endl;
        fout << "Input List of DFG: " << endl;
        for (int i = 0; i < DFG.get_inputList().size(); i++)
        {
            fout << "\t| Source DFG: " + DFG.get_inputList()[i].sourceBlock << endl;
            fout << "\t\t| Input Variabl: " + DFG.get_inputList()[i].inputEdgeName << endl;
        }
        fout << endl;

        // Initialization
        DFG.Initialize();

        int CurrentNode;
        std::queue<int> tq;
        for (int i = 0; i < DFG.get_nodeList().size(); i++)
        {
            if (DFG.Indegree[i] == 0)
            {
                tq.push(i);
            }
        }
        // topological order
        while (!tq.empty())
        {
            CurrentNode = tq.front();
            tq.pop();
            DFG.Mark[CurrentNode] = VISITED;

            fout << "--------------------------------------" << endl;
            fout << endl;
            fout << "Current Node index in DFG: " << CurrentNode << endl;
            fout << endl;
            fout << "Current Node Type: " << DFG.get_nodeList()[CurrentNode].operation.get_strType()<<endl;
            fout << endl;
            fout << "Scheduling Results:"<<endl;
            fout << "\t| Start Time: "<<DFG.get_nodeList()[CurrentNode].get_Tstart()<<endl;
            fout << "\t| End Time: "<<DFG.get_nodeList()[CurrentNode].get_Tend()<<endl;
            fout << endl;
            fout << "Input Variable of Current Node:" << endl;
            for (int i = 0; i < DFG.get_nodeList()[CurrentNode].inputVar.size(); i++)
                fout << "\t| Variable: " << DFG.get_nodeList()[CurrentNode].inputVar[i] << endl;
            fout << endl;
            fout << "Output Node Index of Current Node: " << endl;
            for (int i = 0; i < DFG.get_desNode(CurrentNode).size(); i++)
                fout << "\t| Node Index: " << DFG.get_desNode(CurrentNode)[i] << endl;
            fout << endl;
            for (int i = 0; i < DFG.get_desNode(CurrentNode).size(); i++)
            {
                // 得到第i个下一节点，如果这个节点这某个输入变量依赖于当前节点
                int nextNodeIndex = DFG.get_desNode(CurrentNode)[i];
                Node &NextNode = DFG.get_nodeList()[nextNodeIndex];
                // 根据第i个节点的输入变量情况，减少入度
                DFG.Indegree[nextNodeIndex]--;
                // 入度小于等于0则进入队列
                if (DFG.Indegree[DFG.get_desNode(CurrentNode)[i]] == 0)
                {
                    tq.push(DFG.get_desNode(CurrentNode)[i]);
                }
            }
        }
        fout << endl;
        fout << "--------------------------------------" << endl;
        fout << "Output List of DFG:" << endl;
        for (int i = 0; i < DFG.get_outputList().size(); i++)
        {
            fout << "\t| Destination DFG: " + DFG.get_outputList()[i].desBlock << endl;
            fout << "\t\t| Output Variable: " + DFG.get_outputList()[i].outputEdgeName << endl;
        }
        fout << endl;
        fout << "--------------------------------------" << endl;
        fout << "Branch of DFG:" << endl;
        for (int i = 0; i < DFG.get_branches().size(); i++)
        {
            fout << "\t| With Condition: " << DFG.get_branches()[i].cond << endl;
            fout << "\t\t| Branch: " + DFG.get_branches()[i].sourceBlock + "->" + DFG.get_branches()[i].desBlock << endl;
        }
        fout << endl;
        fout << "--------------------------------------" << endl;
    }
    fout << endl;
    fout << flush;
    fout.close();
    std::cout << "Finish Scheduling" << endl;
    std::cout << endl;
}