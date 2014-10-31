#ifndef GRAPH_H
#define GRAPH_H

/*
The MIT License (MIT)

Copyright (c) 2014 Fabio Vaccari

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// DEBUG
#include <iostream>

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>

namespace futil {

/*!
    \brief Calculate the betweenness.
    
    Performs the Girvan-Newman algorithm.
    
    - O node.
    - I node iterator, I -> O
    - J neighbour iterator, J -> O
    - T scalar type.
    
    - N neighbours function: std::pair<J, J> neighbours(O)
    accepts a node and returns an iterator from the first neighbour
    to one past last neighbour.
    
    - W weight function: T& weight(O, O)
    given two nodes returns a reference to the weight
    of the associated edge.
    
    Given a undirected graph, set the weight of the edges to the betweennes,
    i.e. the number of shortest paths upon edges of weight 1. If two nodes
    have more than one shortest path, the weight is a fraction according to
    ne number of paths.
*/
template <typename O, typename T, typename I, typename N, typename W>
inline
void betweenness(I node_begin, I node_end, N neighbours, W weight)
{
    struct btw
    {
        std::unordered_map<O, int> btw_map;
        std::vector<T> btw_paths;
        std::vector<int> btw_level;
        
        void make_dag(O root, N neighbours)
        {
            std::queue<O> queue;
            std::queue<O> next;
            int m = btw_map[root];
            int count = 0;
            
            queue.push(root);
            btw_paths[m] = 0;
            btw_level[m] = count;
            ++count;
            do {
                auto range = neighbours(queue.front());
                
                for (auto j = range.first; j != range.second; ++j) {
                    m = btw_map[*j];
                    if (btw_level[m] < 0) {
                        btw_level[m] = count;
                        btw_paths[m] = 0;
                        next.push(*j);
                    }
                }
                
                queue.pop();
                if (queue.empty()) {
                    std::swap(queue, next);
                    ++count;
                }
            } while( ! queue.empty());
        }
        
        void count_paths(O o, int l, N neighbours)
        {
            int m = btw_map[o];
            auto range = neighbours(o);
            
            btw_paths[m] += 1;
            for (auto j = range.first; j != range.second; ++j) {
                if (btw_level[btw_map[*j]] > l) {
                    count_paths(*j, l + 1, neighbours);
                }
            }
        }
        
        T summarize(O o, int l, N neighbours, W weight)
        {
            int m = btw_map[o];
            T sum = 1 / btw_paths[m];
            auto range = neighbours(o);
            
            for (auto j = range.first; j != range.second; ++j) {
                if (btw_level[btw_map[*j]] == l + 1) {
                    T bw = summarize(*j, l + 1, neighbours, weight);
                    
                    weight(o, *j) += bw / 2;
                    sum += bw;
                }
            }
            return sum;
        }
        
        btw(I node_begin, I node_end, N neighbours, W weight)
        {
            int num_nodes = 0;
            for (auto i = node_begin; i != node_end; ++i) {
                btw_map[*i] = num_nodes++;
            }
            btw_paths = std::vector<T>(num_nodes);
            btw_level = std::vector<int>(num_nodes);
                      
            for (auto i = node_begin; i != node_end; ++i) {
                std::fill(btw_paths.begin(), btw_paths.end(), 0);
                std::fill(btw_level.begin(), btw_level.end(), -1);
                make_dag(*i, neighbours);
                count_paths(*i, 0, neighbours);
                summarize(*i, 0, neighbours, weight);
            }
        }
        
    } instance(node_begin, node_end, neighbours, weight);
}

} // end futil

#endif