#ifndef _GRAPH_H
#define _GRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

struct GNode {
    uint64_t node_id;
    GNode(uint64_t id) {
        node_id = id;
    }
};

struct Graph {
    
    unordered_map<uint64_t, unordered_set<uint64_t> > g;
    
    int addNode(uint64_t node_id);
    
    int addEdge(uint64_t node_id_a, uint64_t node_id_b);
    
    int removeNode(uint64_t node_id);
    
    int removeEdge(uint64_t node_id_a, uint64_t node_id_b);
    
    pair<int, int> getNode(uint64_t node_id);
    
    pair<int, int> getEdge(uint64_t node_id_a, uint64_t node_id_b);
    
    pair<int, vector<uint64_t> > getNeighbors(uint64_t node_id);
    
    pair<int, int> shortestPath(uint64_t node_id_a, uint64_t node_id_b);
};


#endif
