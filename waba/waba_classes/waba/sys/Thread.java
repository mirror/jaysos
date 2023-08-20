/* $Id: Thread.java,v 1.6 2001/12/23 03:36:58 isachan Exp $

Copyright (c) 2000 Wabasoft  All rights reserved.

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

/* 2001.08.06
 * This file is modified by Monaka (monamour@monaka.org)
 * enabled to use Runnable interface.
 * appended dummy code for yield.
 */

/*
Waba Thread:
<p>
It works pretty much like Java's Thread class, except with few cabiats...
<p>
On all platforms:
Everytime you use it, you have to write out as "waba.sys.Thread", instead of just "Thread".
This way, Java compiler doesn't get confused with java.lang.Thread.



<p>
On PalmOS:
Threads are implemented as "GREEN" threads which let you run threads
on platforms which doesn't support native multi-threading.
<p>
You have to ".stop()" it, when it's done.
<p>
It blocks everything if ".run()" method calls other time-consuming methods.
i.e. serial and socket communication.
Since it blocks everything else, you don't have to do any shared-resource protection.
<p>
The main thread scheduler (sounds very fancy, but it's very simple) is always running,
even there's no thread is running. So, it eats the battery power some more.
<p>
Also, the run() method is always called by the VM,
so the while(true){...} is not necessary within the run() method.



<p>
On WinCE & Win32:
Now native threading is implemented on WinCE & Win32 paltforms(11/15/2001).
<p>
Thus, the resolution of context switch becomes system dependent,
possibly slower than green thread implementation.
<p>
Also any shared-resources, i.e. Graphics, GUIs, Serial Port, Socket, etc.
need some kind of thread synchronization.
<br>
Sync methods / blocks and signal(for notify() in Java) / signalAll (notifyAll() in Java) are
provided for this purpose, but they are not tested at the time of writing (2001-11-20).
<p>
If you could provide me some detail information of the problem when it's encountered
(when and how or what caused the problem, and the trace of the situation leading up to that point),
I will try to fix as much as my free time alllows (as many other open-source project).  ;-(
*/

package waba.sys;
import waba.sys.Runnable;

public class Thread
{
    private Runnable target;

    /** Allocate a new thread object */
    public Thread(Runnable target) {
	this.target = target;
    }

    /** Allocate a new thread object */
    public Thread() {
	target = null;
    }

    //private native void _nativeCreate();

    /**
     * Causes this thread to starts execution.
     * After this function called, WabaVM calls run() method.
     * @also run()
     */
    public final native void start();

    /**
     * If this thread was constructed with the object Runnable inheritance,
     * it calls the Runnable object's run(). Otherwize it just returns and
     * does nothing.
     *
     * On PalmOS:
     * Once Threads are ".start()"ed,
     * the run() method is always called by the VM,
     * so the while(true){...} is not necessary within the run() method.
     */
    public void run() {
	if (target != null)
	    target.run();
    }

    /**
     * On PalmOS:
     * Causes this thread to stop execution.
     * This method is still necessary for green threads.
     */
    public final native void stop();

    /**
     * Causes this thread to sleep.
     * <p>
     * On PalmOS:
     * @param millisecond   The value is NOT based on mill-seconds,
     *                      but rather specifies how many times
     *                      a thread yields its turn to other threads.
     *<p>
     * i.e. when sleep counts are set to two different threads as threadA.sleep(1) and threadB.sleep(5),
     * Thread B gives up 5 turns to Thread A.
     * The implementation became like this, since there is no high-resolution timer
     * for some of the supported platforms (notably PalmOS).
     */
    public final native void sleep(int millisecond);

    /**
     * Causes currently executed thread to pause tempolary and to enable the another thread execution.
     * This is not implemented yet.
     */
    public static final /*native*/ void yield() {};

    public final native void waitForSignal();

    public final native void signalAll();

    public final native void signal();

    public static native Thread currentThread();

    public static native void setPriority(int priorityNumber);
}


