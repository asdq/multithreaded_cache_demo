/**	
	@file dijkstra.c
	@author Vaccari Fabio, fabio.vaccari@gmail.com
	@license MIT
	@version 0.2
	@date 20110803
	@date 20110807 better yet simple implementation
	
	usage: dijkstra [net1.txt [net2.txt [..]]]
	
	Where netX.txt are are a network graph. Example for a graph:
	
		0	6	*	*	7
		*	0	5	-4	*
		*	-2	0	*	*
		2	*	7	0	*
		*	*	-3	9	*
	
	The sequence is a matrix 5x5, integers are the weights of the lines.
	The node indexes starts from 0. Any space character is a separator.
	Tokens other than integer values are considered as infinity. Infinity
	means "no link". Coordinates (i,j) mean from i to j.
*/

/*
The MIT License

Copyright (c) 2011 Vaccari Fabio

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

#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include "graph.h"

#define DJ_ALLOC_MSG "Memory allocation failed in funcion dijkstra."
#define DJ_OVERFLOW_MSG "Arithmetic overflow in funcion dijkstra."
#define DJ_NEGATIVE_WEIGHT_MSG "Negative weight found in funcion dijkstra."

/**
	Exits the program with an error message.
	@param msg a message
*/
void fail(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static void exch(int *a, int *b) {
	int c;
	
	c = *a;
	*a = *b;
	*b = c;
}

/**
	The dijkstra algorithm. Node numbers starts from zero.
	Weights must be positive.
	@param g the matrix of the line costs
	@param source the root of the path tree
	@return the path tree in a prufer's notation
*/
prufer_ptr dijkstra(graph_ptr g, int source) {
	const int dim = GRAPH_DIM(g);
	prufer_ptr path_tree = NULL;

	if (dim > 0 && source >= 0 && source < dim) {
		int32_t distance[dim]; /* the distance from the source */
		
		/* the array of nodes is: 
			[0..m[ the set S (path known, ordered)
			[m..n] the boundary (modes in NS connected to a node in S)
			]n..dim[ the set NS (path unknow, not ordered) */
		int node[dim], m, n;
		int i, j; /* i = m - 1 */
		
		/* initialize, set the source with its neighbours */
		if (PRUFER_ALLOC(path_tree, dim) == NULL) fail(DJ_ALLOC_MSG);
		node[0] = source;
		distance[source] =  0;
		PRUFER_FATHER(path_tree, source) = NO_NODE;
		i = 0; m = 1; n = 0;
		for (j = 1; j < dim; ++j) {
			if (j != source) {
				node[j] = j;
				distance[j] = GRAPH_WEIGHT(g, source, j);
			} else {
				node[j] = 0;
				distance[0] = GRAPH_WEIGHT(g, source, 0);
			}
			if (distance[node[j]] < INFINITY) {
				PRUFER_FATHER(path_tree, node[j]) = source;
				if (++n < j) exch(&node[n],&node[j]);
			} else PRUFER_FATHER(path_tree, node[j]) =  NO_NODE;
		}
		
		/* the main cycle */
		while (m <= n) {
		
			/* find the minimum in the boundary */
			for (j = m; j <= n; ++j)
				if (distance[node[j]] < distance[node[m]])
					exch(&node[m], &node[j]);
			i = m++;
			
			/* update nodes in the boundary */
			for (j = m; j <= n; ++j) {
				int32_t sum = distance[node[i]] + GRAPH_WEIGHT(g, node[i], node[j]);
				
				if (sum >= 0 && sum < distance[node[j]]) {
					distance[node[j]] = sum;
					PRUFER_FATHER(path_tree, node[j]) = node[i];
				}
			}
			
			/* add nodes to the boundary */
			for (j = n + 1; j < dim; ++j) {
				if (GRAPH_WEIGHT(g, node[i], node[j]) < INFINITY) {
					distance[node[j]] =  distance[node[i]] + GRAPH_WEIGHT(g, node[i], node[j]);
					if (distance[node[j]] < 0) fail(DJ_OVERFLOW_MSG);
					PRUFER_FATHER(path_tree, node[j]) = node[i];
					if (++n < j) exch(&node[n],&node[j]);
				}
			}
		}
	}
	return path_tree;
}

/*
	Print the shortest paths for each node in a graph to the standard output.
	@param g a network
*/
static void all_paths(graph_ptr g) {
	int dim = GRAPH_DIM(g);
	int i, j;
		
	/* print the graph */
	graph_write(stdout, g);
	putchar('\n');
		
	/* print the paths */
	for (i = 0; i < dim; ++i) {
		prufer_ptr path_tree = dijkstra(g, i);
		
		if (path_tree != NULL)
			for (j = 0; j < dim; ++j)
				write_path(stdout, path_tree, j);
		putchar('\n');
		PRUFER_FREE(path_tree);
	}
}

/**
	Calculate a minimum path with the labelling algorithm.
	File representing a network are passed as parameters.
	For each network calculate the label's field and write it to the standard output.
	Write the path trees.
	@param argc th number of arguments
	@param argv the list of arguments
*/
int main(int argc, char **argv) {
	char *msg = "\nProgram Dijkstra v0.2\nusage: dijkstra [net1.txt [net2.txt [..]]]\n\n";
	char *gre_msg[] = {
		"No errors in function graph_read.",
		"Illegal matrix or number format.",
		"Input stream error.",
		"Memory allocation error in function graph_read."
	};
	int i;

	if(argc == 1) {
		printf("%s", msg);
		exit(EXIT_SUCCESS);
	}
	for (i = 1; i < argc; ++i) {
		FILE *ifs = fopen(argv[i], "r");
		
		if (ifs != NULL) { 
			graph_ptr g;
			graph_read_error gre;
			
			printf("\n%s:\n", argv[i]);
			g = graph_read(ifs, &gre);
			if (gre != gre_no_errors) {
				fail(gre_msg[gre]);
			}
			if (g != NULL) {
				all_paths(g);
				GRAPH_FREE(g);
			}
			fclose(ifs);
		} else {
			fprintf(stderr, "Cannot open file %s.\n", argv[i]);
		}
	}
	return EXIT_SUCCESS;
}
