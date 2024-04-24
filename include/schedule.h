#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <algorithm>
#include <queue>
#include <map>
#include "dataflow.h"
#include "controlflow.h"

using namespace std;

// use this class to manage the hardware resource
class HardwareRes
{
private:
    int adder;
    int mul;
    int div;
    int sram;
    int adder_aval; // available adder
    int mul_aval;   // available mul
    int div_aval;   // available div
    int sram_aval;  // available sram
public:
    HardwareRes(int _adder = 2, int _mul = 1, int _div = 1, int _sram = 1)
    { // by default, we consider we only have 2 adders, 1 mul, 1 div, and 1 sram
        adder = _adder;
        mul = _mul;
        div = _div;
        sram = _sram;
        adder_aval = adder;
        mul_aval = mul;
        div_aval = div;
        sram_aval = sram;
    }

    int get_adder_num()
    {
        return adder;
    }

    int get_mul_num()
    {
        return mul;
    }

    int get_div_num()
    {
        return div;
    }

    int get_sram_num()
    {
        return sram;
    }

    int get_adder_aval()
    {
        return adder_aval;
    }

    int get_mul_aval()
    {
        return mul_aval;
    }

    int get_div_aval()
    {
        return div_aval;
    }

    int get_sram_aval()
    {
        return sram_aval;
    }

    void set_adder_aval(int adder_curr)
    {
        adder_aval = adder_curr;
    }

    void set_div_aval(int div_curr)
    {
        div_aval = div_curr;
    }

    void set_mul_aval(int mul_curr)
    {
        mul_aval = mul_curr;
    }

    void set_sram_aval(int sram_curr)
    {
        sram_aval = sram_curr;
    }
};

class InitSchedule
{ // store the result of initial schduling (ASAP & ALAP)
private:
    vector<pair<int, int>> ASAP_RESULT;
    vector<pair<int, int>> ALAP_RESULT;

public:
    InitSchedule(DataFlowGraph &DFG)
    {
        for (int k = 0; k < DFG.get_nodeList().size(); k++)
        {
            ASAP_RESULT.push_back(std::make_pair(0, 0));
            ALAP_RESULT.push_back(std::make_pair(0, 0));
        }
    }

    vector<pair<int, int>> &get_ASAP()
    {
        return ASAP_RESULT;
    }
    vector<pair<int, int>> &get_ALAP()
    {
        return ALAP_RESULT;
    }
};

// Check whther the operation satisfy the resource constraints
bool meet_resources_constraint(HardwareRes &res, int i, DataFlowGraph &DFG);

// release the resource configured
void reset(HardwareRes &hardware, int i, DataFlowGraph &DFG);

// get the max number between a and b
int max(int a, int b);

// get the min number between a and b
int min(int a, int b);

// perform the ASAP algorithm for DFG, store the result in InitResult
void ASAP(DataFlowGraph &DFG, InitSchedule &InitResult);

// perform the ALAP algorithm for DFG, store the result in InitResult
void ALAP(DataFlowGraph &DFG, InitSchedule &InitResult);

// if a.second < b.second, return true
bool cmp(const pair<int, int> &a, const pair<int, int> &b);

// Perfrom the list scheduling for DFG
void ListSchedule_for_DFG(DataFlowGraph &DFG);

// Perform scheduling for each DFG in CFG
void Schedule_for_CFG(ControlFlowGraph &CFG);
#endif