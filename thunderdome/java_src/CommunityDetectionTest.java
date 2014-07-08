import generated.scala.communitydetectionp.Tup2DeliteArrayIntDeliteArrayInt;
import org.la4j.matrix.dense.Basic2DMatrix;
import org.la4j.vector.dense.BasicVector;
import xlang.java.Graph;

import java.util.Arrays;

public class CommunityDetectionTest {
  public static void main(String[] args) {
    int[] nodes = {0, 2, 4};
    int[] edges = {1, 2, 0, 2, 0, 1};
    Tup2DeliteArrayIntDeliteArrayInt result = communityDetection(nodes, edges);

    int[] resultNodes = result._1();
    int[] resultEdges = result._2();

    for (Integer n : resultNodes) System.out.println("node " + n);
    for (Integer n : resultEdges) System.out.println("edge " + n);


    Basic2DMatrix matrix = new Basic2DMatrix(3,3);
    int i = 0;
    matrix.setRow(i++, new BasicVector(new double[]{0, 1, 1}));
    matrix.setRow(i++, new BasicVector(new double[]{1, 0, 1}));
    matrix.setRow(i++, new BasicVector(new double[]{1, 1, 0}));

    String[] objects = new String[]{"0", "2", "4"};

    communityDetection(new Graph(objects,matrix));

    testCD();
  }

  public static Graph testCD() {
    Basic2DMatrix matrix = new Basic2DMatrix(3,3);
    int i = 0;
    matrix.setRow(i++, new BasicVector(new double[]{0, 1, 1}));
    matrix.setRow(i++, new BasicVector(new double[]{1, 0, 1}));
    matrix.setRow(i++, new BasicVector(new double[]{1, 1, 0}));

    String[] objects = new String[]{"0", "2", "4"};

    Graph graph = new Graph(objects, matrix);
    System.out.println("graph " + graph);
    communityDetection(graph);
    return graph;
  }


  public static void communityDetection(Graph graph) {
    double k = 0.01;

    graph.getNodeNames();
    int [] nodes = new int[graph.getNodeNames().length];
    int i = 0;
    for (String name : graph.getNodeNames()) nodes[i++] =Integer.parseInt(name);
    int [] edges = new int[graph.getValues().length];
    for (i = 0; i < edges.length; i++) {
      System.out.println(" i " + i + " val " + graph.getColIndex()[i]);
    }
    Tup2DeliteArrayIntDeliteArrayInt result = CommunityDetection.apply(nodes, graph.getColIndex(), k);

    int[] resultNodes = result._1();
    int[] resultEdges = result._2();

    for (Integer n : resultNodes) System.out.println("node " + n);
    for (Integer n : resultEdges) System.out.println("edge " + n);

  }

  private static Tup2DeliteArrayIntDeliteArrayInt communityDetection(int[] nodes, int[] edges) {
    double k = 0.01;

    return CommunityDetection.apply(nodes, edges, k);
  }
}
