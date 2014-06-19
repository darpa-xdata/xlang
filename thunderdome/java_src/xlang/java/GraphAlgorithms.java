package xlang.java;

import org.la4j.matrix.dense.Basic2DMatrix;
import org.la4j.matrix.sparse.CRSMatrix;
import org.la4j.vector.dense.BasicVector;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.util.Random;

/**
 * Created by go22670 on 6/19/14.
 */
public class GraphAlgorithms  {
  public void someMethod() {}

  static Random random = new Random();
  public static int sqr(int n) {
    return n * n;
  }

  public static double sin(double deg) {
    double sin = AnotherClass.sin(deg);
    //new Exception().printStackTrace();
    //System.out.println("Xlang : sin of " + deg +  " = " + sin);
    return sin;
  }

  public static boolean isEven(int n) { return n % 2==0;}

  public static int mult(int n, int m) {
    return n * m;
  }
  public static int strLen(String n) {
    return n.length();
  }
  public static int nextInt() {
    return random.nextInt();
  }
  public static String toUpper(String str) {
    return str.toUpperCase();
  }

  public static Graph echoGraph(Graph input) {
    return input;
  }

  public static Graph getExampleGraph() {
    Basic2DMatrix matrix = new Basic2DMatrix(4, 4);    // not really needed
    int i = 0;
    matrix.setRow(i++, new BasicVector(new double[]{0, 0, 0, 0}));
    matrix.setRow(i++, new BasicVector(new double[]{5, 8, 0, 0}));
    matrix.setRow(i++, new BasicVector(new double[]{0, 0, 3, 0}));
    matrix.setRow(i++, new BasicVector(new double[]{0, 6, 0, 0}));

    String[] nodeNames = {"one", "two", "three", "four"};

    return new Graph(nodeNames, matrix);
  }

  public static Graph getExampleGraph2() {
    int n = 6;
    Basic2DMatrix matrix = new Basic2DMatrix(n, n);    // not really needed
    int i = 0;
    matrix.setRow(i++, new BasicVector(new double[]{10, 20, 0, 0, 0, 0}));
    matrix.setRow(i++, new BasicVector(new double[]{0, 30, 0, 40, 0, 0}));
    matrix.setRow(i++, new BasicVector(new double[]{0, 0, 50, 60, 70, 0}));
    matrix.setRow(i++, new BasicVector(new double[]{0, 0, 0, 0, 0, 80}));
    matrix.setRow(i++, new BasicVector(new double[]{0, 0, 0, 0, 0, 0}));
    matrix.setRow(i++, new BasicVector(new double[]{0, 0, 0, 0, 0, 0}));

    String[] nodeNames = {"one", "two", "three", "four", "five", "six"};

    return new Graph(nodeNames, matrix);
  }

  private static String printMatrix(Graph exampleGraph) {
    StringBuilder stringBuilder = new StringBuilder();
    double[][] doubles = exampleGraph.toMatrix();


    // square
    for (int i = 0; i < doubles.length; i++) {
      for (int j = 0; j < doubles.length; j++) {
        stringBuilder.append(doubles[i][j]).append(" ");
      }
      stringBuilder.append("\n");
    }
    return stringBuilder.toString();
  }

  public static void main(String [] arg) {
    Graph exampleGraph = getExampleGraph();

    System.out.println("example " + exampleGraph);
    System.out.println("back    \n");
    System.out.println(printMatrix(exampleGraph));

    Graph exampleGraph2 = getExampleGraph2();

    System.out.println("example " + exampleGraph2);
    System.out.println("back    \n");

    System.out.println(printMatrix(exampleGraph2));

/*
    int n = nodeNames.length;
    //CRSMatrix crsMatrix = new CRSMatrix(matrix);
    List<Double> values = new ArrayList<Double>();
    List<Integer> rowIndex = new ArrayList<Integer>();
    List<Integer> colIndex = new ArrayList<Integer>();

    int nonZero = 0;
    for (int r = 0; r < matrix.rows(); r++) {
      Vector row = matrix.getRow(r);
      if (!row.is(Vectors.ZERO_VECTOR)) {
         for (int c = 0; c < n; c++) {
           if (Math.abs(row.get(c)) > LinearAlgebra.EPS) {
             values.add(row.get(c));
             colIndex.add(c);
             nonZero++;
           }
        }
        if (rowIndex.isEmpty()) {
          rowIndex.add(0);
        }
        rowIndex.add(nonZero);
      }
    }
    Graph graph = new Graph(nodeNames,
        new double[]{5,8,3,6},
        new double[]{1,8,3,6},
        )
*/

/*
    CRSMatrix crsMatrix = new CRSMatrix(matrix);

    try {
      ByteArrayOutputStream out = new ByteArrayOutputStream();
      ObjectOutputStream oout = new ObjectOutputStream(out);
      crsMatrix.writeExternal(oout);
      oout.close();
      System.out.println(crsMatrix);
      System.out.println(out.size());
     // System.out.println(oout.size());


    } catch (IOException e) {
      e.printStackTrace();
    }

    //System.out.println(crsMatrix);*/
  }
}
