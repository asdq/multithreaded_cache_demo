#include "../../include/graph.h"
#include <iostream>
#include <sstream>
#include <tuple>
#include <stdexcept>

using namespace std;

typedef tuple<char, char, float> edge_t;

vector<char> neighbours(char node, const vector<edge_t> &edges)
{
	vector<char> nodes;
	
	for (auto &e : edges) {
		if (node == get<0>(e) && node != get<1>(e)) {
			nodes.push_back(get<1>(e));
		} else if (node == get<1>(e) && node != get<0>(e)) {
			nodes.push_back(get<0>(e));
		}
	}
	
	cout << node << " -> ";
	for (auto c : nodes) {
		cout << c << ' ';
	}
	cout << endl;
	
	return nodes;
}

int main()
{
	vector<char> nodes = { 'A', 'B', 'C', 'D', 'E', 'F', 'G' };
	unordered_map<char, vector<char>> neighbour_map;
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
	
	for (auto n : nodes) {
		neighbour_map[n] = neighbours(n, edges);
	}
	cout << endl;
	
	auto weight = [&edges] (char a, char b) -> float& {
		for (auto &e : edges) {
			if ((a == get<0>(e) && b == get<1>(e)) ||
			    (a == get<1>(e) && b == get<0>(e))) {
				return get<2>(e);
			}
		}
		
		ostringstream in;
		
		in << "Edge (" << a << ',' << b << ") not found.";
		throw range_error(in.str());
	};
	
	auto neighbours_range = [&neighbour_map] (char n) {
		auto i = neighbour_map.find(n);
		
		if (i != neighbour_map.end()) {
			return make_pair(i -> second.begin(), i -> second.end());
		}
		
		ostringstream in;
		
		in << "Node '" << n << "' not found.";
		throw range_error(in.str());
	};
	
	futil::betweenness<char, float>(nodes.begin(), nodes.end(),
		neighbours_range, weight);
	
	sort(edges.begin(), edges.end(),
	    [] (const edge_t &a, const edge_t &b) {
	        return get<2>(a) > get<2>(b);
	    });
	
	for (auto &e : edges) {
		cout << '(' << get<0>(e) << ", " << get<1>(e)
		     << ", " << get<2>(e) << ')' << endl;
	}
}
