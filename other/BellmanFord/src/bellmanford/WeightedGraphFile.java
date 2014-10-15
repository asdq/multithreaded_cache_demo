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

import edu.uci.ics.jung.graph.Graph;
import edu.uci.ics.jung.graph.SparseGraph;
import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.io.GraphFile;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;


/**
 * Read a graph from a file. The file format is a sequence of numbers
 * representing the matrix of weights. The node indexes starts from 0.
 * Any space character is a separator. Tokens other than integer values are
 * considered as infinity. Infinity means "no link".
 * @author Vaccari Fabio, fabio.vaccari@gmail.com
 * @version 0.1
 */
public class WeightedGraphFile implements GraphFile<LabelledNode, WeightedEdge>
{
   private double[][] scan(Scanner sc, int dim, int count) {
      double[][] matrix = null;

      if (sc.hasNext()) {
         double d;

         if (sc.hasNextDouble()) {
            d = sc.nextDouble();
         } else {
            d = Double.POSITIVE_INFINITY;
            sc.next();
         }
         if (dim * dim < count) {
            ++dim;
         }

         matrix = scan(sc, dim, count + 1);

         if (matrix != null) {
            dim = matrix.length;
            matrix[count / dim][count % dim] = d;
         }
      } else if (dim * dim == count) {
         matrix = new double[dim][dim];
      }
      return matrix;
   }

   @Override
   public Graph<LabelledNode, WeightedEdge> load(String aFileName) {
      Graph<LabelledNode, WeightedEdge> graph = new SparseGraph<LabelledNode, WeightedEdge>();

      try {
         Scanner sc = new Scanner(new File(aFileName));
         double[][] weightMatrix = scan(sc, 0, 0);

         for (int i = 0; i < weightMatrix.length; ++i) {
            for (int j = i + 1; j < weightMatrix[i].length; ++j) {
               LabelledNode ni = new LabelledNode(i);
               LabelledNode nj = new LabelledNode(j);
               EdgeType et = (weightMatrix[i][j] == weightMatrix[j][i]) ? EdgeType.UNDIRECTED : EdgeType.DIRECTED;

               ni.setLabel(Integer.toString(i));
               nj.setLabel(Integer.toString(j));
               if (weightMatrix[i][j] != Double.POSITIVE_INFINITY) {
                  int edgeId = i * weightMatrix.length + j;
                  WeightedEdge eij = new WeightedEdge(edgeId);

                  eij.setLabel(Double.toString(weightMatrix[i][j]));
                  eij.setWeight(weightMatrix[i][j]);
                  graph.addEdge(eij, ni, nj, et);
               }
               if (et == EdgeType.DIRECTED && weightMatrix[j][i] != Double.POSITIVE_INFINITY) {
                  int edgeId = i + weightMatrix.length * j;
                  WeightedEdge eji = new WeightedEdge(edgeId);

                  eji.setLabel(Double.toString(weightMatrix[j][i]));
                  eji.setWeight(weightMatrix[j][i]);
                  graph.addEdge(eji, nj, ni, et);
               }
            }
         }
      } catch (FileNotFoundException ex) {
         Logger.getLogger(this.getClass().getName()).log(Level.SEVERE, null, ex);
      }
      Logger.getLogger(this.getClass().getName()).log(Level.INFO, graph.toString());
      return graph;
   }

   @Override
   public void save(Graph<LabelledNode, WeightedEdge> graph, String filename) {
      throw new UnsupportedOperationException("Not supported yet.");
   }
}
