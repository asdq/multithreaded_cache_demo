#include "../../include/graph.h"
#include <iostream>
#include <sstream>
#include <tuple>
#include <stdexcept>

using namespace std;

static
void test_raw_graph()
{
    typedef tuple<char, char, float> edge_t;

    vector<char> nodes = { 'A', 'B', 'C', 'D', 'E', 'F', 'G' };
    unordered_map<char, vector<char>> neighbor_map;
    vector<edge_t> edges;
    
    edges.emplace_back('A', 'B', 0.f);
    edges.emplace_back('A', 'C', 0.f);
    edges.emplace_back('B', 'C', 0.f);
    edges.emplace_back('B', 'D', 0.f);
    edges.emplace_back('D', 'E', 0.f);
    edges.emplace_back('D', 'F', 0.f);
    edges.emplace_back('D', 'G', 0.f);
    edges.emplace_back('E', 'F', 0.f);
    edges.emplace_back('F', 'G', 0.f);
    
    cout << "Graph:\n";
    for (auto node : nodes) {
        vector<char> neighbors;
        
        for (auto &e : edges) {
            if (node == get<0>(e) && node != get<1>(e)) {
                neighbors.push_back(get<1>(e));
            } else if (node == get<1>(e) && node != get<0>(e)) {
                neighbors.push_back(get<0>(e));
            }
        }
        
        cout << node << " -> ";
        for (auto c : neighbors) {
            cout << c << ' ';
        }
        cout << '\n';
        
        neighbor_map[node] = neighbors;
    }
    cout << endl;
    
    futil::betweenness<char, float>(nodes.begin(), nodes.end(),
    
        [&neighbor_map] (char n) {
            auto i = neighbor_map.find(n);
            
            if (i != neighbor_map.end()) {
                return make_pair(i -> second.begin(), i -> second.end());
            }
            throw range_error("Node not found.");
        },
        
        [&edges] (char a, char b) -> float& {
            for (auto &e : edges) {
                if ((a == get<0>(e) && b == get<1>(e)) ||
                    (a == get<1>(e) && b == get<0>(e))) {
                    return get<2>(e);
                }
            }
            throw range_error("Edge not found.");
        });
    
    sort(edges.begin(), edges.end(),
        [] (const edge_t &a, const edge_t &b) {
            return get<2>(a) > get<2>(b);
        });
    
    for (auto &e : edges) {
        cout << '(' << get<0>(e) << ", " << get<1>(e)
             << ", " << get<2>(e) << ')' << endl;
    }
}

int main()
{
    test_raw_graph();
}