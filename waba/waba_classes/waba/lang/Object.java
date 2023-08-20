/* $Id: Object.java,v 1.2 2001/08/18 20:37:52 bornet Exp $

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

package java.lang;

/**
 * Object is the the base class for all objects.
 * <p>
 * The number of methods in this class is
 * small since each method added to this class is added to all other classes
 * in the system.
 * <p>
 * As with all classes in the waba.lang package, you can't reference the
 * Object class using the full specifier of waba.lang.Object.
 * The waba.lang package is implicitly imported.
 * Instead, you should simply access the Object class like this:
 * <pre>
 * Object obj = (Object)value;
 * </pre>
 */

public class Object
{
/** Returns the string representation of the object. */
public String toString()
	{
	return "";
	}
}
