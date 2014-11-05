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

#include <algorithm>
#include <functional>
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
    
    \param root starting node.
    \param neighbors neighbor function.
    \param label label function.
    \param fill_graph function to fill the graph.
    \return the depth reached.
*/
template <typename O, typename T, typename N, typename L, typename F>
inline
T make_dag(O root, N neighbors, L label, F fill_graph)
{
    fill_graph(-1);
    
    std::queue<O> queue;
    std::queue<O> next;
    T depth = 0;
    
    queue.push(root);
    label(root) = depth;
    ++depth;
    
    do {
        auto range = neighbors(queue.front());
        
        for (auto j = range.first; j != range.second; ++j) {
            if (label(*j) < 0) {
                label(*j) = depth;
                next.push(*j);
            }
        }
        
        queue.pop();
        if (queue.empty()) {
            std::swap(queue, next);
            ++depth;
        }
    } while( ! queue.empty());
    
    return depth;
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
    
    Given a undirected graph, set the weight of the edges to the betweennes
    i.e. the number of shortest paths upon edges of weight 1. If two nodes
    have more than one shortest path, the weight is a fraction according to
    ne number of paths.
    
    \param node_begin first node of the graph.
    \param node_end one past last node of the graph.
    \param neighbors neighbor function.
    \param weight weight function.
*/
template <typename O, typename T, typename I, typename N, typename W>
inline
void betweenness(I node_begin, I node_end, N neighbors, W weight)
{
    int num_nodes = 0;
    std::unordered_map<O, int> node_map(num_nodes);
    
    for (auto i = node_begin; i != node_end; ++i) {
        node_map[*i] = num_nodes++;
    }
              
    std::vector<T> paths(num_nodes);
    std::vector<int> graph_depth(num_nodes);
    std::vector<std::tuple<O, int>> stack;
    
    for (auto i = node_begin; i != node_end; ++i) {
        std::fill(paths.begin(), paths.end(), 0);
        
        // make a direct aciclic graph:
        // calculate the depth of the graph
        // starting form a root node
        futil::make_dag<O, int>(*i, neighbors,
            
            // label
            [&] (O o) -> int& {
                return graph_depth[node_map[o]];
            },
            
            // fill_graph
            [&] (T t) {
                std::fill(graph_depth.begin(), graph_depth.end(), t);
            });
        
        // count the number of the paths that reach each node
        // starting from the root
        stack.emplace_back(*i, 0);
        do {
            auto node = std::get<0>(stack.back());
            auto depth = std::get<1>(stack.back());
            stack.pop_back();
            
            auto m = node_map[node];
            auto range = neighbors(node);
            paths[m] += 1;
            for (auto j = range.first; j != range.second; ++j) {
                if (graph_depth[node_map[*j]] > depth) {
                    stack.emplace_back(*j, depth + 1);
                }
            }
        } while ( ! stack.empty());
        
        // calculate the betweennes from the root node
        std::function<T(O, int)> summarize = [&] (O o, int l) {
            int m = node_map[o];
            T sum = 1 / paths[m];
            auto range = neighbors(o);
            
            for (auto j = range.first; j != range.second; ++j) {
                if (graph_depth[node_map[*j]] == l + 1) {
                    T bw = summarize(*j, l + 1);
                    
                    weight(o, *j) += bw / 2;
                    sum += bw;
                }
            }
            return sum;
        };
        
        summarize(*i, 0);
    }
}

} // end futil

#endif