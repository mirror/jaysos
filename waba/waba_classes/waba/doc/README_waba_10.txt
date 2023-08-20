
Waba Virtual Machine 1.0 Source
-------------------------------

Welcome to the source release of the Waba Virtual Machine and foundation
classes.

The source for the VM executable is in the 'vm' directory. The source for
the Waba foundation classes is in the 'classes' directory.

There are a couple documents explaining how to build the virtual machine
executable in the "How-to papers" section of wabasoft's web site. Basically,
you just use a C compiler to compile the .c code in the vm directory.

After compiling the VM executable, you'll want to build the foundation
classes located in the classes directory. This is just a matter
of compiling them with a java compiler and then packaging them up with
the warp program for PalmOS and WinCE platforms. For Windows NT, you can
just set your CLASSPATH to include the classes directory to run waba
programs.


Some notes on the Foundation Classes
------------------------------------

The source for the Waba foundation classes in the classes directory of this
distribution is different from the source in the WabaSDK. The source in the
WabaSDK is used to build programs that run with a JavaVM. The source in this
distribution is used to build the native foundation classes to be used
with a WabaVM executable.

If you accidentally set your CLASSPATH to include the WabaSDK foundation
classes instead of the classes in this distribution, you'll get some
class not found errors because the java classes are referenced by the
foundation classes that are part of the SDK.

If you are using Windows NT/98/2000 and you have compiled the foundation
classes, you can run waba programs by setting your CLASSPATH to include
the classes directory:

> set CLASSPATH=%CLASSPATH%;\wabavm-src.10\classes

Under the Win32 platform, the VM loads classes by looking the CLASSPATH.
You don't have to use warp to put them in a warp file.

Under WinCE, you'll need to build a waba.wrp that contains the waba foundation
classes. You can do that using the warp program contained in the WabaSDK
like this:

> cd classes
> warp c /c WABA waba waba\fx\*.class waba\io\*.class waba\lang\*.class
  waba\sys\*.class waba\ui\*.class waba\util\*.class

This will create a waba.wrp you should install in the \Program Files\waba
directory on the CE device.

For PalmOS, you need to build a waba.pdb and install it on the Palm device.
You build the waba.pdb with the same warp command line as above after
compiling the foundation classes. The warp command above builds both
a waba.wrp and waba.pdb.

Notice that the creator id was overridden to WABA in the warp command line
above.


Some notes on Porting to a new Platform
---------------------------------------

If you're thinking about porting the WabaVM to a new platform, look
at the nmport_a.c, nmport_b.c and nmport_c.c files in the vm directory.
To port the VM, you'll need to fill in those files for the new platform.


Some notes on adding new Native Functions
-----------------------------------------

To add new native functions, look at how the existing native functions
in the VM work. In the foundation classes, you'll see native functions
are declared as native like this:

public native void setDrawOp(int drawOp);

There is a table of all the native functions in the VM in waba.c that
contains a number and a pointer to the C function:

	// waba/fx/Graphics_setDrawOp_(I)V
	{ 2182095437, GraphicsSetDrawOp },

The number is a hash number computed from the name and parameters. To add
a new native function, first add the native function to the class. Here's
an example:

public class Test
{
public native void myFunc();
}

Now, write a waba program that references that class and then run that
program using the VM under Windows NT/98/2000.

When the program runs, and that native function is accessed, a window
will pop up that says:

** Native Method Missing:
// Test_myFunc_(I)V
{ 2182345437, func },

Cut and paste that signature into the waba.c native function table in
sorted order by the hash number. That's how you hook your native function
in the waba class up to the internal C function.

It's rather primitive, I know :-) Change 'func' to the name of your new
native function. For this example, we'll change it to TestMyFunc.

Now you need to add your native function into the VM for each of the
various platforms. Add something like the following to nmpalm_c.c and
nmwin32_c.c to get the native function into the Palm and Windows VMs:

static Var TestMyFunc(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

then add your code into the native function for each platform. You can
look at some of the existing native functions to see how variables are
passed and manipulated.


License
-------

    Copyright (C) 2000 Wabasoft
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    license.txt file containing the GNU General Public License for more details.

Trademark Notice
----------------

PalmPilot and PalmOS are trademarks of 3Com Corporation. Waba, WabaVM and
WabaSDK are trademarks of Wabasoft Corporation.
