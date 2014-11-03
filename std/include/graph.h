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
    \brief Build a Direct Aciclic Graph.
    
    - O node.
    - J neighbor iterator, J -> O
    - T scalar type: -1, 0, 1, 2, 3, ...
    
    - N neighbors function: std::pair<J, J> neighbors(O)
    accepts a node and returns an iterator from the first neighbor
    to one past last neighbor.
    
    - L label function: T& label(O)
    given two nodes returns a reference to the label
    of the associated node.
    
    - F fill function: void fill_graph(T)
    fill the labels of all nodes with the given value.
    
    Given an undirected graph and a root node, label nodes with the
    distance from the root, where the distance is the number of hops.
*/
template <typename O, typename T, typename N, typename L, typename F>
inline
void make_dag(O root, N neighbors, L label, F fill_graph)
{
    fill_graph(-1);
    
    std::queue<O> queue;
    std::queue<O> next;
    T count = 0;
    
    queue.push(root);
    label(root) = count;
    ++count;
    
    do {
        auto range = neighbors(queue.front());
        
        for (auto j = range.first; j != range.second; ++j) {
            if (label(*j) < 0) {
                label(*j) = count;
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

/*!
    \brief Calculate the betweenness.
    
    Performs the Girvan-Newman algorithm.
    
    - O node.
    - I node iterator, I -> O
    - J neighbor iterator, J -> O
    - T scalar type
    
    - N neighbors function: std::pair<J, J> neighbors(O)
    accepts a node and returns an iterator from the first neighbor
    to one past last neighbor.
    
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
void betweenness(I node_begin, I node_end, N neighbors, W weight)
{
    struct btw
    {
        std::unordered_map<O, int> btw_map;
        std::vector<T> btw_paths;
        std::vector<int> btw_level;
        
        void count_paths(O o, int l, N neighbors)
        {
            int m = btw_map[o];
            auto range = neighbors(o);
            
            btw_paths[m] += 1;
            for (auto j = range.first; j != range.second; ++j) {
                if (btw_level[btw_map[*j]] > l) {
                    count_paths(*j, l + 1, neighbors);
                }
            }
        }
        
        T summarize(O o, int l, N neighbors, W weight)
        {
            int m = btw_map[o];
            T sum = 1 / btw_paths[m];
            auto range = neighbors(o);
            
            for (auto j = range.first; j != range.second; ++j) {
                if (btw_level[btw_map[*j]] == l + 1) {
                    T bw = summarize(*j, l + 1, neighbors, weight);
                    
                    weight(o, *j) += bw / 2;
                    sum += bw;
                }
            }
            return sum;
        }
        
        btw(I node_begin, I node_end, N neighbors, W weight)
        {
            int num_nodes = 0;
            for (auto i = node_begin; i != node_end; ++i) {
                btw_map[*i] = num_nodes++;
            }
            btw_paths.resize(num_nodes);
            btw_level.resize(num_nodes);
                      
            for (auto i = node_begin; i != node_end; ++i) {
                std::fill(btw_paths.begin(), btw_paths.end(), 0);
                
                futil::make_dag<O, int>(*i, neighbors,
                    
                    [this] (O o) -> int& {
                        return btw_level[btw_map[o]];
                    },
                    
                    [this] (T t) {
                        std::fill(btw_level.begin(), btw_level.end(), t);
                    });
                    
                count_paths(*i, 0, neighbors);
                summarize(*i, 0, neighbors, weight);
            }
        }
        
    } instance(node_begin, node_end, neighbors, weight);
}

} // end futil

#endif