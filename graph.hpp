#ifndef _GRAPH_H
#define _GRAPH_H

#include <vector>
#include <queue>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct GNode { 
  uint64_t node_id;
  GNode(uint64_t id) {
    node_id = id;
  }
};

struct Graph { 
  unordered_map<uint64_t, unordered_set<uint64_t> > g;

  int addNode(uint64_t node_id) {
    if (g.find(node_id) == g.end()) {
      g[node_id] = unordered_set<uint64_t>();
      return 200;
    }else {
      //The node already exists in the graph
      return 204;
    }
  }

  int addEdge(uint64_t node_id_a, uint64_t node_id_b) {
    if (g.find(node_id_a) == g.end() or g.find(node_id_b) == g.end()
        or node_id_a == node_id_b) {
       return 400;
    }
    if (g[node_id_a].find(node_id_b) != g[node_id_a].end()) {
      //the edge already exist
      return 204;
    }
    //add the edge
    g[node_id_a].insert(node_id_b);
    g[node_id_b].insert(node_id_a);
    return 200;
  }

  int removeNode(uint64_t node_id) {
    if (g.find(node_id) == g.end()) {
      //the node doesn't exist in graph
      return 400;
    }
    //remove the node
    for (auto it = g[node_id].begin(); it != g[node_id].end(); ++it) {
        g[*it].erase(node_id);
    }
    g.erase(node_id);
    return 200;
  }

  int removeEdge(uint64_t node_id_a, uint64_t node_id_b) {
    //edge doesn't exsit
    if (g.find(node_id_a) == g.end() or g[node_id_a].find(node_id_b) == g[node_id_a].end()) {
      return 400;
    }
    //remove edge
    g[node_id_a].erase(node_id_b);
    g[node_id_b].erase(node_id_a);
    return 200;
  }

  pair<int, int> getNode(uint64_t node_id) {
    pair<int, int> res = make_pair(200, 1);
    if (g.find(node_id) == g.end()) {
      res.second = 0;
    }
    return res;
  }

  pair<int, int> getEdge(uint64_t node_id_a, uint64_t node_id_b) {
    pair<int, int> res = make_pair(200, 1);
    if (g.find(node_id_a) == g.end() or g.find(node_id_b) == g.end()) {
       //at least one vertice doesn't exist
       res.first = 400;
       res.second = 0;
       return res;
    }
    if (g[node_id_a].find(node_id_b) == g[node_id_a].end()) {
      //the edge doesn't exist
      res.second = 0;
    }
    return res;
  }
  
  pair<int, vector<uint64_t> > getNeighbors(uint64_t node_id) {
    pair<int, vector<uint64_t> > res = make_pair(200, vector<uint64_t>());
    if (g.find(node_id) == g.end()) {
      res.first = 400;
      return res;
    }
    unordered_set<uint64_t>::iterator iter = g[node_id].begin();
    while (iter != g[node_id].end()) {
      res.second.push_back(*iter);
      iter++;
    }
    return res;
  }

  pair<int, int> shortestPath(uint64_t node_id_a, uint64_t node_id_b) {
    pair<int, int> res;
    if (g.find(node_id_a) == g.end() or g.find(node_id_b) == g.end()) {
      res.first = 400;
      return res;
    }
    queue<pair<uint64_t, int> > q;
    q.push(make_pair(node_id_a, 0));
    int shortest = -1;
    unordered_set<uint64_t> visited;
    visited.insert(node_id_a);
    while (!q.empty()) {
      uint64_t node = q.front().first;
      int dist = q.front().second;
      q.pop();
      if (node == node_id_b) {
        shortest = dist;
        break;
      }
      for (unordered_set<uint64_t>::iterator iter = g[node].begin(); iter != g[node].end(); ++iter) {
        if (visited.find(*iter) == visited.end()) {
          visited.insert(*iter);
          q.push(make_pair(*iter, dist + 1));
        }
      }
    }
    if (shortest == -1) {
      //no path found
      res.first = 204;
      return res;
    }
    res.first = 200;
    res.second = shortest;
    return res;
  }
};


#endif
