/* $Id: HelloWorld.java,v 1.3 2002/01/25 17:24:17 cgrigis Exp $ */

package Applications;

// library inclusions
import java.lang.*;
import waba.sys.*;
import waba.sys.VmShell;

class HelloWorld {

    public static void main (String[] args) {
	VmShell.print("Hello ");
	VmShell.println("World!");
	VmShell.println("");

	VmShell.println("Number of arguments:" + args.length);

	for(int i=0; i<args.length; ++i) {
	    VmShell.println("  args["+i+"]="+'"'+args[i]+'"');
	}
    }  // method main

}  // class HelloWorld
