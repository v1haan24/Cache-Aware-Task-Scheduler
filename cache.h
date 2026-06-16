#ifndef CACHE_H
#define CACHE_H
#include <iostream>
#include <string>
#include <deque>
#include <unordered_set>
using namespace std;

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

#endif