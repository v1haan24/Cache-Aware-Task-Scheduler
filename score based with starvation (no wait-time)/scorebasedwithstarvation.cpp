//Score Based CPU Scheduler with starvation (backup)
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <unordered_set>
#include <chrono>

using namespace std;

struct task{
    string id;
    int burst;
    int remaining;
    vector<string> mem;
    int memindex=0;
    bool completed=false;
};

class cachelevel{
public:
    string name;
    int capacity;
    int latency;
    deque<string> slots;         
    unordered_set<string> lookup;

    cachelevel(string n,int cap,int lat){
        name=n;
        capacity=cap;
        latency=lat;
    }

    bool contains(const string& block) const{
        return lookup.count(block)>0;
    }

    void insert(const string& block){
        if(lookup.count(block)) return;   
        if((int)slots.size()>=capacity){
            cout<<"    ("<<slots.front()<<" evicted)\n";
            lookup.erase(slots.front());
            slots.pop_front();
        }
        slots.push_back(block);
        lookup.insert(block);
    }

    string display() const{
        string s="[";
        for(int i=0;i<(int)slots.size();i++){
            if(i) s+=", ";
            s+=slots[i];
        }
        return s+"]";
    }
};

class cacheschedulersimulator{
public:
    int picknexttask(const vector<task>& tasks,const cachelevel& l1,const cachelevel& l2,const cachelevel& l3){
        int best=-1;
        int bestscore=-1;
        for(int i=0;i<(int)tasks.size();i++){
            if(tasks[i].completed) continue;
            int score=0;
            for(const auto& block:tasks[i].mem){
                if(l1.contains(block)) score+=3;
                else if(l2.contains(block)) score+=2;
                else if(l3.contains(block)) score+=1;
            }
            if(score>bestscore){ 
                bestscore=score; 
                best=i; 
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
    int l1hits=0,l2hits=0,l3hits=0;

    cout<<"=== CPU Scheduler + Cache Hierarchy Simulator ===\n";
    cout<<"Algorithm: Score Based\n";
    cout<<"Cache policy: Inclusive (block in L1 => also in L2 and L3)\n";
    cout<<"Tasks loaded: "<<total<<"\n\n";

    auto starttime=chrono::high_resolution_clock::now();

    while(completed<total){
        int idx=solver.picknexttask(tasks,l1,l2,l3);
        if(idx==-1) break;
        task& cur=tasks[idx];
        step++;

        cout<<"--------------------------------------\n";
        cout<<"Step "<<step<<" - Running: "<<cur.id<<"  (remaining burst: "<<cur.remaining<<")\n";

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