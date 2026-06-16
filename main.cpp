#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include "cache.h"
#include "scheduler.h"
using namespace std;

vector<task> parsetasks(const string& filename){
    vector<task> tasks;
    ifstream f(filename);
    if(!f.is_open()){
        cerr<<"Error: cannot open "<<filename<<"\n";
        exit(1);
    }
    string line;
    while(getline(f,line)){
        if(line.empty()) continue;
        istringstream ss(line);
        string tok;
        task t;
        ss>>tok>>t.id>>tok>>t.burst;
        t.remaining=t.burst;
        ss>>tok;
        while(ss>>tok) t.mem.push_back(tok);
        tasks.push_back(t);
    }
    return tasks;
}

int main(int argc,char* argv[]){
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    if(argc<2) return 1;
    vector<task> tasks=parsetasks(argv[1]);
    cachelevel l1("L1",32,4);
    cachelevel l2("L2",128,12);
    cachelevel l3("L3",512,40);
    cacheschedulersimulator solver;
    int step=0;
    int totalcycles=0;
    int ramaccesses=0;
    int completed=0;
    int total=(int)tasks.size();
    int l1hits=0;
    int l2hits=0;
    int l3hits=0;
    cout<<"=== CPU Scheduler + Cache Hierarchy Simulator ===\n";
    cout<<"Algorithm: Score Based Scheduler\n";
    cout<<"Cache policy: Inclusive (block in L1 => also in L2 and L3)\n";
    cout<<"Tasks loaded: "<<total<<"\n\n";
    auto starttime=chrono::high_resolution_clock::now();
    while(completed<total){
        int idx=solver.picknexttask(tasks,l1,l2,l3);
        if(idx==-1) break;
        task& cur=tasks[idx];
        step++;
        cout<<"--------------------------------------\n";
        cout<<"Cycle "<<step<<" - Running: "<<cur.id<<"  (remaining burst: "<<cur.remaining<<")\n";
        const string& block=cur.mem[cur.memindex%(int)cur.mem.size()];
        cout<<"  Requesting: "<<block<<"\n";
        int latency=solver.accessblock(block,l1,l2,l3,ramaccesses,l1hits,l2hits,l3hits);
        totalcycles+=latency;
        cur.memindex++;
        cur.remaining--;
        if(cur.remaining==0){
            cur.completed=true;
            completed++;
            cout<<"  >> "<<cur.id<<" COMPLETED\n";
        }
    }
    auto endtime=chrono::high_resolution_clock::now();
    auto duration=chrono::duration_cast<chrono::milliseconds>(endtime-starttime);
    int totalaccesses=l1hits+l2hits+l3hits+ramaccesses;
    double hitrate=totalaccesses?(100.0*(l1hits+l2hits+l3hits)/totalaccesses):0;
    cout<<"\n==========================================\n";
    cout<<"=== Final Results ===\n";
    cout<<"==========================================\n";
    cout<<"Total Steps:     "<<step<<"\n";
    cout<<"Total Cycles:    "<<totalcycles<<"\n";
    cout<<"Tasks Completed: "<<completed<<"\n";
    cout<<"Scheduler:       Score Based\n";
    cout<<"RAM Accesses:    "<<ramaccesses<<"\n";
    cout<<"L1 Hits:         "<<l1hits<<"\n";
    cout<<"L2 Hits:         "<<l2hits<<"\n";
    cout<<"L3 Hits:         "<<l3hits<<"\n";
    cout<<"Cache Hit Rate:  "<<hitrate<<"%\n";
    cout<<"Execution Time:  "<<duration.count()<<" ms\n";
    cout<<"==========================================\n";
    return 0;
}