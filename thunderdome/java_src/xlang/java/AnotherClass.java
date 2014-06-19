package xlang.java;

import java.lang.reflect.Method;
import java.util.Random;

/**
 Another example illustrating we can use another class.
 */
public class AnotherClass {
  static Random random = new Random();
  public static int sqr(int n) {
    return n * n;
  }

  public static double sin(double deg) {
    double sin = java.lang.Math.sin(deg);
    System.out.println("Xlang : sin of " + deg +  " = " + sin);
    return sin;
  }


  public static String getReturnType(String method) {
    int i = 0;
    for (Method m : AnotherClass.class.getDeclaredMethods()) {
      if (method.equalsIgnoreCase(m.getName())) {

        String s = m.getGenericReturnType().toString();

        return s;

      }
    }
    return "NoMethod";
  }

  public static void main(String[] arg) {
    System.out.println("hello from " +AnotherClass.class.getClass());

  }
}
