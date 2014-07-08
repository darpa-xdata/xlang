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
public class Graph {
  private String[] nodeNames = new String[0];
  private double[] values;
  private int [] rowIndex;
  private int [] colIndex;

  public Graph(int[] nodes, int[] edges) {
    double[] values = new double[edges.length];
    for (int j = 0; j < edges.length; j++) values[j] = 1;
    String[] names = new String[nodes.length-1];
    for (int j = 0; j < names.length; j++) names[j] = ""+j;

    this.nodeNames = names;
    this.values = values;
    this.rowIndex = nodes;
    this.colIndex = edges;
  }


  /**
   * Make a graph with arbitrary node names.
   * @param nodeNames
   * @param values
   * @param rowIndex
   * @param colIndex
   */
  public Graph(String[] nodeNames, double[] values, int[] rowIndex, int[] colIndex) {
    this.nodeNames = nodeNames;
    this.values = values;
    this.rowIndex = rowIndex;
    this.colIndex = colIndex;
  }

  public Graph(String[] nodeNames, Matrix matrix) {
    this.nodeNames = nodeNames;
    int n = nodeNames.length;
    List<Double> values = new ArrayList<Double>();
    List<Integer> rowIndex = new ArrayList<Integer>();
    List<Integer> colIndex = new ArrayList<Integer>();

    int nonZero = 0;
    for (int r = 0; r < matrix.rows(); r++) {
      Vector row = matrix.getRow(r);
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
      }
    }
    rowIndex.add(values.size());
    this.values   = ArrayUtils.toPrimitive(values.toArray(new Double[values.size()]));
    this.rowIndex = ArrayUtils.toPrimitive(rowIndex.toArray(new Integer[rowIndex.size()]));
    this.colIndex = ArrayUtils.toPrimitive(colIndex.toArray(new Integer[colIndex.size()]));
  }

  // recover original matrix
  public double[][] toMatrix() {
    int n = getRowIndex().length - 1;
    double [][] matrix = new double[n][n];
    for (int r = 0; r < n; r++) {
      int vs = getRowIndex()[r];
      int ve = getRowIndex()[r+1];
     // rowIndex[r];
      //int re = rs+1;
      for (int voffset  = vs; voffset < ve; voffset++) {
        double value = getValues()[voffset];
        int col = getColIndex()[voffset];
        matrix[r][col] = value;
      }
    }
    return matrix;
  }

  public String[] getNodeNames() {
    return nodeNames;
  }

  public double[] getValues() {
    return values;
  }

  public int[] getRowIndex() {
    return rowIndex;
  }

  public int[] getColIndex() {
    return colIndex;
  }

  public String toString() {
    StringBuilder builder = new StringBuilder();
    if (getNodeNames().length > 1) {
      builder.append("names [");
      for (String s : getNodeNames()) builder.append(s).append(",");
      builder.append("],\nvalues [");
    }
    else {
      builder.append("values [");
    }

    for (Double s : getValues()) builder.append(s).append(",");
    builder.append("],\nrow index [");

    for (Integer s : getRowIndex()) builder.append(s).append(",");
    builder.append("],\ncol index [");

    for (Integer s : getColIndex()) builder.append(s).append(",");
    builder.append("]");

    return builder.toString();
  }
}
