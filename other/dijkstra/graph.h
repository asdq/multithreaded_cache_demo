/**
	@file graph.h
	@author Vaccari Fabio, fabio.vaccari@gmail.com
	@license MIT
	@version 0.2
	@date 20110730
	@date 20110803 nested macros
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

#ifndef GRAPH_H
#define GRAPH_H

#include<stdint.h>
#include<stdio.h>

#define INFINITY INT32_MAX
#define LOWER_BOUND INT32_MIN

/**
	A graph represented by the matrix of its weights.
	INFINITY means no-link.
*/
struct graph {
	int dim;
	int32_t *weights; /* a square matrix of size dim * dim. */
};
typedef struct graph *graph_ptr;

/**
	The weight of an edge.
	@param g a pointer to a graph, graph_ptr
	@param i the first node of a link, int
	@param j the second node of a link, int
	@return the weight, int32_t
*/
#define GRAPH_WEIGHT(g, i, j) (g -> weights[i * g -> dim + j])

/**
	The number of the nodes.
	@param g the graph, graph_ptr
	@return the number of nodes in g, int
*/
#define GRAPH_DIM(g) (g -> dim)

/**
	Allocate a graph. The graph is not initialized.
	@param g a pointer, graph_ptr
	@param dim the dimension of the graph, dim > 0, int
	@return g pointing to either a new graph of dimension dim or NULL, graph_ptr
*/
#define GRAPH_ALLOC(g, dim) \
	(dim > 0 \
		? NULL == (g = (struct graph*) malloc(sizeof(struct graph))) \
			? g \
			: (NULL == (g -> weights = (int32_t*) malloc(sizeof(int32_t) * dim * dim))) \
				? (free(g), g) \
				: (g -> dim = dim, g) \
		: (g = NULL))

/**
	Dispose a graph.
	@param g the graph, if g == NULL does nothing, graph_ptr
	@return g = NULL, graph_ptr
*/
#define GRAPH_FREE(g) \
	(g == NULL \
		? g \
		: (free(g -> weights), free(g), g))

/**
	Errcode from graph_read.
	gre_no_errors - no errors
	gre_format - invalid format
	gre_istream - input stream error
	gre_malloc - memory allocation error
*/
typedef enum {
	gre_no_errors,
	gre_format,
	gre_istream,
	gre_malloc
} graph_read_error;

/**
	Try to read a square matrix of integers from a stream.
	Any token other than integer numbers is switched to INFINITY.
	Example:
	
		0	6	*	*	7
		*	0	5	-4	*
		*	-2	0	*	*
		2	*	7	0	*
		*	*	-3	9	*
	
	The sequence is a matrix 5x5, integers are the weights of the lines.
	@param istream an input stream
	@param gre exitcode, see graph_read_error
	@return a pointer to a graph or null
*/
graph_ptr graph_read(FILE *istream, graph_read_error *gre);

/**
	Write a graph.
	@param ostream the output stream
	@param g the graph
*/
void graph_write(FILE *ostream, graph_ptr g);

/**
	Repersents a graph in a Prufer's notation. For example:
	    (0)
	    / \               0 1 2 3 4 5 6  : nodes = indexes
	  (1) (2)        ->  [* 0 0 1 2 2 *] : the father of a node, array of int
	  /   / \                
	(3) (4) (5) (6)
*/
#define NO_NODE -1
struct prufer {
	int num_nodes;
	int *tree;
};
typedef struct prufer *prufer_ptr;

/**
	The father of a node.
	@param p a pointer to a tree, prufer_ptr
	@param node a node, int
	@return the father, int
*/
#define PRUFER_FATHER(p, node) (p -> tree[node])

/**
	The number of nodes in a tree.
	@param p a pointer to a tree, prufer_ptr
	@return the number of nodes in p, int
*/
#define PRUFER_DIM(p) (p -> num_nodes)

/**
	Allocate a tree.
	@param p  a pointer to a tree, prufer_ptr
	@param dim the dimension of a tree, dim > 0, int
	@return p pointing either to a new tree or NULL, prufer_ptr
*/
#define PRUFER_ALLOC(p, dim) \
	(dim > 0 \
		? NULL == (p = (struct prufer*) malloc(sizeof(struct prufer))) \
			? p \
			: NULL == (p -> tree = (int*) malloc(sizeof(int) * dim)) \
				? (free(p), p) \
				: (p -> num_nodes = dim, p) \
		: (p = NULL))

/**
	Dispose a tree.
	@param p the tree, if p == NULL does nothing, prufer_ptr
	@return p = NULL, prufer_ptr
*/
#define PRUFER_FREE(p) \
	(p == NULL \
		? p \
		: (free(p -> tree), free(p), p))

/**
	Write a path in reverse order.
	@param ostream the output stream
	@param path_tree a pointer to a tree
	@param node the starting point
*/
void reverse_path(FILE *ostream, prufer_ptr path_tree, int node);

/**
	Write a path of a tree, from the root to to an ending point.
	@param ostream the output stream
	@param path_tree a pointer to a tree
	@param last the ending point
*/
void write_path(FILE *ostream, prufer_ptr path_tree, int last);

#endif

