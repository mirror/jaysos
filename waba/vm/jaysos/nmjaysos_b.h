/* $Id: nmport_b.h,v 1.3 2001/09/02 07:24:03 monaka Exp $ */

#ifdef WITH_THREAD
/* PORT: the required functions by waba.c are follows.
 */
//*** Isao's Multithread implementation START ***
//*** This "_onThreadStart()" method is called from "start()" method.
Var ThreadStart(Var stack[]);

//*** This "_onThreadStop()" method is called from "stop()" method.
Var ThreadStop(Var stack[]);

//*** This "_onThreadSleep()" method is called from "sleep()" method.
Var ThreadSleep(Var stack[]);

//*** This "_onThreadWait()" method is called from "wait()" method.
Var ThreadWaitForSignal(Var stack[]);

Var ThreadSignalAll(Var stack[]);

//*** Isao's Multithread implementation END ***

//*** This "ThreadCurrentThread()" method is called from "curentThread()" method.
extern Var ThreadCurrentThread(Var stack[]);

void tickThread(void);
int registerAsSyncronized(WClassMethod* method);
void exitFromSyncronized(WClassMethod* method);
int enterMonitor(WObject obj);
int exitMonitor(WObject obj);


#endif /* WITH_THREAD */

