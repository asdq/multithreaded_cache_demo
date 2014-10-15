/*
The MIT License

Copyright (c) 2012 Vaccari Fabio

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

package bellmanford;

import edu.uci.ics.jung.algorithms.shortestpath.ShortestPath;
import edu.uci.ics.jung.graph.Graph;
import edu.uci.ics.jung.graph.util.Pair;
import java.util.HashMap;
import java.util.Map;
import org.apache.commons.collections15.Transformer;


/**
 * Calculate the shortest path with the Bellman-Ford algorithm.
 * This class wraps upon a graph, so changes in the graph have side effects
 * in the class instance.
 * Weights in the edges can be negative, but the order of the resulting map in
 * getIncomingEdgeMap is not guaranteed any more.
 * @author Vaccari Fabio, fabio.vaccari@gmail.com
 * @version 0.1
 * @see ShortestPath
 */
public class BellmanFordShortestPath<V, E> implements ShortestPath<V, E>
{
   private final Graph<V, E> graph;
   private final Transformer<E, ? extends Number> weights;

   /**
    * Create a new instance of BellmanFordShortestPath
    * with the default metric (hop based).
    * @param aGraph the graph to wrap
    */
   public BellmanFordShortestPath(Graph<V, E> aGraph) {
      graph = aGraph;
      weights = new Transformer<E, Integer>()
      {
         @Override
         public Integer transform(E input) {
            return 1; // hop based
         }
      };
   }

   /**
    * Create a new instance of BellmanFordShortestPath
    * with specific weights.
    * @param aGraph the graph to wrap
    * @param aWeight map to the weights
    */
   public BellmanFordShortestPath(Graph<V, E> aGraph,
        Transformer<E, ? extends Number> aWeight) {
      graph = aGraph;
      weights = aWeight;
   }

   /**
    * Returns a Map which maps each vertex in the graph (including
    * the source vertex) to the last edge on the shortest path from
    * the source vertex. If a negative cycle is found returns null.
    * @param aSource the source
    * @return the path tree or null
    */
   @Override
   public Map<V, E> getIncomingEdgeMap(V aSource) {
      final int dim = graph.getVertexCount();
      Map<V, Double> labels = new HashMap<V, Double>(dim);
      Map<V, Double> previousLabels = new HashMap<V, Double>(dim);
      Map<V, E> pathTree = new HashMap<V, E>(dim);
      int maxPathLen;
      boolean optimal;

      for (V vertex : graph.getVertices()) {
         labels.put(vertex, Double.POSITIVE_INFINITY);
      }
      labels.put(aSource, Double.valueOf(0));

      optimal = false;
      maxPathLen = 0;
      while (!optimal && maxPathLen < dim) {
         optimal = true;
         previousLabels.clear();
         previousLabels.putAll(labels);
         for (E edge : graph.getEdges()) {
            Pair<V> endPoints = graph.getEndpoints(edge);
            V v1 = endPoints.getFirst();
            V v2 = endPoints.getSecond();
            V source = graph.getSource(edge); // null if undirected edge

            if (!v1.equals(v2)) {
               if (source != null && v2 == source) {
                  v2 = v1;
                  v1 = source;
               }

               double sourceLabel = previousLabels.get(v1).doubleValue();
               double destLabel = previousLabels.get(v2).doubleValue();
               double lineWeight = weights.transform(edge).doubleValue();

               if (sourceLabel + lineWeight < destLabel) {
                  labels.put(v2, Double.valueOf(sourceLabel + lineWeight));
                  pathTree.put(v2, edge);
                  optimal = false;
               }
               if (source == null && destLabel + lineWeight < sourceLabel) {
                  labels.put(v1, Double.valueOf(destLabel + lineWeight));
                  pathTree.put(v1, edge);
                  optimal = false;
               }
            }
         }
         ++maxPathLen;
      }
      return (optimal) ? pathTree : null;
   }
}
