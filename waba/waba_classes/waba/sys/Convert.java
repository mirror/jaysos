/* $Id: Convert.java,v 1.4 2001/08/18 20:37:52 bornet Exp $

Copyright (c) 1998, 1999 Wabasoft  All rights reserved.

This software is furnished under a license and may be used only in accordance
with the terms of that license. This software and documentation, and its
copyrights are owned by Wabasoft and are protected by copyright law.

THIS SOFTWARE AND REFERENCE MATERIALS ARE PROVIDED "AS IS" WITHOUT WARRANTY
AS TO THEIR PERFORMANCE, MERCHANTABILITY, FITNESS FOR ANY PARTICULAR PURPOSE,
OR AGAINST INFRINGEMENT. WABASOFT ASSUMES NO RESPONSIBILITY FOR THE USE OR
INABILITY TO USE THIS SOFTWARE. WABASOFT SHALL NOT BE LIABLE FOR INDIRECT,
SPECIAL OR CONSEQUENTIAL DAMAGES RESULTING FROM THE USE OF THIS PRODUCT.

WABASOFT SHALL HAVE NO LIABILITY OR RESPONSIBILITY FOR SOFTWARE ALTERED,
MODIFIED, OR CONVERTED BY YOU OR A THIRD PARTY, DAMAGES RESULTING FROM
ACCIDENT, ABUSE OR MISAPPLICATION, OR FOR PROBLEMS DUE TO THE MALFUNCTION OF
YOUR EQUIPMENT OR SOFTWARE NOT SUPPLIED BY WABASOFT.
*/

package waba.sys;

/**
 * Convert is used to convert between objects and basic types.
 */

public class Convert
{
private Convert()
	{
	}

/**
 * Converts the given String to an int. If the string passed is not a valid
 * integer, 0 is returned.
 */
public native static int toInt(String s);


/** Converts the given boolean to a String. */
public native static String toString(boolean b);


/** Converts the given char to a String. */
public native static String toString(char c);


/** Converts the given float to its bit representation in IEEE 754 format. */
public native static int toIntBitwise(float f);


/** Converts the given IEEE 754 bit representation of a float to a float. */
public native static float toFloatBitwise(int i);



/** Converts the given float to a String. */
public native static String toString(float f);

/** Converts the given double to a String. */
public native static String toString(double d);

/** Converts the given int to a String. */
   /** converts the string to upper case letters */
   public static String toUpperCase(String s)
   {
      char[] c = s.toCharArray();
      for (int i=0; i < c.length; i++)
         c[i] = toUpperCase(c[i]);
      return new String(c);
   }

   /** converts the string to lower case letters */
   public static String toLowerCase(String s)
   {
      char[] c = s.toCharArray();
      for (int i=0; i < c.length; i++)
         c[i] = toLowerCase(c[i]);
        return new String(c);
   }

   /** converts the char to lower case letter */
   public static char toLowerCase(char c)
   {
      if ('A' <= c && c <= 'Z')
         c += 32;
      return c;
   }

   /** converts the char to upper case letter */
   public static char toUpperCase(char c)
   {
      if ('a' <= c && c <= 'z')
         c -= 32;
      return c;
   }

public native static String toString(int i);

/** Converts the given long to a String. */
public native static String toString(long l);

}
