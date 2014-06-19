package xlang.java;

import org.apache.commons.lang3.ArrayUtils;
import org.la4j.LinearAlgebra;
import org.la4j.matrix.Matrix;
import org.la4j.vector.Vector;
import org.la4j.vector.Vectors;

import java.util.ArrayList;
import java.util.List;

/**
* Created by go22670 on 6/19/14.
*/
class Graph {
  private String[] nodeNames;
  private double[] values;
  private int [] rowIndex;
  private int [] colIndex;

  public Graph(String[] nodeNames, double[] values, int[] rowIndex, int[] colIndex) {
    this.nodeNames = nodeNames;
    this.values = values;
    this.rowIndex = rowIndex;
    this.colIndex = colIndex;
  }

  public Graph(String[] nodeNames, Matrix matrix) {
    //String[] nodeNames = {"one", "two", "three", "four"};
    this.nodeNames = nodeNames;
    int n = nodeNames.length;
    //CRSMatrix crsMatrix = new CRSMatrix(matrix);
    List<Double> values = new ArrayList<Double>();
    List<Integer> rowIndex = new ArrayList<Integer>();
    List<Integer> colIndex = new ArrayList<Integer>();

    int nonZero = 0;
    for (int r = 0; r < matrix.rows(); r++) {
      Vector row = matrix.getRow(r);
//      int prev = nonZero;
      rowIndex.add(nonZero);
      if (!row.is(Vectors.ZERO_VECTOR)) {
        for (int c = 0; c < n; c++) {
          double abs = Math.abs(row.get(c));
          if (abs > LinearAlgebra.EPS) {
            values.add(row.get(c));
            colIndex.add(c);
            nonZero++;
          }
        }
        //if (rowIndex.isEmpty()) {
        //  rowIndex.add(0);
        //}
       // rowIndex.add(nonZero);
      }
     // else {
     // }
    }
    rowIndex.add(values.size());
    this.values   = ArrayUtils.toPrimitive(values.toArray(new Double[values.size()]));
    this.rowIndex = ArrayUtils.toPrimitive(rowIndex.toArray(new Integer[rowIndex.size()]));
    this.colIndex = ArrayUtils.toPrimitive(colIndex.toArray(new Integer[colIndex.size()]));
  }

  // recover original matrix
  public double[][] toMatrix() {
    double [][] matrix = new double[nodeNames.length][nodeNames.length];
    for (int r = 0; r < rowIndex.length-1;r++) {
      int vs = rowIndex[r];
      int ve = rowIndex[r+1];
     // rowIndex[r];
      //int re = rs+1;
      for (int voffset  = vs; voffset < ve; voffset++) {
        double value = values[voffset];
        int col = colIndex[voffset];
        matrix[r][col] = value;
      }
    }
    return matrix;
  }

  public String toString() {
    StringBuilder builder = new StringBuilder();
    builder.append("[");
    for (String s : nodeNames) builder.append(s).append(",");
    builder.append("],\n[");

    for (Double s : values) builder.append(s).append(",");
    builder.append("],\n[");

    for (Integer s : rowIndex) builder.append(s).append(",");
    builder.append("],\n[");

    for (Integer s : colIndex) builder.append(s).append(",");
    builder.append("]");

    return builder.toString();

    //for (String s : nodeNames) builder.append(nodeNames).append(",");
    //return nodeNames + "\nvalues " + values + "\nrowIndex " +rowIndex + "\ncolIndex " +colIndex;
  }
}
