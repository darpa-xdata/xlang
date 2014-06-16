package xlang.java;

import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.lang.reflect.TypeVariable;
import java.util.Random;

/**
 * Created by go22670 on 6/16/14.
 *
 * boolean      Z
 byte         B
 char         C
 double       D
 float        F
 int          I
 long         J
 object       L
 short        S
 void         V
 array        [
 */
public class Xlang {
  static Random random = new Random();
  public static int sqr(int n) {
    return n * n;
  }

  public static int mult(int n, int m) {
    return n * m;
  }
  public static int strLen(String n) {
    return n.length();
  }
  public static int random() {
    return random.nextInt();
  }

  public static boolean nextBool() {
    return random.nextBoolean();
  }
  public static double nextDouble() {
    return random.nextDouble();
  }

  public static String [] getMethods() {
    Method[] declaredMethods = Xlang.class.getDeclaredMethods();
    String [] methods = new String[declaredMethods.length];
    int i = 0;
    for (Method m : Xlang.class.getDeclaredMethods()) {
      methods[i++] = m.getName();
    }
    return methods;
  }

  /**
   * boolean      Z
   byte         B
   char         C
   double       D
   float        F
   int          I
   long         J
   object       L
   short        S
   void         V
   array        [
   * @param method
   * @return
   */
  public static String getMethodDescriptor(String method) {
  //  Method[] declaredMethods = Xlang.class.getDeclaredMethods();
  //  String[] methods = new String[declaredMethods.length];
    int i = 0;
    for (Method m : Xlang.class.getDeclaredMethods()) {
      if (method.equalsIgnoreCase(m.getName())) {
        StringBuffer buffer = new StringBuffer();
        for (Type p : m.getGenericParameterTypes()) {
          System.out.println("\t1 param " + p);
          buffer.append(p.toString());
        }
        return "("+ buffer.toString() + ")" +
             m.getGenericReturnType();

      }
    }
    return "V";
  }


  public static String getReturnType(String method) {
    //  Method[] declaredMethods = Xlang.class.getDeclaredMethods();
    //  String[] methods = new String[declaredMethods.length];
  //  System.out.println("getReturnType for " + method);

    int i = 0;
    for (Method m : Xlang.class.getDeclaredMethods()) {
      if (method.equalsIgnoreCase(m.getName())) {

        String s = m.getGenericReturnType().toString();
       // System.out.println("got "+s);

        return
            s;

      }
    }
    return "V";
  }

  public static void main(String[] arg) {

    System.out.println("hello xlang!");

    for (Method m : Xlang.class.getDeclaredMethods()) {
      System.out.println("method " + m);

      String methodDescriptor = getMethodDescriptor(m.getName());
      System.out.println("\tmethodDescriptor " + methodDescriptor);



    }

  }
}
