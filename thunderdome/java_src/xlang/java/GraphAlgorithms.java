package xlang.java;

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
    double sin = Math.sin(deg);
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


  public static void invokeMega() {}
}
