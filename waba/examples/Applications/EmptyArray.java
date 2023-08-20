/* $Id: EmptyArray.java,v 1.3 2002/01/25 17:24:17 cgrigis Exp $ */

package Applications;

// library inclusions
import java.lang.*;
import waba.sys.*;
import waba.sys.VmShell;

class EmptyArray {

    private static final String a1[] = {"a", "b"};
    private static final String a2[] = {};

    public static void main (String[] args) {
	VmShell.println("String1 (2 el):");
	test(a1);

	VmShell.println("String1 (0 el):");
	test(a2);

    }  // method main


    private static void test(String[] args)
    {
	VmShell.println("String has "+args.length+" element(s)");
    }
}  // class
