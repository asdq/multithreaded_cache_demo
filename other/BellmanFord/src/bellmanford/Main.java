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

/*
 * Main.java
 *
 * Created on 18-Jul-2011, 12:44:17
 * 
 * Changelog:
 * 26-Feb-2012 corrected node selection.
 * 16-Jun-2012 added text area.
 */
package bellmanford;

import edu.uci.ics.jung.algorithms.layout.FRLayout;
import edu.uci.ics.jung.algorithms.layout.ISOMLayout;
import edu.uci.ics.jung.algorithms.layout.Layout;
import edu.uci.ics.jung.algorithms.shortestpath.ShortestPathUtils;
import edu.uci.ics.jung.graph.Graph;
import edu.uci.ics.jung.graph.SparseGraph;
import edu.uci.ics.jung.graph.util.Pair;
import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.control.DefaultModalGraphMouse;
import edu.uci.ics.jung.visualization.control.ModalGraphMouse;
import edu.uci.ics.jung.visualization.picking.PickedState;
import edu.uci.ics.jung.visualization.renderers.Renderer;
import edu.uci.ics.jung.visualization.renderers.Renderer.VertexLabel.Position;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Paint;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JFileChooser;
import javax.swing.JTextArea;
import org.apache.commons.collections15.Transformer;


/**
 * Bellman-Ford shortest path demo.
 *
 * @author Vaccari Fabio, fabio.vaccari@gmail.com
 * @version 0.4
 */
public class Main extends javax.swing.JFrame
{
   private final Logger logger;
   private final VisualizationViewer<LabelledNode, WeightedEdge> graphViewer;
   private final DefaultModalGraphMouse<LabelledNode, WeightedEdge> graphMouse;
   private final ShortestPath shortestPath;
   private Layout<LabelledNode, WeightedEdge> graphLayout;

   /**
    * Creates new form Main
    */
   public Main() {
      super();
      logger = Logger.getLogger(this.getClass().getName());

      // Set a default graph layout, FRLayout is choosen because
      // ISOMLayout does not work with an empty gtaph. So will switch
      // between FRLayout and/or ISOMLayout when loading a graph.
      graphLayout = new FRLayout<LabelledNode, WeightedEdge>(new SparseGraph<LabelledNode, WeightedEdge>());
      graphLayout.setSize(new Dimension(300, 300));
      graphViewer = new VisualizationViewer<LabelledNode, WeightedEdge>(graphLayout);
      graphViewer.setPreferredSize(new Dimension(330, 330));
      graphViewer.getRenderContext().setVertexLabelTransformer(new Transformer<LabelledNode, String>()
      {
         @Override
         public String transform(LabelledNode aNode) {
            return aNode.getLabel();
         }
      });
      graphViewer.getRenderer().getVertexLabelRenderer().setPosition(Position.CNTR);
      graphViewer.getRenderContext().setEdgeLabelTransformer(new Transformer<WeightedEdge, String>()
      {
         @Override
         public String transform(WeightedEdge anEdge) {
            return Double.toString(anEdge.getWeight());
         }
      });
      graphMouse = new DefaultModalGraphMouse<LabelledNode, WeightedEdge>();
      initComponents(); // must be here since we get mouse mode from a button label
      graphMouse.setMode(ModalGraphMouse.Mode.valueOf(mouseModeButton.getText()));
      graphViewer.addKeyListener(graphMouse.getModeKeyListener());
      graphViewer.setGraphMouse(graphMouse);
      getContentPane().add(graphViewer);
      pack();

      // Inject shortest path algorithm. A vertex the mouse was selected actually changes his state.
      shortestPath = new ShortestPath(graphViewer);
      shortestPath.setTextArea(textArea);
      graphViewer.getPickedVertexState().addItemListener(shortestPath);
   }

   /**
    * This method is called from within the constructor to initialise the form.
    * WARNING: Do NOT modify this code. The content of this method is always
    * regenerated by the Form Editor.
    */
   @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        fileDialog = new javax.swing.JDialog();
        fileChooser = new javax.swing.JFileChooser();
        buttonBar = new javax.swing.JToolBar();
        mouseModeButton = new javax.swing.JButton();
        scrollPane = new javax.swing.JScrollPane();
        textArea = new javax.swing.JTextArea();
        mainMenuBar = new javax.swing.JMenuBar();
        fileMenu = new javax.swing.JMenu();
        loadMenuItem = new javax.swing.JMenuItem();
        exitMenuItem = new javax.swing.JMenuItem();

        fileDialog.setTitle("Load a Graph");

        javax.swing.GroupLayout fileDialogLayout = new javax.swing.GroupLayout(fileDialog.getContentPane());
        fileDialog.getContentPane().setLayout(fileDialogLayout);
        fileDialogLayout.setHorizontalGroup(
            fileDialogLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(fileDialogLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(fileChooser, javax.swing.GroupLayout.DEFAULT_SIZE, 376, Short.MAX_VALUE)
                .addContainerGap())
        );
        fileDialogLayout.setVerticalGroup(
            fileDialogLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, fileDialogLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(fileChooser, javax.swing.GroupLayout.PREFERRED_SIZE, 288, Short.MAX_VALUE)
                .addContainerGap())
        );

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        buttonBar.setRollover(true);

        mouseModeButton.setText(ModalGraphMouse.Mode.PICKING.toString());
        mouseModeButton.setFocusable(false);
        mouseModeButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        mouseModeButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        mouseModeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mouseModeButtonActionPerformed(evt);
            }
        });
        buttonBar.add(mouseModeButton);

        getContentPane().add(buttonBar, java.awt.BorderLayout.PAGE_START);

        textArea.setColumns(20);
        textArea.setRows(5);
        scrollPane.setViewportView(textArea);

        getContentPane().add(scrollPane, java.awt.BorderLayout.PAGE_END);

        fileMenu.setText("File");

        loadMenuItem.setText("Load a Graph");
        loadMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                loadMenuItemActionPerformed(evt);
            }
        });
        fileMenu.add(loadMenuItem);

        exitMenuItem.setText("Exit");
        exitMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                exitMenuItemActionPerformed(evt);
            }
        });
        fileMenu.add(exitMenuItem);

        mainMenuBar.add(fileMenu);

        setJMenuBar(mainMenuBar);
    }// </editor-fold>//GEN-END:initComponents

   /**
    * Exit the program.
    *
    * @param evt
    */
    private void exitMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_exitMenuItemActionPerformed
       System.exit(0);
    }//GEN-LAST:event_exitMenuItemActionPerformed

   /**
    * Load a graph.
    *
    * @param evt
    */
    private void loadMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_loadMenuItemActionPerformed
       switch (fileChooser.showOpenDialog(fileDialog)) {
          case JFileChooser.CANCEL_OPTION:
             break;
          case JFileChooser.APPROVE_OPTION:
             WeightedGraphFile wgf = new WeightedGraphFile();
             Graph<LabelledNode, WeightedEdge> graph = wgf.load(fileChooser.getSelectedFile().getAbsolutePath());

             graphViewer.removeAll();
             shortestPath.reset();
             if (graph != null && graph.getVertexCount() > 0) {
                textArea.append(graph.toString() + '\n');
                graphLayout = new ISOMLayout<LabelledNode, WeightedEdge>(graph);
                graphViewer.setGraphLayout(graphLayout);
             } else { // set empty layout on load failure
                graphLayout = new FRLayout<LabelledNode, WeightedEdge>(new SparseGraph<LabelledNode, WeightedEdge>());
                graphViewer.setGraphLayout(graphLayout);
             }
             break;
          case JFileChooser.ERROR_OPTION:
          default:
             logger.log(Level.WARNING, "Unhandled state: {0}", fileChooser.showOpenDialog(fileDialog));
             break;
       }
    }//GEN-LAST:event_loadMenuItemActionPerformed

   /**
    * Change how the mouse acts on a graph.
    *
    * @param evt
    */
    private void mouseModeButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mouseModeButtonActionPerformed
       ModalGraphMouse.Mode mode = ModalGraphMouse.Mode.PICKING;

       switch (ModalGraphMouse.Mode.valueOf(mouseModeButton.getText())) {
          case PICKING:
             mode = ModalGraphMouse.Mode.TRANSFORMING;
             break;
          case TRANSFORMING:
             mode = ModalGraphMouse.Mode.PICKING;
             break;
          case ANNOTATING:
          case EDITING:
          default:
             logger.log(Level.WARNING, "Unhandled state: {0}", ModalGraphMouse.Mode.valueOf(mouseModeButton.getText()));
             mode = ModalGraphMouse.Mode.TRANSFORMING;
             break;
       }
       graphMouse.setMode(mode);
       mouseModeButton.setText(mode.toString());
    }//GEN-LAST:event_mouseModeButtonActionPerformed


   /**
    * Handle mouse selections, calculate and display shortest path. Use the
    * Bellman-Ford algorithm.
    */
   private static class ShortestPath implements ItemListener //, ActionListener
   {
      private final Logger logger;
      private final VisualizationViewer<LabelledNode, WeightedEdge> viewer;
      private final Set<LabelledNode> pathNodes = new TreeSet<LabelledNode>();
      private final Set<WeightedEdge> pathEdges = new TreeSet<WeightedEdge>();
      private JTextArea textArea;

      /**
       * initialise, setup painting.
       *
       * @param aViewer
       */
      public ShortestPath(VisualizationViewer<LabelledNode, WeightedEdge> aViewer) {
         super();
         logger = Logger.getLogger(this.getClass().getName());
         viewer = aViewer;
         textArea = null;

         Transformer<WeightedEdge, Paint> paintEdge = new Transformer<WeightedEdge, Paint>()
         {
            @Override
            public Paint transform(WeightedEdge e) {
               return pathEdges.contains(e) ? Color.YELLOW : Color.BLACK;
            }
         };
         viewer.getRenderContext().setEdgeDrawPaintTransformer(paintEdge);
         viewer.getRenderContext().setArrowDrawPaintTransformer(paintEdge);
         viewer.getRenderContext().setArrowFillPaintTransformer(paintEdge);

         Transformer<LabelledNode, Paint> paintNode = new Transformer<LabelledNode, Paint>()
         {
            @Override
            public Paint transform(LabelledNode v) {
               Color c = Color.WHITE;

               if (viewer.getPickedVertexState().isPicked(v)) {
                  c = Color.RED;
               }
               if (pathNodes.contains(v)) {
                  c = Color.YELLOW;
               }
               return c;
            }
         };
         viewer.getRenderContext().setVertexFillPaintTransformer(paintNode);
         viewer.getRenderContext().setVertexDrawPaintTransformer(paintNode);
      }

      /**
       * Cleanup graph.
       */
      public void reset() {
         pathNodes.clear();
         pathEdges.clear();
      }

      @Override
      public void itemStateChanged(ItemEvent e) {
         LabelledNode subject = (e.getItem() instanceof LabelledNode) ? (LabelledNode) e.getItem() : null;
         PickedState<LabelledNode> source = (e.getSource() instanceof PickedState) ? (PickedState<LabelledNode>) e.getSource() : null;

         if (subject != null && source != null) {
            pathNodes.clear();
            pathEdges.clear();
            if (source.getPicked().size() == 2 && source.isPicked(subject)) {

               // get the nodes respecting order
               LabelledNode[] nodes = source.getPicked().toArray(new LabelledNode[2]);
               if (nodes[0] == subject) {
                  nodes[0] = nodes[1];
                  nodes[1] = subject;
               }

               // get the shortest path
               Graph<LabelledNode, WeightedEdge> graph = viewer.getGraphLayout().getGraph();
               BellmanFordShortestPath<LabelledNode, WeightedEdge> bf = new BellmanFordShortestPath<LabelledNode, WeightedEdge>(graph, new Transformer<WeightedEdge, Double>()
               {
                  @Override
                  public Double transform(WeightedEdge input) {
                     return input.getWeight();
                  }
               });
               List<WeightedEdge> path = ShortestPathUtils.getPath(graph, bf, nodes[0], nodes[1]);

               for (WeightedEdge we : path) {
                  Pair<LabelledNode> endPoints = graph.getEndpoints(we);

                  pathEdges.add(we);
                  pathNodes.add(endPoints.getFirst());
                  pathNodes.add(endPoints.getSecond());
               }
               writePath(path, nodes[0]);
               logger.log(Level.INFO, "Path: {0}", pathEdges.toString());
            }
            rePaint();
         }
      }

      private void rePaint() {
         Graph<LabelledNode, WeightedEdge> graph = viewer.getGraphLayout().getGraph();

         if (!pathEdges.isEmpty()) {
            Renderer.Edge<LabelledNode, WeightedEdge> re = viewer.getRenderer().getEdgeRenderer();
            Renderer.Vertex<LabelledNode, WeightedEdge> rv = viewer.getRenderer().getVertexRenderer();

            for (WeightedEdge we : graph.getEdges()) {
               Pair<LabelledNode> endPoints = graph.getEndpoints(we);
               re.paintEdge(viewer.getRenderContext(), viewer.getGraphLayout(), we);
               rv.paintVertex(viewer.getRenderContext(), viewer.getGraphLayout(), endPoints.getFirst());
               rv.paintVertex(viewer.getRenderContext(), viewer.getGraphLayout(), endPoints.getSecond());
            }
         }

      }

      private void writePath(List<WeightedEdge> path, LabelledNode source) {
         if (textArea == null) {
            return;
         }

         Graph<LabelledNode, WeightedEdge> graph = viewer.getGraphLayout().getGraph();
         LabelledNode last = source;
         double cost = 0;

         textArea.append("path: " + last.getLabel() + ' ');
         for (WeightedEdge we : path) {
            Pair<LabelledNode> endPoints = graph.getEndpoints(we);
            
            cost += we.getWeight();
            last = (last.equals(endPoints.getFirst())) ? endPoints.getSecond() : endPoints.getFirst();
            textArea.append(last.getLabel() + ' ');
         }
         textArea.append("cost: " + cost + '\n');
      }

      /**
       * Get the value of textArea. TextArea is used to print the path in a
       * readable text form.
       *
       * @return the value of textArea
       */
      public JTextArea getTextArea() {
         return textArea;
      }

      /**
       * Set the value of textArea. TextArea is used to print the path in a
       * readable text form.
       *
       * @param t new value of textArea
       */
      public void setTextArea(JTextArea t) {
         textArea = t;
      }

      /*
       * @Override public void actionPerformed(ActionEvent e) { throw new
       * UnsupportedOperationException("Not supported yet."); }
       */
   }

   /**
    * Run.
    *
    * @param args the command line arguments, not used
    */
   public static void main(String args[]) {
      java.awt.EventQueue.invokeLater(new Runnable()
      {
         @Override
         public void run() {
            new Main().setVisible(true);
         }
      });
   }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JToolBar buttonBar;
    private javax.swing.JMenuItem exitMenuItem;
    private javax.swing.JFileChooser fileChooser;
    private javax.swing.JDialog fileDialog;
    private javax.swing.JMenu fileMenu;
    private javax.swing.JMenuItem loadMenuItem;
    private javax.swing.JMenuBar mainMenuBar;
    private javax.swing.JButton mouseModeButton;
    private javax.swing.JScrollPane scrollPane;
    private javax.swing.JTextArea textArea;
    // End of variables declaration//GEN-END:variables
}
