#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <iostream>
#include <string>
#include <vector>
#include "cache.h"
using namespace std;

struct task{
    string id;
    int burst;
    int remaining;
    vector<string> mem;
    int memindex=0;
    bool completed=false;
    int waitcycles=0;
};

class cacheschedulersimulator{
public:
    int picknexttask(vector<task>& tasks,const cachelevel& l1,const cachelevel& l2,const cachelevel& l3){
        int best=-1;
        int bestscore=-1e9;
        for(int i=0;i<(int)tasks.size();i++){
            if(tasks[i].completed) continue;
            int cachescore=0;
            for(const auto& block : tasks[i].mem){
                if(l1.contains(block)) cachescore+=3;
                else if(l2.contains(block)) cachescore+=2;
                else if(l3.contains(block)) cachescore+=1;
            }
            int finalscore=cachescore+tasks[i].waitcycles/20;
            if(finalscore>bestscore){
                bestscore=finalscore;
                best=i;
            }
            else if(finalscore==bestscore&&bestscore!=-1){
                if(tasks[i].remaining<tasks[best].remaining) best=i;
            }
        }
        if(best!=-1){
            for(int j=0;j<(int)tasks.size();j++){
                if(tasks[j].completed) continue;
                if(j==best) tasks[j].waitcycles=0;
                else tasks[j].waitcycles++;
            }
        }
        return best;
    }

    int accessblock(const string& block,cachelevel& l1,cachelevel& l2,cachelevel& l3,
                    int& ramaccesses,int& l1hits,int& l2hits,int& l3hits){
        if(l1.contains(block)){
            cout<<"    L1: "<<l1.display()<<" -> HIT\n";
            cout<<"    L2: "<<l2.display()<<"\n";
            cout<<"    L3: "<<l3.display()<<"\n";
            l1hits++;
            return l1.latency;
        }
        cout<<"    L1: "<<l1.display()<<" >> MISS\n";
        if(l2.contains(block)){
            cout<<"    L2: "<<l2.display()<<" >> HIT ("<<l2.latency<<" cycles)\n";
            cout<<"    L3: "<<l3.display()<<"\n";
            cout<<"    Promoting "<<block<<" -> L1\n";
            l1.insert(block);
            l2hits++;
            cout<<"    L1: "<<l1.display()<<"\n";
            cout<<"    L2: "<<l2.display()<<"\n";
            return l2.latency;
        }
        cout<<"    L2: "<<l2.display()<<" >> MISS\n";
        if(l3.contains(block)){
            cout<<"    L3: "<<l3.display()<<" >> HIT ("<<l3.latency<<" cycles)\n";
            cout<<"    Promoting "<<block<<" -> L1\n";
            l1.insert(block);
            l3hits++;
            cout<<"    L1: "<<l1.display()<<"\n";
            return l3.latency;
        }
        cout<<"    L3: "<<l3.display()<<" >> MISS\n";
        cout<<"    Fetching "<<block<<" from RAM (200 cycles)\n";
        ramaccesses++;
        l3.insert(block);
        l2.insert(block);
        l1.insert(block);
        cout<<"    L1: "<<l1.display()<<"\n";
        cout<<"    L2: "<<l2.display()<<"\n";
        cout<<"    L3: "<<l3.display()<<"\n";
        return 200;
    }
};

#endif