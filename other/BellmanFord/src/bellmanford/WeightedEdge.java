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

/**
 * As is.
 * @author Vaccari Fabio, fabio.vaccari@gmail.com
 * @version 0.1
 */
public class WeightedEdge implements Comparable<WeightedEdge>
{
   private final int id;
   private double weight;
   private String label;

   public WeightedEdge(int anId) {
      id = anId;
   }

   public String getLabel() {
      return label;
   }

   public void setLabel(String aLabel) {
      label = aLabel;
   }

   public double getWeight() {
      return weight;
   }

   public void setWeight(double aWeight) {
      weight = aWeight;
   }

   @Override
   public String toString() {
      return Integer.toString(id);
   }

   @Override
   public boolean equals(Object obj) {
      if (obj == null) {
         return false;
      }
      if (getClass() != obj.getClass()) {
         return false;
      }
      final WeightedEdge other = (WeightedEdge) obj;
      if (this.id != other.id) {
         return false;
      }
      return true;
   }

   @Override
   public int hashCode() {
      int hash = 7;
      hash = 47 * hash + this.id;
      return hash;
   }

   @Override
   public int compareTo(WeightedEdge w) {
      return id - w.id;
   }
}
