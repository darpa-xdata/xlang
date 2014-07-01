package xlang.java;

import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.List;
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

  public static boolean nextBool() {
    return random.nextBoolean();
  }
  public static double nextDouble() {
    return random.nextDouble();
  }
  public static String toUpper(String str) {
    return str.toUpperCase();
  }

  // not supported
  public static int howMany(List<String> strs) {
    return strs.size();
  }

  public static int howManyArr(String [] strs) {
   // for (int i = 0; i < strs.length; i++) System.out.println("#"+i + " : " + strs[i]);

    return strs.length;
  }

  public static void toUpperInOut(String [] inValues, String [] outValues) {
    for (int i = 0; i < inValues.length; i++) outValues[i] = inValues[i].toUpperCase();

  }

  // not supported
  public static List<String> toUpperList(List<String> strs) {
    List<String>ret = new ArrayList<String>();
    for (String s:strs) ret.add(s.toUpperCase());
    return ret;
  }

  // not supported
  public static int sum(List<Integer> nums) {
    int total = 0;
    for (Integer s:nums) total+=s;
    return total;
  }

  public static int sumArr(int [] nums) {
    int total = 0;
    for (int s:nums) {
      total+=s;
    }
    return total;
  }

  public static double sumDoubleArr(double [] nums) {
    double total = 0;
    for (double s:nums) {
      total+=s;
    }
    return total;
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


  public static String getReturnTypeInClass(String className, String method) {
    int i = 0;

    Class<?> xlangClass = null;
    try {
      xlangClass = Class.forName(className.replaceAll("\\/","\\."));
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
      return "No_Class_found";
    }
    for (Method m : xlangClass.getDeclaredMethods()) {
      if (method.equalsIgnoreCase(m.getName())) {

        String s = m.getGenericReturnType().toString();
        if (s.startsWith("class ")) {
          s = s.substring("class ".length());
          s = s.replaceAll("\\.","\\/");
        }

        return s;

      }
    }
    return "NoMethod";
  }


  public static String getReturnType(String method) {
    int i = 0;
    for (Method m : Xlang.class.getDeclaredMethods()) {
      if (method.equalsIgnoreCase(m.getName())) {

        String s = m.getGenericReturnType().toString();
         if (s.startsWith("class ")) {
           s = s.substring("class ".length());
           s = s.replaceAll("\\.","\\/");
         }

        return s;

      }
    }
    return "NoMethod";
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
