/* $Id: VmShell.java,v 1.2 2001/08/18 20:37:52 bornet Exp $

Copyright (c) 2000 SmartData

This software is furnished under a license and may be used only in accordance
with the terms of that license. This software and documentation, and its
copyrights are owned by SmartData and are protected by copyright law.

THIS SOFTWARE AND REFERENCE MATERIALS ARE PROVIDED "AS IS" WITHOUT WARRANTY
AS TO THEIR PERFORMANCE, MERCHANTABILITY, FITNESS FOR ANY PARTICULAR PURPOSE,
OR AGAINST INFRINGEMENT. SMARTDATA ASSUMES NO RESPONSIBILITY FOR THE USE OR
INABILITY TO USE THIS SOFTWARE. SMARTDATA SHALL NOT BE LIABLE FOR INDIRECT,
SPECIAL OR CONSEQUENTIAL DAMAGES RESULTING FROM THE USE OF THIS PRODUCT.

SMARTDATA SHALL HAVE NO LIABILITY OR RESPONSIBILITY FOR SOFTWARE ALTERED,
MODIFIED, OR CONVERTED BY YOU OR A THIRD PARTY, DAMAGES RESULTING FROM
ACCIDENT, ABUSE OR MISAPPLICATION, OR FOR PROBLEMS DUE TO THE MALFUNCTION OF
YOUR EQUIPMENT OR SOFTWARE NOT SUPPLIED BY SMARTDATA.
*/

package waba.sys;


/**
 * Vm contains various system level methods.
 * <p>
 * This class contains methods to copy arrays, obtain a timestamp,
 * sleep and get platform and version information.
 */

// NOTE:
// In the future, these methods may include getting unique object id's,
// getting object classes, sleep (for single threaded apps),
// getting amount of memory used/free, etc.
// The reason these methods should appear in this class and not somewhere
// like the Object class is because each method added to the Object class
// adds one more method to every object in the system.

public class VmShell
{
    private VmShell() {}
    public static native void print(String str);
    public static native void println(String str);
}
