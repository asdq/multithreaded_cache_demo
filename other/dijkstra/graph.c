/**
	@file graph.h
	@author Vaccari Fabio, fabio.vaccari@gmail.com
	@license MIT
	@version 0.2
	@date 20110730
	@date 20110803 nested macros
	
	See graph.h.
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

#include "graph.h"
#include <stdlib.h>
#include <ctype.h>

#define RT_MINUS '-'
#define RT_PLUS '+'

/*
	Tries to read an integer value from a stream.
	Integers are in decimal notation. Uses ASCII code.
	Tokens other than integers are converted to INFINITY.
	Exits the program on a stream error or integer overflows.
*/
static int read_token(FILE *istream, graph_read_error *gre) {
	enum expect { blank, number, digit } next = number;
	int sgn = 1;
	int num = 0;
	
	while (!feof(istream)) {
		int c = getc(istream);
		
		if (ferror(istream)) {
			*gre = gre_istream;
			return INFINITY;
		}
		if (num < 0) {
			*gre = gre_format;
			return INFINITY;
		}
		switch (next) {
			case number:
				if (isspace(c)) continue;
				if (c == RT_MINUS) {
					sgn = -1;
					next = digit;
					break;
				}
				if (c == RT_PLUS) {
					next = digit;
					break;
				}
			case digit:
				if (isdigit(c)) {
					num = 10 * num + (c - '0'); /* ASCII code */
					next = digit;
					break;
				}
			case blank:
				if (isspace(c)) {
					ungetc(c, istream);
					return sgn * num;
				}
			default:
				return INFINITY;
		}
	}
	return sgn * num;
}

#undef RT_MINUS
#undef RT_PLUS

graph_ptr graph_read(FILE *istream, graph_read_error *gre) {
	int dim, count;
	int weight;
	struct node_list {
		struct node_list *next_node;
		int32_t weight;
	} *head, *next;
	graph_ptr g = NULL;
	
	*gre = gre_no_errors;
	/* build the token list */
	dim = count = 0;
	head = NULL;
	weight = read_token(istream, gre);
	while (!feof(istream) && *gre == gre_no_errors) {
		if (++count > dim * dim) ++dim;
		next = head;
		head = (struct node_list*) malloc(sizeof(struct node_list));
		if (head != NULL) {
			head -> weight = (int32_t) weight; /* TODO check bounds */
			head -> next_node = next;
			weight = read_token(istream, gre);
		} else *gre = gre_malloc;
	}
	
	if (*gre == gre_no_errors) {
		if (count == dim * dim) GRAPH_ALLOC(g, dim);
		else *gre = gre_format;
	}
	if (g == NULL) {
		count = 0;
	}
	
	/* assign weights (if any) and free the token list */
	while (head != NULL) {
		if (count > 0) {
			int i, j;
			
			--count;
			i = count / dim;
			j = count % dim;
			GRAPH_WEIGHT(g, i, j) = head -> weight;
		}
		next = head;
		head = head -> next_node;
		free(next);
	}
	return g;
}

void graph_write(FILE *ostream, graph_ptr g) {
	if (g != NULL) {
		int dim = GRAPH_DIM(g);
		int i, j;
		
		for (i = 0; i < dim; ++i) {
			for (j = 0; j < dim; ++j) {
				if (GRAPH_WEIGHT(g, i, j) == INFINITY) fprintf(ostream, "*\t");
				else fprintf(ostream, "%i\t", GRAPH_WEIGHT(g, i, j));
			}
			fprintf(ostream, "\n");
		}
	}
}

void reverse_path(FILE *ostream, prufer_ptr path_tree, int node) {
	int father = PRUFER_FATHER(path_tree, node);
	
	if (father != NO_NODE) {
		reverse_path(ostream, path_tree, father);
		fprintf(ostream, " %i", node);
	} else {
		fprintf(ostream, "%i", node);
	}
}

void write_path(FILE *ostream, prufer_ptr path_tree, int last) {
	if (PRUFER_FATHER(path_tree, last) != NO_NODE) {
		fprintf(ostream, "(");
		reverse_path(ostream, path_tree, last);
		fprintf(ostream, ")\n");
	}
}

