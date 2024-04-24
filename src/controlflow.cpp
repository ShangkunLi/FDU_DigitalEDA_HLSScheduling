#include "controlflow.h"

int ControlFlowGraph::get_Index(string label)
{
    if (IndexDFG.find(label) != IndexDFG.end())
    {
        return IndexDFG[label];
    }
    else
        return 0;
}

ControlFlowGraph::ControlFlowGraph(parser &p)
{
    // function name
    func_name = p.get_function_name();
    // function return type
    ret_type = p.get_ret_type();
    // function parameters
    vars = p.get_function_params();

    // use a DFG to store the parameters
    DataFlowGraph dfg;
    for (int i = 0; i < vars.size(); i++)
    {
        IndexDFG[dfg.get_label()] = 0;
        dfg.get_varTable()[vars[i]._name] = 1;
    }
    dfg.get_branches()[0].desBlock = p.get_basic_blocks()[0].get_label_name();
    DFGs.push_back(graph_node(dfg));

    // generate CFG from DFGs
    for (int i = 0; i < p.get_basic_blocks().size(); i++)
    {
        int currentBlock = i + 1;
        basic_block &bb = p.get_basic_blocks()[i];
        DataFlowGraph DFG(bb);
        IndexDFG[DFG.get_label()] = currentBlock;
        if (DFG.get_branches().size() == 1 && DFG.get_branches()[0].desBlock.empty())
        {
            std::string nextLabel = p.get_basic_blocks()[i + 1].get_label_name();
            DFG.get_branches()[0].sourceBlock = DFG.get_label();
            DFG.get_branches()[0].desBlock = nextLabel;
        }
        DFGs.push_back(graph_node(DFG));
    }

    // generate the input list from node list in DFG
    int MemIndex = 0;
    for (int i = 0; i < DFGs.size(); i++)
    {
        DataFlowGraph &CurDFG = DFGs[i].DFG;
        for (int j = 0; j < CurDFG.get_inputList().size(); j++)
        {
            if (CurDFG.get_inputList()[j].sourceBlock.empty())
            {
                string inputEdgeName = CurDFG.get_inputList()[j].inputEdgeName;
                DataFlowGraph *desBlock = findSourceVar(inputEdgeName);
                desBlock->get_outputList().push_back(OutputEdge(CurDFG.get_label(), inputEdgeName));
                CurDFG.get_inputList()[j].sourceBlock = desBlock->get_label();
                if (desBlock->get_label() != "fiction_head")
                {
                    MemMap[inputEdgeName] = MemIndex++;
                    Data.push_back(inputEdgeName);
                }
            }
        }
    }

    for (int i = 0; i < DFGs.size(); i++)
    {
        DataFlowGraph &CurDFG = DFGs[i].DFG;
        int desIndex = CurDFG.get_nodeList().size() - 1;
        for (int j = 0; j < CurDFG.get_outputList().size(); j++)
        {
            std::string outputEdgeName = CurDFG.get_outputList()[j].outputEdgeName;
            int sourceIndex = CurDFG.get_varTable()[outputEdgeName];
            // 在from和to之间创建一个边
            CurDFG.CreateEdge(sourceIndex, desIndex);
            CurDFG.get_nodeList()[desIndex].inputVar.push_back(outputEdgeName);
        }
    }
}


vector<int> ControlFlowGraph::NextNode(string label){
    int index = IndexDFG[label];
    return NextNode(index);
}

vector<int> ControlFlowGraph::NextNode(int index){
    vector<int> nn;
    for (int i = 0; i < DFGs[index].controlEdges.size(); i++)
    {
        nn.push_back(IndexDFG[DFGs[index].controlEdges[i].desBlock]);
    }
    return nn;
}

DataFlowGraph *ControlFlowGraph::findSourceVar(std::string VarName)
{
    for (int i = 0; i < DFGs.size(); i++)
    {
        // return the label if found
        std::unordered_map<std::string, int> &map = DFGs[i].DFG.get_varTable();
        if (map.find(VarName) != map.end() && map[VarName] != 0)
        {
            return &DFGs[i].DFG;
        }
    }
    return NULL;
}
