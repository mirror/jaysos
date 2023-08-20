/* $Id: waba.c,v 1.33 2002/02/11 08:59:47 cgrigis Exp $

Copyright (C) 1998, 1999, 2000 Wabasoft

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.
*/

/*

  IMPORTANT NOTICE: To compile this program, you need to define one of the
  folowing symbol :

      For WinCE: define WINCE
      For PalmOS: define PALMOS
      For Windows NT, 98, 2000 or similar: define WIN32

*/

/* all includes are made in waba.h */
#include "waba.h"

#if defined(NO_PLATFORM_DEFINED)
#error To compile, you need to define one platform.
#endif

//#if defined(WINCE)
//#define WIN32 1
//#endif

#if DEBUG
#  include <sys/time.h>
#  include <unistd.h>
#endif /* DEBUG */

/*

Welcome to the WabaVM source. This is the platform indepdenent code for the
interpreter, class parser and garbage collector. I like the code myself (of
course I wrote it, so that's not saying much) and hopefully it doesn't seem
too complicated. Actually, I'm hoping it reads rather easily given what it
does.

If you're looking through the source and wondering about the QUICKBIND stuff,
you should know that you can turn it off completely by removing all the code
with an #ifdef QUICKBIND around it. It's an optimization that speeds things
up quite a bit. So, if you're trying to figure out the code, ignore the
QUICKBIND stuff at first and then look at it later on.

The SMALLMEM define determines whether the VM uses a small or large memory
model (16 bit or 32 bit offsets). The default is SMALLMEM which means that if
progams use memory beyond a certain size, they jump to being slow since they
can't QUICKBIND any more. It still works since the QUICKBIND is adaptive, if
the offset fits, it uses it, if not, it doesn't use QUICKBIND.

This file should pretty much compile on any platform. To port the VM, you
create the 3 native method (nm) files. See the nmport_a.c if you are
interested in starting a port or to see how easy/difficult it would be.

Have a blast!

Rick Wild

*/

//
// OS SPECIFIC AREA CONTAINING RUNNERS
//

/*  #if defined(PALMOS) */
/*  #include "palm/nmpalm_b.c" */
/*  #elif defined(WIN32) */
/*  #include "win32/nmwin32_b.c" */
/*  #elif defined(LINUX) */
/*  #include "linux/nm_linux_b.c" */
/*  #endif */

//
// public functions
//

/*
 "I have three treasures that I keep and hold:
  one is mercy,
  the second is frugality,
  the third is not presuming to be at the head of the world.
  By reason of mercy, one can be brave.
  By reason of frugality, one can be broad.
  By not presuming to be at the head of the world,
  one can make your potential last."
 */

//
// global vars
//

int vmInitialized = 0;

int isApplication = FALSE;

// virtual machine stack
#if FIXED_VMSTACK_SIZE > 0
static Var vmStackArea[FIXED_VMSTACK_SIZE / sizeof(Var)];
#endif
#if FIXED_NMSTACK_SIZE > 0
static WObject nmStackArea[FIXED_NMSTACK_SIZE / sizeof(WObject)];
#endif
#if FIXED_CLASS_HEAP_SIZE > 0
static uchar classHeapArea[FIXED_CLASS_HEAP_SIZE];
#endif
#if FIXED_OBJECT_HEAP_SIZE > 0
static uchar objectHeapArea[(FIXED_OBJECT_HEAP_SIZE + 3) & ~3];
#endif
/**/
Var *vmStack;
uint32 vmStackSize; // in Var units
uint32 vmStackPtr;

// native method stack
WObject *nmStack;
uint32 nmStackSize; // in WObject units
uint32 nmStackPtr;

// class heap
uchar *classHeap;
uint32 classHeapSize;
uint32 classHeapUsed;
WClass *classHashList[CLASS_HASH_SIZE];

// error status
ErrorStatus vmStatus;

// pointer to String class (for performance)
WClass *stringClass;

// guich@200: now these vars are globals. (taken from executeMethod)
// needed for long/double support
int pushReturnedValue;         // see "goto methodreturn"
Var returnedValue;
Var returnedValue2;            // guich@200 - added 1 more returnedValue for 64 bits

#ifdef WITH_THREAD_NATIVE
//10/29/2001 - Isao F. Yamashita
Var*        getVmStack(void){return vmStack;}
WObject*    getNmStack(void){return nmStack;}
uint32      getVmStackSizeInBytes(void){return vmStackSize;}
uint32      getNmStackSizeInBytes(void){return nmStackSize;}
#endif

void VmInit(uint32 vmStackSizeInBytes, uint32 nmStackSizeInBytes,
		   uint32 _classHeapSize, uint32 _objectHeapSize)
{
  int status;
  uint32 i;
#ifdef PALMOS
  char *extra = NULL;
#endif
#ifdef SANITYCHECK
  static uchar floatTest[] = { 64, 160, 0, 0 };
  static uchar intTest1[] = { 0, 0, 255, 255 };
  static uchar intTest2[] = { 255, 255, 128, 8 };
  static uchar intTest3[] = { 255, 240, 189, 193 };
  static uchar intTest4[] = { 255, 254 };
  static uchar intTest5[] = { 39, 16 };
#endif

  if (vmInitialized)
    return;

  // NOTE: ordering is important here. The first thing we
  // need to do is initialize the global variables so if we
  // return not fully initialized, a VmFree() call will still
  // operate correctly. Also, its important not to statically
  // initialize them since VmInit() can be called after VmFree()
  // and if they were just statically intialized, they wouldn't
  // get reset.
  vmStack = NULL;
  vmStackSize = vmStackSizeInBytes / sizeof(Var);
  vmStackPtr = 0;

  nmStack = NULL;
  nmStackSize = nmStackSizeInBytes / sizeof(WObject);
  nmStackPtr = 0;

#ifndef SECURE_CLASS_HEAP
  classHeap = NULL;
#endif
  classHeapSize = _classHeapSize;
  classHeapUsed = 0;

  for (i = 0; i < CLASS_HASH_SIZE; i++)
    classHashList[i] = NULL;

  xmemzero((uchar *)&vmStatus, sizeof(vmStatus));

#ifdef SANITYCHECK
#ifdef WINCE
  // NOTE: This is required by the Catalog class
  if (sizeof(CEOID) > sizeof(void *))
    VmQuickError(ERR_SanityCheckFailed);
#endif
  // sanity checks
  if (sizeof(int16) != 2 || sizeof(float32) != 4 || sizeof(int32) != 4 ||
      sizeof(VMapValue) != 2 || sizeof(Var) != 4 || getInt32(intTest1) != 65535 ||
      getInt32(intTest2) != -32760 || getInt32(intTest3) != -999999 ||
      getInt16(intTest4) != -2 || getInt16(intTest5) != 10000 ||
      getFloat32(floatTest) != 5.0f)
    {
      VmQuickError(ERR_SanityCheckFailed);
      return;
    }

  // heap marking sanity check
  i = 100001;
  i |= 0x80000000;
  if (i & 0x80000000)
    i &= 0x7FFFFFFF;
  else
    i = -1;
  if (i != 100001)
    {
      VmQuickError(ERR_SanityCheckFailed);
      return;
    }
#endif

#ifdef PALMOS
  // NOTE: We allocate 2.5K before allocating anything else when running
  // under PalmOS. PalmOS has problems if you allocate the full amount of
  // memory in the dynamic heap since PalmOS also uses this memory when
  // you switch programs. If you don't leave a little extra, PalmOS
  // may crash and, at very least, won't let you switch programs.
  extra = (char *)xmalloc(2500);
  if (extra == NULL)
    {
      VmQuickError(ERR_CantAllocateMemory);
      return;
    }
#endif

#if (FIXED_CLASS_HEAP_SIZE > 0) && defined(SECURE_CLASS_HEAP)
#error "You can't use the macro FIXED_CLASS_HEAP_SIZE and SECURE_CLASS_HEAP at the same time"
#endif

  // allocate stacks and init
#if FIXED_VMSTACK_SIZE > 0
  vmStack = &vmStackArea[0];
  vmStackSize = FIXED_VMSTACK_SIZE / sizeof(Var);
#else
  vmStack = (Var*)xmalloc(vmStackSizeInBytes);
#endif

#if FIXED_NMSTACK_SIZE > 0
  nmStack = &vmStackArea[0];
  nmStackSize = FIXED_NMSTACK_SIZE / sizeof(WObject);
#else
  nmStack = (WObject*)xmalloc(nmStackSizeInBytes);
#endif

#if FIXED_CLASS_HEAP_SIZE > 0
  classHeap = &classHeapArea[0];
  classHeapSize = FIXED_CLASS_HEAP_SIZE;
#elif !defined(SECURE_CLASS_HEAP)
  classHeap = (uchar *)xmalloc(classHeapSize);
#endif

  if (vmStack == NULL || nmStack == NULL || classHeap == NULL)
    {
#if FIXED_VMSTACK_SIZE <= 0
      if (vmStack != NULL)
	xfree(vmStack);
#endif
#if FIXED_NMSTACK_SIZE <= 0
      if (nmStack != NULL)
	xfree(nmStack);
#endif
#if FIXED_CLASS_HEAP_SIZE <= 0 && !defined(SECURE_CLASS_HEAP)
      if (classHeap != NULL)
	xfree(classHeap);
#endif
#ifdef PALMOS
      xfree(extra);
#endif
      VmQuickError(ERR_CantAllocateMemory);
      return;
    }

  // zero out memory areas
  xmemzero((uchar *)vmStack, vmStackSizeInBytes);
  xmemzero((uchar *)nmStack, nmStackSizeInBytes);
  LOCK_CLASS_HEAP
    xmemzero((uchar *)classHeap, classHeapSize);
  UNLOCK_CLASS_HEAP

    status = initObjectHeap(_objectHeapSize);

#ifdef PALMOS
  xfree(extra);
#endif
  if (status != 0)
    {
      VmQuickError(ERR_CantAllocateMemory);
      return;
    }

#ifdef WITH_THREAD
  status = InitThread(-1);
  if (status != 0)
    {
      VmQuickError(ERR_CantAllocateMemory);
      return;
    }
#endif /* WITH_THREAD */

  vmInitialized = 1;
}

// copies one or two UtfStrings (by concatination) into a character
// buffer (null terminated) suitable for output
void printToBuf(char *buf, int maxbuf, UtfString *s1, UtfString *s2)
{
  uint16 len, len2;

  len = 0;
  if (s1 != NULL && s1->str != NULL)
    {
      len = s1->len;
      if (len >= maxbuf)
	len = maxbuf - 1;
      xstrncpy(buf, s1->str, len);
    }
  if (s2 != NULL && s2->str != NULL)
    {
      len2 = s2->len;
      if (len2 + len >= maxbuf)
	len2 = maxbuf - len - 1;
      xstrncpy(&buf[len], s2->str, len2);
      len += len2;
    }
  buf[len] = 0;
}

void VmError(uint16 errNum, WClass *iclass, UtfString *desc1, UtfString *desc2)
{
  WClass *wclass;
  WClassMethod *method;
  UtfString className, iclassName, methodName, methodDesc;

  // NOTE: Don't overwrite an existing error since it may be the
  // root cause of this error.
  if (vmStatus.errNum != 0)
    return;
  vmStatus.errNum = errNum;

  // get current class and method off stack
  if (vmStackPtr > 0)
    {
      wclass = (WClass *)vmStack[vmStackPtr - 1].refValue;
      method = (WClassMethod *)vmStack[vmStackPtr - 2].refValue;
    }
  else
    {
      wclass = 0;
      method = 0;
    }

  // output class and method name
  if (wclass)
    {
      className = getUtfString(wclass, wclass->classNameIndex);
      printToBuf(vmStatus.className, 40, &className, NULL);
    }
  if (method)
    {
      methodName = getUtfString(wclass, METH_nameIndex(method));
      methodDesc = getUtfString(wclass, METH_descIndex(method));
      printToBuf(vmStatus.methodName, 40, &methodName, &methodDesc);
    }

  // output additional error arguments (target class, desc, etc.)
  if (iclass)
    {
      iclassName = getUtfString(iclass, iclass->classNameIndex);
      printToBuf(vmStatus.arg1, 40, &iclassName, NULL);
    }
  printToBuf(vmStatus.arg2, 40, desc1, desc2);

#ifdef WIN32
#ifndef WINCE
  dumpStackTrace();
#endif
#endif
}

void VmQuickError(uint16 errNum)
{
  VmError(errNum, NULL, NULL, NULL);
}

void VmFree()
{
  if (!vmInitialized)
    return;
  // NOTE: The object heap is freed first since it requires access to
  // the class heap to call object destroy methods
  // destroy methods
  freeObjectHeap();
#if FIXED_VMSTACK_SIZE <= 0
  if (vmStack != NULL)
    xfree(vmStack);
#endif
#if FIXED_NMSTACK_SIZE <= 0
  if (nmStack != NULL)
    xfree(nmStack);
#endif
#if (FIXED_CLASS_HEAP_SIZE <= 0) && !defined(SECURE_CLASS_HEAP)
  if (classHeap != NULL)
    xfree(classHeap);
#endif

#ifdef WITH_THREAD_NATIVE
  DeleteCriticalSection(&vmThreadLock);
  DeleteCriticalSection(&nmThreadLock);
#endif

  vmInitialized = 0;
}

WObject VmStartApp(char *className)
{
  WObject mainWinObj;
  WClass *wclass, *vclass, *mainWinClass;
  WClassMethod *method;
  Var params[1];

#ifdef WITH_THREAD_NATIVE
  WVmParam wParams;
#endif

  if (!vmInitialized)
    return 0;
  stringClass = getClass(createUtfString("java/lang/String"));
  if (stringClass == NULL)
    return 0;
  wclass = getClass(createUtfString(className));
  if (wclass == NULL)
    return 0;
  // make sure its a MainWindow class
  mainWinClass = getClass(createUtfString("waba/ui/MainWindow"));
  if (!compatible(wclass, mainWinClass))
    return 0;
  // create MainWindow object
  mainWinObj = createObject(wclass);
  if (mainWinObj == 0)
    return 0;
  if (pushObject(mainWinObj) == -1)  // make sure it doesn't get GC'd
    return 0;
  params[0].obj = mainWinObj;
  // call MainWindow constructor
  method = getMethod(wclass, createUtfString("<init>"), createUtfString("()V"), NULL);
  if(method != NULL)
  {
#ifdef WITH_THREAD_NATIVE
    wParams.vclass = wclass;
    wParams.method = method;
    wParams.paramSize = 1;
    xmemmove(wParams.params, params, (sizeof(Var)*wParams.paramSize));
    executeMethod(&wParams);
#else
    executeMethod(wclass, method, params, 1);
#endif
  }

  // call onStart()
  method = getMethod(wclass, createUtfString("onStart"), createUtfString("()V"), &vclass);
  if(method != NULL)
  {
#ifdef WITH_THREAD_NATIVE
    wParams.vclass = vclass;
    wParams.method = method;
    wParams.paramSize = 1;
    xmemmove(wParams.params, params, (sizeof(Var)*wParams.paramSize));
    executeMethod(&wParams);
#else
    executeMethod(vclass, method, params, 1);
#endif
  }

  // NOTE: main window object is pushed on native stack to prevent it being GC'd
  return mainWinObj;
}

int VmStartApplication(char *className, int argc, char** argv)
{ // SD
  WObject appObj;
  WClass *wclass, *vclass/*, *mainWinClass*/;
  WClassMethod *method;
  Var params[16];
  //uint32 numParams;
  int16 mFlags;
  UtfString mname, mdesc;
  WObject stringArray, *strings;
  int i;
  Var v;

#ifdef WITH_THREAD_NATIVE
  WVmParam wParams;
#endif

  if (!vmInitialized)
    return 0;
  stringClass = getClass(createUtfString("java/lang/String"));
  if (stringClass == NULL)
    return 0;
  wclass = getClass(createUtfString(className));
  if (wclass == NULL)
    return 0;

  // create app object
  appObj = createObject(wclass);
  if (pushObject(appObj) == -1)  // make sure it doesn't get GC'd
    return 0;

  // call class constructor
  params[0].obj = appObj;
  method = getMethod(wclass, createUtfString("<init>"),
		     createUtfString("()V"), NULL);
    if(method != NULL)
    {
#ifdef WITH_THREAD_NATIVE
        wParams.vclass = wclass;
        wParams.method = method;
        wParams.paramSize = 1;
        xmemmove(wParams.params, params, (sizeof(Var)*wParams.paramSize));
        executeMethod(&wParams);
#else
        executeMethod(wclass, method, params, 1);
#endif
    }
/*
  else
    DPUTS("VmStartApplication(): No method <init>");
*/
  /*
  params[0].obj = mainWinObj;
    executeMethod(wclass, method, params, 1);
  */

  // call void main(String[] args)
  mname = createUtfString("main");
  mdesc = createUtfString("([Ljava/lang/String;)V");
  method = getMethod(wclass, mname, mdesc, &vclass);

  if (method == NULL){
    VmError(ERR_CantFindMethod, wclass, &mname, &mdesc);
    return 0;
  }

  // check method modifiers
  mFlags = METH_accessFlags(method);
  if ((mFlags & ACCESS_STATIC) != ACCESS_STATIC) {
    mname = createUtfString("public static main");
    mdesc = createUtfString("(String[] args)"),
    VmError(ERR_CantAccessAppClasses, wclass,
	    &mname,
	    &mdesc);
    return 0;
  }

    // convert arg list into string array
    stringArray = createArrayObject(1, argc);
    if (pushObject(stringArray) == -1) {
        VmError(ERR_CantAllocateMemory, wclass, &mname, &mdesc);
    }
    else
    {
    strings = (WObject *)WOBJ_arrayStart(stringArray);
    for (i=0; i<argc; ++i) {
        strings[i] = createString(argv[i]);
    }
    popObject(); // stringArray

    v.obj = stringArray;

#ifdef WITH_THREAD_NATIVE
    wParams.vclass = vclass;
    wParams.method = method;
    wParams.paramSize = argc;
    xmemmove(wParams.params, &v, (sizeof(Var)*wParams.paramSize));
    executeMethod(&wParams);
#else
    executeMethod(vclass, method, &v, argc);
#endif
  }

  return 1;
}

void VmStopApp(WObject mainWinObj)
{
    WClass *wclass, *vclass;
    WClassMethod *method;
    Var params[1];

#ifdef WITH_THREAD_NATIVE
  WVmParam wParams;
#endif

    if(!vmInitialized || mainWinObj == 0)
        return;
    // call onExit()
    wclass = WOBJ_class(mainWinObj);
    method = getMethod(wclass, createUtfString("onExit"), createUtfString("()V"), &vclass);
    if(method != NULL)
    {
        params[0].obj = mainWinObj;
#ifdef WITH_THREAD_NATIVE
        wParams.vclass = vclass;
        wParams.method = method;
        wParams.paramSize = 1;
        xmemmove(wParams.params, params, (sizeof(Var)*wParams.paramSize));
        executeMethod(&wParams);
#else
        executeMethod(vclass, method, params, 1);
#endif
    }

    popObject();
}

//
// Class Loader
//

uint32 genHashCode(UtfString name)
{
  uint32 value, i;

  value = 0;
  for (i = 0; i < name.len; i++)
    value += name.str[i];
  value = (value << 6) + name.len;
  return value;
}

uchar *allocClassPart(uint32 size)
{
  uchar *p;

  // align to 4 byte boundry
  size = (size + 3) & ~3;
  if (classHeapUsed + size > classHeapSize)
    {
      VmQuickError(ERR_OutOfClassMem);
      return NULL;
    }
  p = &classHeap[classHeapUsed];
  classHeapUsed += size;
  return p;
}

static char *specialClasses[] =
{
  "java/",
  "waba/lang/Object",
  "waba/lang/String",
  "waba/lang/StringBuffer"
};

WClass *getClass(UtfString className)
{
  WClass *wclass, *superClass;
  WClassMethod *method;
  UtfString iclassName;
  uint16 i, n, classIndex;
  uint32 classHash, size;
  uchar *p;

#ifdef WITH_THREAD_NATIVE
  WVmParam wParams;
#endif

  // look for class in hash list
  classHash = genHashCode(className) % CLASS_HASH_SIZE;
  wclass = classHashList[classHash];
  while (wclass != NULL)
    {
      iclassName = getUtfString(wclass, wclass->classNameIndex);
      if (className.len == iclassName.len &&
	  !xstrncmp(className.str, iclassName.str, className.len))
	break;
      wclass = wclass->nextClass;
    }
  if (wclass != NULL)
    return wclass;

  // NOTE: Classes mapping to those in the java/lang package can be
  // found in the path waba/lang. This is to avoid any confusion that
  // the java/lang classes exist in the base set of classes. Note that
  // we change the name only for loading purposes, not for hash table
  // lookup, etc. Also note that we aren't changing the name, just the
  // pointer which is on the stack so we aren't modifying the data
  // passed from the caller.
  if (className.len == 16 && !xstrncmp(className.str, specialClasses[0], 5))
    {
      if (className.str[15] == 't')
	// map java/lang/Object to waba/lang/Object
	className.str = specialClasses[1];
      else if (className.str[15] == 'g')
	// map java/lang/String to waba/lang/String
	className.str = specialClasses[2];
    }
  else if (className.len == 22 && !xstrncmp(className.str, specialClasses[0], 5) &&
	   className.str[21] == 'r')
    // map java/lang/Object to waba/lang/Object
    className.str = specialClasses[3];

  p = nativeLoadClass(className, NULL);
  if (p == NULL)
    {
      VmError(ERR_CantFindClass, NULL, &className, NULL);
      return NULL;
    }

  // NOTE: The garbage collector may run at any time during the
  // loading process so we need to make sure the class is valid
  // whenever an allocation may occur (static class vars, <clinit>, etc)
  // The various int variables will be initialzed to zero since
  // the entire class areas is zeroed out to start with.
  wclass = (WClass *)allocClassPart(sizeof(struct WClassStruct));
  if (wclass == NULL)
    return NULL;

  LOCK_CLASS_HEAP

    wclass->byteRep = p;

  // Initialization to NULL (NULL is required to be 0 - see sanity check) is
  // automatic since memory regions are zeroed out when created. So, can comment
  // out the following code since we don't have to NULL anything out, its
  // already 0
  //
  // wclass->superClasses = NULL;
  // wclass->numSuperClasses = 0;
  // wclass->attrib2 = NULL;
  // wclass->constantOffsets = NULL;
  // wclass->fields = NULL;
  // wclass->methods = NULL;
  // wclass->nextClass = NULL;
  // wclass->objDestroyFunc = NULL;

  // parse constants
  p += 8;
  wclass->numConstants = getUInt16(p);
  p += 2;
  if (wclass->numConstants != 0)
    {
      size = sizeof(ConsOffset) * wclass->numConstants;
      wclass->constantOffsets = (ConsOffset *)allocClassPart(size);

      if (wclass->constantOffsets == NULL)
	{
	  wclass->numConstants = 0;
	  goto unlockReturnNull;
	}
      for (i = 1; i < wclass->numConstants; i++)
	{
	  if (p - wclass->byteRep > MAX_consOffset)
	    {
	      VmError(ERR_ClassTooLarge, NULL, &className, NULL);
	      goto unlockReturnNull;
	    }
	  wclass->constantOffsets[i - 1] = p - wclass->byteRep;
	  p = loadClassConstant(wclass, i, p);
	  // after a long or double, next entry does not exist
	  if (CONS_tag(wclass, i) == CONSTANT_Long ||
	      CONS_tag(wclass, i) == CONSTANT_Double)
	    i++;
	}
    }

  // second attribute section
  wclass->attrib2 = p;
  p += 6;

  // assign class name
  wclass->classNameIndex = CONS_nameIndex(wclass, WCLASS_thisClass(wclass));

  // NOTE: add class to class list here so garbage collector can
  // find it during the loading process if it needs to collect.
  wclass->nextClass = classHashList[classHash];
  classHashList[classHash] = wclass;

  // load superclasses (recursive) here so we can resolve var
  // and method offsets in one pass
  superClass = NULL;
  classIndex = WCLASS_superClass(wclass);
  if (classIndex != 0)
    {
      UNLOCK_CLASS_HEAP
	superClass = getClassByIndex(wclass, classIndex);
      if (superClass == NULL)
	return NULL; // can't find superclass
      // fill in superclasses table
      n = superClass->numSuperClasses + 1;
#ifdef QUICKBIND
      if (n > MAX_superClassNum)
	{
	  VmQuickError(ERR_ClassTooLarge);
	  return NULL;
	}
#endif
      size = n * sizeof(WClass *);
      LOCK_CLASS_HEAP
	wclass->superClasses = (WClass **)allocClassPart(size);
      if (wclass->superClasses == NULL)
	{
	  VmQuickError(ERR_OutOfClassMem);
	  goto unlockReturnNull;
	}
      size = (n - 1) * sizeof(WClass *);
      xmemmove(wclass->superClasses, superClass->superClasses, size);
      wclass->superClasses[n - 1] = superClass;
      wclass->numSuperClasses = n;

      // inherit num of superclass variables to start
      wclass->numVars = superClass->numVars;
    }

  // skip past interfaces
  n = getUInt16(p);
  p += 2 + (n * 2);

  // parse fields
  wclass->numFields = getUInt16(p);
  p += 2;
  if (wclass->numFields != 0)
    {
      size = sizeof(WClassField) * wclass->numFields;
      wclass->fields = (WClassField *)allocClassPart(size);
      if (wclass->fields == NULL)
	{
	  wclass->numFields = 0;
	  goto unlockReturnNull;
	}
      for (i = 0; i < wclass->numFields; i++)
	p = loadClassField(wclass, &wclass->fields[i], p);
    }

  // parse methods
  wclass->numMethods = getUInt16(p);
  p += 2;
  size = sizeof(WClassMethod) * wclass->numMethods;
  if (size != 0)
    {
      wclass->methods = (WClassMethod *)allocClassPart(size);
      if (wclass->methods == NULL)
	{
	  wclass->numMethods = 0;
	  goto unlockReturnNull;
	}
      for (i = 0; i < wclass->numMethods; i++)
	p = loadClassMethod(wclass, &wclass->methods[i], p);
    }

#ifdef QUICKBIND
  // sort the methods and create the virtual method map for fast lookup
  if (createVirtualMethodMap(wclass) < 0)
    goto unlockReturnNull;
#endif

  // skip final attributes section

  // set hooks (before class init which might create/free objects of this type)
  setClassHooks(wclass);

  // if our superclass has a destroy func, we inherit it. If not, ours overrides
  // our base classes destroy func (the native destroy func should call the
  // superclasses)
  if (superClass != NULL && wclass->objDestroyFunc == NULL)
    wclass->objDestroyFunc = superClass->objDestroyFunc;

  UNLOCK_CLASS_HEAP

    // call static class initializer method if present
    method = getMethod(wclass, createUtfString("<clinit>"), createUtfString("()V"), NULL);
    if(method != NULL)
    {
#ifdef WITH_THREAD_NATIVE
        wParams.vclass = wclass;
        wParams.method = method;
        wParams.paramSize = 0;
        xmemmove(wParams.params, NULL, (sizeof(Var)*wParams.paramSize));
        executeMethod(&wParams);
#else
        executeMethod(wclass, method, NULL, 0);
#endif
    }
  return wclass;

 unlockReturnNull:
  UNLOCK_CLASS_HEAP
    return NULL;
}

#ifdef QUICKBIND

int createVirtualMethodMap(WClass *wclass)
{
  WClass *superClass;
  VirtualMethodMap *superVMap;
  uint32 size;
  uint16 i, n, nLow, nHigh, numSuperClasses;
  WClassMethod *method, tempMethod;
  VirtualMethodMap *vMap;
  uint16 methodHash[OVERRIDE_HASH_SIZE];

  // This method is responsible for filling in the VirtualMethodMap
  // structure to allow fast method lookup.
  //
  // It also sorts the method table so new virtual methods appear first,
  // overridden virtual methods second and non-virtual methods last.
  //
  // The method map contains the list of this classes superclasses
  // as well as an array that maps virtual method indicies to
  // class and method indicies to handle overridden methods.
  vMap = &wclass->vMethodMap;

  // The following code is commented out since memory regions are
  // zeroed out when created. The following code is commented out to
  // show this type of initialization is not required.
  //
  // vMap->mapValues = NULL;
  // vMap->mapSize = 0;
  // vMap->numVirtualMethods = 0;
  // vMap->numOverriddenMethods = 0;

  // sort methods so virtual appear first and non-virtual last
  n = wclass->numMethods;
  nLow = 0;
  nHigh = 0;
  while (nLow + nHigh < n)
    {
      method = &wclass->methods[nLow];
      // NOTE: the virtual section will not include <init> methods since
      // they should not be inherited
      if (((METH_accessFlags(method) & (ACCESS_PRIVATE | ACCESS_STATIC)) == 0) && !method->isInit)
	nLow++;
      else
	{
	  nHigh++;
	  // swap non-virtual to bottom
	  tempMethod = *method;
	  *method = wclass->methods[n - nHigh];
	  wclass->methods[n - nHigh] = tempMethod;
	}
    }
  vMap->numVirtualMethods = nLow;

  numSuperClasses = wclass->numSuperClasses;
  if (numSuperClasses == 0)
    {
      // Object class - no superclass map to inherit and no inherited methods
      // to override
      return 0;
    }

  superClass = wclass->superClasses[numSuperClasses - 1];
  superVMap = &superClass->vMethodMap;

  // create method map by copying superclass method map and inheriting
  // superclass virtual methods
  vMap->mapSize = superVMap->mapSize + superVMap->numVirtualMethods;
  if (vMap->mapSize + wclass->numMethods > MAX_methodNum + 1)
    {
      VmQuickError(ERR_ClassTooLarge);
      return -1;
    }
  size = vMap->mapSize * sizeof(VMapValue);
  vMap->mapValues = (VMapValue *)allocClassPart(size);
  if (vMap->mapValues == NULL)
    {
      VmQuickError(ERR_OutOfClassMem);
      return -1;
    }
  size = superVMap->mapSize * sizeof(VMapValue);
  xmemmove(vMap->mapValues, superVMap->mapValues, size);
  //add superclass #'s + method numbers into second portion
  n = 0;
  for (i = superVMap->mapSize; i < vMap->mapSize; i++)
    {
      vMap->mapValues[i].classNum = numSuperClasses - 1;
      vMap->mapValues[i].methodNum = n++;
    }

  // generate hash table of inherited methods allowing fast check
  // for overriden methods
  xmemzero(methodHash, sizeof(uint16) * OVERRIDE_HASH_SIZE);
  for (i = 0; i < vMap->mapSize; i++)
    {
      VMapValue mapValue;
      WClass *iclass;
      UtfString name, desc;
      uint16 hash;

      mapValue = vMap->mapValues[i];
      iclass = wclass->superClasses[mapValue.classNum];
      method = &iclass->methods[mapValue.methodNum];
      name = getUtfString(iclass, METH_nameIndex(method));
      desc = getUtfString(iclass, METH_descIndex(method));
      hash = (genHashCode(name) + genHashCode(desc)) % OVERRIDE_HASH_SIZE;
      methodHash[hash] = i + 1;
    }

  // in virtual method section, determine overrides and move overrides to
  // bottom of virtual section
  n = vMap->numVirtualMethods;
  nLow = 0;
  nHigh = 0;
  while (nLow + nHigh < n)
    {
      uint16 hash;
      int32 overrideIndex;
      UtfString name, desc;

      method = &wclass->methods[nLow];
      name = getUtfString(wclass, METH_nameIndex(method));
      desc = getUtfString(wclass, METH_descIndex(method));

      // look in hash table first
      hash = (genHashCode(name) + genHashCode(desc)) % OVERRIDE_HASH_SIZE;
      overrideIndex = methodHash[hash];
      if (!overrideIndex)
	{
	  nLow++;
	  continue;
	}
      overrideIndex -= 1;
      if (compareMethodNameDesc(wclass, (uint16)overrideIndex, name, desc))
	; // found it from hash
      else
	overrideIndex = getMethodMapNum(wclass, name, desc, SEARCH_INHERITED);
      if (overrideIndex == -1)
	nLow++;
      else
	{
	  // override - swap overridden method to bottom and add to map
	  nHigh++;
	  tempMethod = wclass->methods[nLow];
	  wclass->methods[nLow] = wclass->methods[n - nHigh];
	  wclass->methods[n - nHigh] = tempMethod;
	  vMap->mapValues[overrideIndex].classNum = numSuperClasses;
	  vMap->mapValues[overrideIndex].methodNum = n - nHigh;
	}
    }
  vMap->numVirtualMethods -= nHigh;
  vMap->numOverriddenMethods = nHigh;

#ifdef NEVER
  // NOTE: This is some code you can run under Windows to see what the
  // internal class structure/virtual method map looks like
  {
    VMapValue mapValue;
    int ii, methodIndex, superIndex;
    WClass *iclass;
    UtfString mname, mdesc;
    UtfString className;
    uint32 i;

    //AllocConsole();
    cprintf("className: ");
    className = getUtfString(wclass, wclass->classNameIndex);
    for (i = 0; i < className.len; i++)
      cprintf("%c", className.str[i]);
    cprintf("\n");
    cprintf("- nSuperClasses=%2d inherited=%2d new=%2d override=%2d total=%2d\n",
	    wclass->numSuperClasses, vMap->mapSize, vMap->numVirtualMethods, vMap->numOverriddenMethods,
	    wclass->numMethods);
    cprintf("- FULL METHOD MAP\n");

    for (ii = 0; ii < vMap->mapSize + wclass->numMethods; ii++)
      {
	if (ii < vMap->mapSize)
	  {
	    mapValue = vMap->mapValues[ii];
	    superIndex = mapValue.classNum;
	    if (superIndex < wclass->numSuperClasses)
	      iclass = wclass->superClasses[superIndex];
	    else
	      iclass = wclass;
	    methodIndex = mapValue.methodNum;
	    cprintf("- inherited %d/%d %d ", superIndex, wclass->numSuperClasses, methodIndex);
	  }
	else
	  {
	    iclass = wclass;
	    methodIndex = ii - vMap->mapSize;
	    cprintf("- this class %d ", methodIndex);
	  }
	method = &iclass->methods[methodIndex];
	if (methodIndex >= iclass->numMethods)
	  cprintf("*************************************\n");
	mname = getUtfString(iclass, METH_nameIndex(method));
	mdesc = getUtfString(iclass, METH_descIndex(method));
	cprintf("- [%d] %s %s\n", ii, mname.str, mdesc.str);
      }
  }
#endif

  return 0;
}

#endif

uchar *loadClassConstant(WClass *wclass, uint16 idx, uchar *p)
{
  p++;
  switch (CONS_tag(wclass, idx))
    {
    case CONSTANT_Utf8:
      p += 2;
      p += CONS_utfLen(wclass, idx);
      break;
    case CONSTANT_Integer:
    case CONSTANT_Float:
    case CONSTANT_Fieldref:
    case CONSTANT_Methodref:
    case CONSTANT_InterfaceMethodref:
    case CONSTANT_NameAndType:
      p += 4;
      break;
    case CONSTANT_Class:
    case CONSTANT_String:
      p += 2;
      break;
    case CONSTANT_Long:
    case CONSTANT_Double:
      p += 8;
      break;
    }
  return p;
}

#ifdef WITH_64BITS

// guich@200 - return 1 if the field is a D or a J
// ps: is there a better way to discover this?
static int fieldIs64wide(WClass *wclass, WClassField *field)
{
#ifdef WIN32
    //For some reason, this works perfect under Win32.
    UtfString fdesc = getUtfString(wclass, field->header[4]);
#else
    //For some reason, this doesn't work under Win32,
    //when some GUI controls are clicked.
    UtfString fdesc = getUtfString(wclass, FIELD_descIndex(field));
#endif
    return fdesc.str[0] == 'D' || fdesc.str[0] == 'J';
}

#endif  /* WITH_64BITS */

uchar *loadClassField(WClass *wclass, WClassField *field, uchar *p)
{
  uint32 i, bytesCount;
  uint16 attrCount, nameIndex;
  UtfString attrName;

  field->header = p;

  // compute offset of this field's variable in the object
  if (!FIELD_isStatic(field))
  {

    field->var.varOffset = wclass->numVars++;

#ifdef WITH_64BITS

    /*  guich@200 - added support for 64 bit vars. (just reusing attrName as temp var) */
    attrName = getUtfString(wclass, FIELD_descIndex(field));
    if (attrName.str[0] == 'D' || attrName.str[0] == 'J')
      field->var2.varOffset = wclass->numVars++;

#endif /* WITH_64BITS */

  } else
    field->var.staticVar.obj = 0;
  p += 6;
  attrCount = getUInt16(p);
  p += 2;

  for (i = 0; i < attrCount; i++)
    {
      nameIndex = getUInt16(p);
      attrName = getUtfString(wclass, nameIndex);
      p += 2;
      bytesCount = getUInt32(p);
      p += 4;
      if (FIELD_isStatic(field) && attrName.len == 13 && bytesCount == 2 &&
	  !xstrncmp(attrName.str, "ConstantValue", 13))
	field->var.staticVar = constantToVar(wclass, getUInt16(p));
      else
	; // MS Java has COM_MapsTo field attributes which we skip
      p += bytesCount;
    }
  return p;
}

Var constantToVar(WClass *wclass, uint16 idx)
{
  Var v;
  uint16 stringIndex;
/*
  printf( "--->0x%08x, %d/%d, 0x%08x\n",
	  wclass, idx, wclass->numConstants, wclass->constantOffsets[idx - 1] );
*/

  switch (CONS_tag(wclass, idx))
    {
    case CONSTANT_Integer:
      v.intValue = CONS_integer(wclass, idx);
      break;
    case CONSTANT_Float:
      v.floatValue = CONS_float(wclass, idx);
      break;
    case CONSTANT_String:
      stringIndex = CONS_stringIndex(wclass, idx);
      v.obj = createStringFromUtf(getUtfString(wclass, stringIndex));
      break;

    case CONSTANT_Long:
    case CONSTANT_Double:
      /* not used here. See double2vars() and int642vars() */

    default:
      v.obj = 0; // bad constant or not checked here
      break;
    }
  return v;
}

#if defined(DEBUG) && DEBUG
typedef struct DumpMethodStruct
{
  char *className;
  char *methodName;
  char *resultType;
  char *desc;
  char *flags;
} DumpMethod;


DumpMethod dumpMethod(UtfString cname, UtfString mname,
			     UtfString desc, int16 flags)
{
  uint32 n;
  char *c;
  char *buf;
  static char abuf[255];
  static char qbuf[255];
  static char rbuf[255];
  static char cresult[255];
  static char mresult[255];
  static char dresult[255];
  static char rresult[255];
  static char fresult[255];
  DumpMethod result;
  int len;
  int done = 0;

  abuf[0] = '\0';
  qbuf[0] = '\0';
  rbuf[0] = '\0';
  buf = abuf;
  len = 0;

  c = desc.str;

  if (*c++ != '(')
    goto argsDone;
  n = 0;
  while (!done) {
    switch (*c)	{
    case 'B':
      xstrcat(buf, "byte,"); n++; c++; break;
    case 'C':
      xstrcat(buf, "char,"); n++; c++; break;
    case 'F':
      xstrcat(buf, "float,"); n++; c++; break;
    case 'I':
      xstrcat(buf, "int,"); n++; c++; break;
    case 'S':
      xstrcat(buf, "short,"); n++; c++; break;
    case 'Z':
      xstrcat(buf, "boolean,"); n++; c++; break;
    case 'D':
      xstrcat(buf, "double,"); n++; c++; break;
    case 'J':
      xstrcat(buf, "long,"); n++; c++; break;
    case 'V':
      xstrcat(buf, "void,"); n++; c++; break;
    case 'L':
      c++;
      //strcat(buf, "class ");
      while (*c++ != ';') {
	static char tbuf[2];
	tbuf[0] = *(c-1);
	tbuf[1] = '\0';
	xstrcat(buf, tbuf);
      }
      xstrcat(buf, ",");
      n++;
      break;
    case '[':
      xstrcat(qbuf, "[]");
      c++;
      break;
    case ')':
      buf = rbuf;
      c++;
      break;
    case 0:
      done = 1;
      break;
    default:
      buf[xstrlen(buf)+1] = 0;
      buf[xstrlen(buf)] = *c;
      c++;
    }
    {
      int l = xstrlen(abuf);
      if (l!=len)  {
	if (qbuf[0]) {
	  xstrncpy(buf+xstrlen(buf) -1, qbuf, 255 - xstrlen(buf) -1);
	  qbuf[0] = '\0';
	  xstrcat(buf, ",");
	}
	len = xstrlen(abuf);
      }
    }
  }
 argsDone:
  if (rbuf[0] && rbuf[xstrlen(rbuf)-2]==',') rbuf[xstrlen(rbuf)-2] = '\0';
  if (abuf[0] && abuf[xstrlen(abuf)-1]==',') abuf[xstrlen(abuf)-1] = '\0';

  cresult[0] = '\0';
  mresult[0] = '\0';
  dresult[0] = '\0';
  rresult[0] = '\0';
  fresult[0] = '\0';

  result.className=cresult;
  result.methodName=mresult;
  result.resultType=rresult;
  result.desc=dresult;
  result.flags=fresult;

  // class name
  c = cresult;
  xstrncpy(c, cname.str, cname.len);
  c += cname.len; *c = 0;

  // methode name
  c = mresult;
  xstrncpy(c, mname.str, mname.len);
  c += mname.len; *c = 0;
  sprintf(c, "(%s)", abuf);

  // desc
  c = dresult;
  xstrncpy(c, desc.str, desc.len);
  c += desc.len; *c = 0;

  // flags
  c = fresult;
  if (flags&ACCESS_PUBLIC)
    xstrcat(c, "PUBLIC ");
  if (flags&ACCESS_PRIVATE)
    xstrcat(c, "PRIVATE ");
  if (flags&ACCESS_PROTECTED)
    xstrcat(c, "PROTECTED ");
  if (flags&ACCESS_STATIC)
    xstrcat(c, "STATIC ");
  if (flags&ACCESS_FINAL)
    xstrcat(c, "FINAL ");
  if (flags&ACCESS_SYNCHRONIZED)
    xstrcat(c, "SYNCHRONIZED ");
  if (flags&ACCESS_VOLATILE)
    xstrcat(c, "VOLATILE ");
  if (flags&ACCESS_TRANSIENT)
    xstrcat(c, "TRANSIENT ");
  if (flags&ACCESS_NATIVE)
    xstrcat(c, "NATIVE ");
  if (flags&ACCESS_INTERFACE)
    xstrcat(c, "INTERFACE ");
  if (flags&ACCESS_ABSTRACT)
    xstrcat(c, "ABSTRACT ");
  if (0&&xstrlen(c)>1)
    c[xstrlen(c)-2] = '\0';

  // return
  c = rresult;
  sprintf(c, "%s ", rbuf[0]?rbuf:"void");
  c += xstrlen(c);

  return result;
}
#endif

uchar *loadClassMethod(WClass *wclass, WClassMethod *method, uchar *p)
{
  uint32 i, j, bytesCount, codeCount;
  uint16 attrCount, attrNameIndex, numHandlers, numAttributes;
  int32 numParams;
  uchar *attrStart;
  UtfString attrName, methodName, methodDesc;

  method->header = p;
  p += 6;
  attrCount = getUInt16(p);
  p += 2;
  method->code.codeAttr = NULL;

  for (i = 0; i < attrCount; i++)
    {
      attrStart = p;
      attrNameIndex = getUInt16(p);
      p += 2;
      attrName = getUtfString(wclass, attrNameIndex);
      bytesCount = getUInt32(p);
      p += 4;
      if (attrName.len != 4 || xstrncmp(attrName.str, "Code", 4))
	{
	  p += bytesCount;
	  continue;
	}
      // Code Attribute
      method->code.codeAttr = attrStart;
      p += 4;
      codeCount = getUInt32(p);
      p += 4 + codeCount;

      // skip handlers and attributes
      numHandlers = getUInt16(p);
      p += 2;
      for (j = 0; j < numHandlers; j++)
	p += 8;
      numAttributes = getUInt16(p);
      p += 2;
      for (j = 0; j < numAttributes; j++)
	{
	  p += 2;
	  p += getUInt32(p) + 4;
	}
    }

  // determine numParams, isInit and returnsValue
  methodDesc = getUtfString(wclass, METH_descIndex(method));
  methodName = getUtfString(wclass, METH_nameIndex(method));
#if defined(DEBUG) && DEBUG
  {
    DumpMethod dm;
    dm = dumpMethod(getUtfString(wclass, wclass->classNameIndex),
		    methodName,
		    methodDesc,
		    METH_accessFlags(method));
    /*
    printf("+++ %s %s\n", dm.flags, dm. desc);
    printf("    %s %s.%s\n", dm.resultType, dm.className, dm. methodName);
    */
  }
#endif
  numParams = countMethodParams(methodDesc);
  if (numParams < 0)
    numParams = 0; // error
  method->numParams = (uint16)numParams;
  if (methodName.len > 2 && methodName.str[0] == '<' && methodName.str[1] == 'i')
    method->isInit = 1;
  else
    method->isInit = 0;
  if (methodDesc.str[methodDesc.len - 1] != 'V')
    method->returnsValue = 1;
  else
    method->returnsValue = 0;

  // resolve native functions
  if ((METH_accessFlags(method) & ACCESS_NATIVE) > 0) {
    method->code.nativeFunc = getNativeMethod(wclass, methodName, methodDesc);
    //printf("methodName=%s got nativeFunc %p\r\n", methodName.str,  method->code.nativeFunc);
  }

  return p;
}

int32 countMethodParams(UtfString desc)
{
  uint32 n;
  char *c;

  c = desc.str;
  if (*c++ != '(')
    return -1;
  n = 0;
  while (1)
    {
      switch (*c)
	{
	case 'B':
	case 'C':
	case 'F':
	case 'I':
	case 'S':
	case 'Z':
	  n++;
	  c++;
	  break;
	case 'D':
	case 'J':

#ifdef WITH_64BITS

	  // guich@200 long/double support
	  n+=2; // push 2 consecutive vars
	  c++;

#else

	  // longs/doubles not supported
	  return -1;

#endif   /* WITH_64BITS */

	  break;
	case 'L':
	  c++;
	  while (*c++ != ';')
	    ;
	  n++;
	  break;
	case '[':
	  c++;
	  break;
	case ')':
	  return n;
	default:
	  return -1;
	}
    }
}

//
// UtfString Routines
//

// *** Isao's simpler Unicode stuff START ***

//Consideration 1: using mbcstowc(), etc.
//Consideration 2: UCS -> UTF8, then UTF8<->UC<->MBCS.

//Returns Unicode string in "ucStr" and its size.
uint32 ucsToUc(uint16* ucsStr, uint16* ucStr, uint32 ucsStrSize)
{
    uint32 index, ucStrSize;

    for(index=0, ucStrSize=0; index < ucsStrSize; ucStrSize++)
	{
		//Java compiler encodes Unicode strings in non-standard way (UCS2 or UCS4).
		//So, we need to decode them according to thier range.
		if( (0xE0 & ucsStr[index]) == 0xE0)
		{
			// Unicode Characters which are mapped from '\u0800' to '\uFFFF' range are encoded in 3 bytes.
			ucStr[ucStrSize] = ((ucsStr[index]   & 0xF ) << 12) +
			                   ((ucsStr[index+1] & 0x3F) << 6)  +
			                    (ucsStr[index+2] & 0x3F);
			index += 3; //Advance the pointer by 3 bytes.
		}
		else if( (0xC0 & ucsStr[index]) == 0xC0)
		{
			// Unicode Characters which are mapped from '\u0080' to '\u07FF' range are encoded in 2 bytes.
			ucStr[ucStrSize] = ((ucsStr[index] & 0x1F) << 6) + (ucsStr[index+1] & 0x3F);
			index += 2;
		}
		else
		{
			// Unicode Characters which are mapped from '\u0001' to '\u007F' range are encoded in 1 byte.
			ucStr[ucStrSize] = (ucsStr[index] & 0x7F);
			index++;
		}
	}

	return ucStrSize;
}

//Returns MBCS string in "mbcsStr" and its size.
uint32 ucToMbcs(uint16* ucStr, char* mbcsStr, uint32 ucStrSize)
{
    uint32 ucStrIndex, mbcsStrIndex;

    for(ucStrIndex=0, mbcsStrIndex=0; ucStrIndex < ucStrSize; ucStrIndex++)
    {
        if(ucStr[ucStrIndex] <= 0x00FF)
        {
            mbcsStr[mbcsStrIndex++] = (ucStr[ucStrIndex] & 0x00FF)<<8;
        }
        else
        {
            mbcsStr[mbcsStrIndex++] = (ucStr[ucStrIndex] & 0xFF00)>>8;
            mbcsStr[mbcsStrIndex++] = (ucStr[ucStrIndex] & 0x00FF);
        }
    }

    return mbcsStrIndex;
}
// *** Isao's simpler Unicode stuff END ***

// *** Sean Luke's suggestion START ***
#define U_NULL (0)
#define STOP_AT_NULL_TERMINATOR (-1)
#define BAD_UTF8 (-1)

static uchar sbytes[64];

#ifdef WITH_UTF8_NEW
WObject createStringFromUtf(UtfString s)
{
	WObject obj, charArrayObj;
	uint16 *charStart;

    // String objects don't need to be null-terminated,
    // so we don't add 1 to len -- no need to tack on a null.
    int len = NumUnicodeCharsInUtf8(s.str,s.len);

	// create and fill char array
	charArrayObj = createArrayObject(5, len);
	if (!charArrayObj)
		return 0;
	if (pushObject(charArrayObj) == -1)
		return 0;
	charStart = (uint16 *)WOBJ_arrayStart(charArrayObj);

    if (ConvertUtf8ToUnicode(charStart,s.str,len) == BAD_UTF8)
        VmQuickError(ERR_BadUtfString);  // it's not good if this happens

	// create String object and set char array
	obj = createObject(stringClass);
	if (obj != 0)
		WOBJ_StringCharArrayObj(obj) = charArrayObj;
	popObject(); // charArrayObj
	return obj;
}
#else
WObject createStringFromUtf(UtfString s)
{
  WObject obj, charArrayObj;
  uint16 *charStart;
  uint32 i;

  // create and fill char array
  charArrayObj = createArrayObject(5, s.len);
  if (!charArrayObj)
    return 0;
  if (pushObject(charArrayObj) == -1)
    return 0;
  charStart = (uint16 *)WOBJ_arrayStart(charArrayObj);
  for (i = 0; i < s.len; i++)
    charStart[i] =(uint16)s.str[i];
  // create String object and set char array
  obj = createObject(stringClass);
  if (obj != 0)
    WOBJ_StringCharArrayObj(obj) = charArrayObj;
  popObject(); // charArrayObj
  return obj;
}
#endif

#ifdef WITH_UTF8_NEW
// NOTE: Only set STU_USE_STATIC if the string is temporary and will not be
// needed before stringToUtf is called again. The flags parameter is a
// combination of the STU constants.
UtfString stringToUtf(WObject string, int flags)
{
	UtfString s;
	WObject charArray;
	uint32 len, extra;
	uint16 *chars;
	char *bytes;
    int bytelen;
	int nullTerminate, useStatic;

	nullTerminate = flags & STU_NULL_TERMINATE;
	useStatic = flags & STU_USE_STATIC;
	s.str = "";
	s.len = 0;
	if (string == 0)
		return s;
	charArray = WOBJ_StringCharArrayObj(string);
	if (charArray == 0)
		return s;
	len = WOBJ_arrayLen(charArray);
	chars = (uint16 *)WOBJ_arrayStart(charArray);
    bytelen = LengthOfUtf8ForUnicode(chars,len);
	extra = 0;
	if (nullTerminate)
		extra = 1;
	if (useStatic && (bytelen + extra) <= 64)
		bytes = (char*)sbytes;  // no reason for sbytes to be uchar*
                //...but what the heck, why not keep it like it is...
	else
	{
		WObject byteArray;
		byteArray = createArrayObject(8, bytelen + extra);
		if (byteArray == 0)
			return s;
		bytes = (char*)WOBJ_arrayStart(byteArray);
	}

	// let's convert the unicode values into REAL UTF-8,
	// previously it was just high-byte ascii masquerading
	// as utf-8, argh.
    ConvertUnicodeToUtf8(chars,bytes,len);

	if (nullTerminate)
		bytes[bytelen] = 0;

	s.str = bytes;
	s.len = bytelen+extra;  // I *assume* you're supposed to include the terminator

	return s;
}
#else
// NOTE: Only set STU_USE_STATIC if the string is temporary and will not be
// needed before stringToUtf is called again. The flags parameter is a
// combination of the STU constants.
UtfString stringToUtf(WObject string, int flags)
{
  UtfString s;
  WObject charArray;
  uint32 i, len, extra;
  uint16 *chars;
  uchar *bytes;
  int nullTerminate, useStatic;

  nullTerminate = flags & STU_NULL_TERMINATE;
  useStatic = flags & STU_USE_STATIC;
  s.str = "";
  s.len = 0;
  if (string == 0)
    return s;
  charArray = WOBJ_StringCharArrayObj(string);
  if (charArray == 0)
    return s;
  len = WOBJ_arrayLen(charArray);
  extra = 0;
  if (nullTerminate)
    extra = 1;
  if (useStatic && (len + extra) <= 64)
    bytes = sbytes;
  else
    {
      WObject byteArray;

      byteArray = createArrayObject(8, len + extra);
      if (byteArray == 0)
	return s;
      bytes = (uchar *)WOBJ_arrayStart(byteArray);
    }
  chars = (uint16 *)WOBJ_arrayStart(charArray);
  for (i = 0; i < len; i++)
    bytes[i] = (uchar)chars[i];
  if (nullTerminate)
    bytes[i] = 0;
  s.str = (char *)bytes;
  s.len = len;
  return s;
}
#endif
//Returns the length of a UTF8 string for a given unicode string, where
//0x0 is encoded in 2 bytes per Java tradition.  len indicates the length of
//the unicode block to encode (in this case, null terminators do not signify
//the "end" of the string -- it's just a block off characters). Or if
//_length_ is <= STOP_AT_NULL_TERMINATOR, then the unicode string is read
//until and including the first null terminator.  Returns the length, in
//bytes, of the converted Utf8 string, including any possible ending null terminator.
int LengthOfUtf8ForUnicode(const unsigned short *unicode, int len)
{
    int length = 0;
    int base = 0;

    for (; len<=STOP_AT_NULL_TERMINATOR ? *unicode : base < len ;
		unicode++, base++)
    {
        int ch = *unicode;
        if (ch == 0) length += 2;
        else if (ch <= 0x007F) length++;
        else if (ch <= 0x07FF) length+=2;
        else length+=3;
    }

    if (len <= STOP_AT_NULL_TERMINATOR) // null terminated
    	length++;

    return length;
}

// Converts a unicode string into a UTF8 string, where 0x0 is encoded in 2
//bytes per Java tradition.  len indicates the length of the unicode block
//to encode (in this case, null terminators do not signify the "end" of the
//string -- it's just a block off characters). Or if _length_ is <=
//STOP_AT_NULL_TERMINATOR, then the unicode string is read until and
//including the first null terminator.  Returns the length, in bytes, of the
//converted Utf8 string, including any possible ending null terminator.
//
//The char string utf8 is presumed to have been allocated and to be large
//enough to accommodate the conversion.  You can get the necessary size for
//the buffer in utf8 by calling LengthOfUtf8ForUnicode(unicode, len);
int ConvertUnicodeToUtf8(const unsigned short *unicode, char *utf8, int len)
{
    int length = 0;
    int base = 0;

    for (; len<=STOP_AT_NULL_TERMINATOR ? *unicode : base < len ; unicode++, base++)
    {
        int ch = *unicode;

        if (ch == 0)
        {
             utf8[length++] = (char) 0xC0;
             utf8[length++] = (char) 0x80;
        }
        else if (ch <= 0x007F)
             utf8[length++] = (char) ch;
        else if (ch <= 0x07FF)
        {
             utf8[length++] = (char) ((char) 0xC0 | (char) ((ch >> 6) & 0x001F)); // bits 6-10
             utf8[length++] = (char) ((char) 0x80 | (char) (ch & 0x003F)); // bits 0-5
        }
        else
        {
             utf8[length++] = (char) ((char) 0xE0 | (char) ((ch >> 12) &
				0x000F));
             utf8[length++] = (char) ((char) 0x80 | (char) ((ch >> 6) &
				0x003F));
             utf8[length++] = (char) ((char) 0x80 | (char) (ch & 0x3F));
        }
    }
    if (len <= STOP_AT_NULL_TERMINATOR) // null terminated
	utf8[length++] = U_NULL;

    return length;
}

//Convert the Utf8 string of length len (bytes) pointed to by utf8 into
//its unicode representation pointed to by unicode. The unsigned short
//string unicode is presumed to have been allocated and to be large enough
//to accomodate the conversion.  You can get this size with
//NumUnicodeCharsInUtf8(*utf8,len); The resulting string *may* not be
//null-terminated (if the utf8 string wasn't null-terminated) so you might
//have to tack on a null-termination at the end if that's an issue for you.
//len is the length of the utf8 string in bytes, including any null
//terminator if it's there. Returns BAD_UTF8 if there were bad UTF
//characters in the string.  This is a serious error and shouldn't be ignored.
int ConvertUtf8ToUnicode(unsigned short *unicode, const char *utf8, int len)
{
    int i;
    unsigned short *ptr = unicode;
    for(i=0; i < len; i++, ptr++)
    {
        unsigned char ch = utf8[i];

        if ((ch & 0x80) == 0)
            *ptr = ch;
        else if ((ch & 0xE0) == 0xC0)
        {
            *ptr = ch & 0x1F;
            *ptr <<= 6;
            i++;
            ch = utf8[i] & 0x3F;
            *ptr += ch;
        }
        else if ((ch & 0xF0) == 0xE0)
        {
            *ptr = ch & 0x0F;
            *ptr <<= 6;
            i++;
            ch = utf8[i] & 0x3F;
            *ptr += ch;

            *ptr <<= 6;
            i++;
            ch = utf8[i] & 0x3F;
            *ptr += ch;
        }
        else
	    return BAD_UTF8;
    }

    // *ptr = U_NULL; 	//we don't do null termination any more

    return (ptr - unicode);
}

// Returns the number of unicode characters in the block of bytes of utf8
// up to length len, or BAD_UTF8 if the UTF8 is bad.
int NumUnicodeCharsInUtf8(const char *utf8, int len)
{
    int ptr=0;
    int i;

    for(i=0; i < len; i++, ptr++)
    {
        unsigned char ch = utf8[i];
        if ((ch & 0x80) == 0)
	    { }
        else if ((ch & 0xE0) == 0xC0)
        {
            i++;
        }
        else if ((ch & 0xF0) == 0xE0)
        {
            i+=2;
        }
        else
	    return BAD_UTF8;
    }
    return ptr;
}
// *** Sean Luke's suggestion END ***

UtfString createUtfString(char *buf)
{
  UtfString s;

  s.str = buf;
  s.len = xstrlen(buf);
  return s;
}

UtfString getUtfString(WClass *wclass, uint16 idx)
{
  UtfString s;

  if (idx >= 1 && CONS_tag(wclass, idx) == CONSTANT_Utf8)
    {
      s.str = (char *)CONS_utfStr(wclass, idx);
      s.len = CONS_utfLen(wclass, idx);
    }
  else
    {
      s.str = "";
      s.len = 0;
    }
  return s;
}

//
// Object Routines
//

WObject createObject(WClass *wclass)
{
  WObject obj;

  if (wclass == NULL)
    return 0;
  if ((WCLASS_accessFlags(wclass) & ACCESS_ABSTRACT) > 0)
    return 0; // interface or abstract class
  obj = allocObject(WCLASS_objectSize(wclass));
  if (obj == 0)
    return 0;
  WOBJ_class(obj) = wclass;
  return obj;
}

int32 arrayTypeSize(int32 type)
{
  switch (type)
    {
    case 1:  // object
    case 2:  // array
      return 4;
    case 4: // boolean
    case 8: // byte
      return 1;
    case 5:  // char
    case 9:  // short
      return 2;
    case 6:  // float
    case 10: // int
      return 4;
    case 7:  // double
    case 11: // long
      return 8;
    }
  return 0;
}

int32 arraySize(int32 type, int32 len)
{
  int32 typesize, size;

  typesize = arrayTypeSize(type);
  size = (3 * sizeof(Var)) + (typesize * len);
  // align to 4 byte boundry
  size = (size + 3) & ~3;
  return size;
}

WObject createArrayObject(int32 type, int32 len)
{
  WObject obj;

  obj = allocObject(arraySize(type, len));
  if (len < 0 || obj == 0)
    return 0;
  // pointer to class is NULL for arrays
  WOBJ_class(obj) = NULL;
  WOBJ_arrayType(obj) = type;
  WOBJ_arrayLen(obj) = len;
  return obj;
}

uint16 arrayType(char c)
{
  switch(c)
    {
    case 'L': // object
      return 1;
    case '[': // array
      return 2;
    case 'Z': // boolean
      return 4;
    case 'B': // byte
      return 8;
    case 'C': // char
      return 5;
    case 'S': // short
      return 9;
    case 'F': // float
      return 6;
    case 'I': // int
      return 10;
    case 'D': // double
      return 7;
    case 'J': // long
      return 11;
    }
  return 0;
}

WObject createMultiArray(int32 ndim, char *desc, Var *sizes)
{
  WObject arrayObj, subArray, *itemStart;
  int32 i, len, type;

  len = sizes[0].intValue;
  type = arrayType(desc[0]);
  arrayObj = createArrayObject(type, len);
  if (len < 0 || !arrayObj)
    return 0;
  if (type != 2 || ndim <= 1)
    return arrayObj;
  // NOTE: it is acceptable to push only the "upper"
  // array objects and not the most derived subarrays
  // because if the array is only half filled and we gc,
  // the portion that is filled will still be found since
  // its container was pushed
  if (pushObject(arrayObj) == -1)
    return 0;
  // create subarray (recursive)
  for (i = 0; i < len; i++)
    {
      // NOTE: we have to recompute itemStart after calling createMultiArray()
      // since calling it can cause a GC to occur which moves memory around
      subArray = createMultiArray(ndim - 1, desc + 1, sizes + 1);
      itemStart = (WObject *)WOBJ_arrayStart(arrayObj);
      itemStart[i] = subArray;
    }
  popObject();
  return arrayObj;
}

WObject createString(char *buf)
{
  return createStringFromUtf(createUtfString(buf));
}

int arrayRangeCheck(WObject array, int32 start, int32 count)
{
  int32 len;

  if (array == 0 || start < 0 || count < 0)
    return 0;
  len = WOBJ_arrayLen(array);
  if (start + count > len)
    return 0;
  return 1;
}

Var copyArray(Var stack[])
{
  Var v;
  WObject srcArray, dstArray;
  int32 srcStart, dstStart, len, srcType, typeSize;
  uchar *srcPtr, *dstPtr;

  v.intValue = 0;
  srcArray = stack[0].obj;
  srcStart = stack[1].intValue;
  dstArray = stack[2].obj;
  dstStart = stack[3].intValue;
  len = stack[4].intValue;
  if (srcArray == 0 || dstArray == 0)
    {
      VmQuickError(ERR_NullArrayAccess);
      return v;
    }
  // ensure both src and dst are arrays
  if (WOBJ_class(srcArray) != NULL || WOBJ_class(dstArray) != NULL)
    return v;
  // NOTE: This is not a full check to see if the two arrays are compatible.
  // Any two arrays of objects are compatible according to this check
  // see also: compatibleArray()
  srcType = WOBJ_arrayType(srcArray);
  if (srcType != WOBJ_arrayType(dstArray))
    return v;
  // check ranges
  if (arrayRangeCheck(srcArray, srcStart, len) == 0 ||
      arrayRangeCheck(dstArray, dstStart, len) == 0)
    {
      VmQuickError(ERR_IndexOutOfRange);
      return v;
    }
  typeSize = arrayTypeSize(srcType);
  srcPtr = (uchar *)WOBJ_arrayStart(srcArray) + (typeSize * srcStart);
  dstPtr = (uchar *)WOBJ_arrayStart(dstArray) + (typeSize * dstStart);
  xmemmove((uchar *)dstPtr, (uchar *)srcPtr, len * typeSize);
  v.intValue = 1;
  return v;
}

WClassField *getField(WClass *wclass, UtfString name, UtfString desc)
{
  WClassField *field;
  UtfString fname, fdesc;
  uint16 i;

  for (i = 0; i < wclass->numFields; i++)
    {
      field = &wclass->fields[i];
      fname = getUtfString(wclass, FIELD_nameIndex(field));
      fdesc = getUtfString(wclass, FIELD_descIndex(field));
      if (name.len == fname.len &&
	  desc.len == fdesc.len &&
	  !xstrncmp(name.str, fname.str, name.len) &&
	  !xstrncmp(desc.str, fdesc.str, desc.len))
	return field;
    }
  VmError(ERR_CantFindField, wclass, &name, &desc);
  return NULL;
}

WClass *getClassByIndex(WClass *wclass, uint16 classIndex)
{
  UtfString className;
  WClass *targetClass;
#ifdef QUICKBIND
  ConsOffset offset;
  uint32 off32;

  offset = CONS_offset(wclass, classIndex);
  if (offset & CONS_boundBit)
    return (WClass *)(classHeap + (offset & CONS_boundOffsetMask));
#endif
  className = getUtfString(wclass, CONS_nameIndex(wclass, classIndex));
  if (className.len > 1 && className.str[0] == '[')
    return NULL; // arrays have no associated class
  targetClass = getClass(className);
  if (targetClass == NULL)
    return NULL;
#ifdef QUICKBIND
  // adaptive quickbind - bind to pointer only if pointer fits in the offset
  off32 = (uchar *)targetClass - classHeap;
  if (off32 <= MAX_consOffset)
    {
      LOCK_CLASS_HEAP
	CONS_offset(wclass, classIndex) = CONS_boundBit | off32;
      UNLOCK_CLASS_HEAP
	}
#endif
  return targetClass;
}

WClassField *getFieldByIndex(WClass *wclass, uint16 fieldIndex)
{
  WClassField *field;
  WClass *targetClass;
  uint16 classIndex, nameAndTypeIndex;
  UtfString fieldName, fieldDesc;
#ifdef QUICKBIND
  ConsOffset offset;
  uint32 off32;

  offset = CONS_offset(wclass, fieldIndex);
  if (offset & CONS_boundBit)
    return (WClassField *)(classHeap + (offset & CONS_boundOffsetMask));
#endif
  classIndex = CONS_classIndex(wclass, fieldIndex);
  targetClass = getClassByIndex(wclass, classIndex);
  if (targetClass == NULL)
    return NULL;
  nameAndTypeIndex = CONS_nameAndTypeIndex(wclass, fieldIndex);
  fieldName = getUtfString(wclass, CONS_nameIndex(wclass, nameAndTypeIndex));
  fieldDesc = getUtfString(wclass, CONS_typeIndex(wclass, nameAndTypeIndex));
  field = getField(targetClass, fieldName, fieldDesc);
  if (field == NULL)
    return NULL;
#ifdef QUICKBIND
  // adaptive quickbind - bind to pointer only if pointer fits in the offset
  off32 = (uchar *)field - classHeap;
  if (off32 <= MAX_consOffset)
    {
      LOCK_CLASS_HEAP
	CONS_offset(wclass, fieldIndex) = CONS_boundBit | off32;
      UNLOCK_CLASS_HEAP
	}
#endif
  return field;
}

//
// Method Routines
//

#ifdef QUICKBIND

int compareMethodNameDesc(WClass *wclass, uint16 mapNum, UtfString name, UtfString desc)
{
  UtfString mname, mdesc;
  WClassMethod *method;
  WClass *iclass;

  method = getMethodByMapNum(wclass, &iclass, mapNum);
  mname = getUtfString(iclass, METH_nameIndex(method));
  mdesc = getUtfString(iclass, METH_descIndex(method));
  if (name.len == mname.len &&
      desc.len == mdesc.len &&
      !xstrncmp(name.str, mname.str, name.len) &&
      !xstrncmp(desc.str, mdesc.str, desc.len))
    return 1;
  return 0;
}

int32 getMethodMapNum(WClass *wclass, UtfString name, UtfString desc, int searchType)
{
  VirtualMethodMap *vMap;
  uint16 start, end, i;

  if( wclass == NULL ) {

    /* no wclass given => can't search */
    return -1;

  }

  vMap = &wclass->vMethodMap;
  if (searchType == SEARCH_ALL)
    {
      start = 0;
      end = vMap->mapSize + wclass->numMethods;
    }
  else if (searchType == SEARCH_INHERITED)
    {
      start = 0;
      end = vMap->mapSize;
    }
  else // SEARCH_THISCLASS
    {
      start = vMap->mapSize;
      end = vMap->mapSize + wclass->numMethods;
    }
  for (i = start; i < end; i++)
    {
      if (compareMethodNameDesc(wclass, i, name, desc))
	return i;
    }
  return -1;
}

WClassMethod *getMethodByMapNum(WClass *wclass, WClass **vclass, uint16 mapNum)
{
  VirtualMethodMap *vMap;
  VMapValue mapValue;
  uint16 superIndex, methodIndex;

  if( wclass == NULL ) {

    /* no wclass given => can't search */
    return NULL;

  }

  vMap = &wclass->vMethodMap;
  if (mapNum < vMap->mapSize)
    {
      // inherited or overridden method
      mapValue = vMap->mapValues[mapNum];
      superIndex = mapValue.classNum;
      if (superIndex < wclass->numSuperClasses)
	wclass = wclass->superClasses[superIndex];
      methodIndex = mapValue.methodNum;
    }
  else
    methodIndex = mapNum - vMap->mapSize;
  if (vclass != NULL)
    *vclass = wclass;
  return &wclass->methods[methodIndex];
}

WClassMethod *getMethod(WClass *wclass, UtfString name, UtfString desc, WClass **vclass)
{
  int searchType;
  int32 mapNum;

  if (vclass != NULL)
    searchType = SEARCH_ALL;
  else
    searchType = SEARCH_THISCLASS;
  mapNum = getMethodMapNum(wclass, name, desc, searchType);
  if (mapNum < 0)
    return NULL;
  return getMethodByMapNum(wclass, vclass, (uint16)mapNum);
}

#else

// vclass is used to return the class the method was found in
// when the search is virtual (when a vclass is given)
WClassMethod *getMethod(WClass *wclass, UtfString name, UtfString desc, WClass **vclass)
{
  WClassMethod *method;
  UtfString mname, mdesc;
  uint32 i, n;

  if( wclass == NULL ) {

    /* no wclass given => can't search */
    return NULL;

  }

  n = wclass->numSuperClasses;
  while (1)
    {
      for (i = 0; i < wclass->numMethods; i++)
	{
	  method = &wclass->methods[i];
	  mname = getUtfString(wclass, METH_nameIndex(method));
	  mdesc = getUtfString(wclass, METH_descIndex(method));
	  if (name.len == mname.len &&
	      desc.len == mdesc.len &&
	      !xstrncmp(name.str, mname.str, name.len) &&
	      !xstrncmp(desc.str, mdesc.str, desc.len))
	    {
	      if (vclass)
		*vclass = wclass;
	      return method;
	    }
	}
      if (!vclass)
	break; // not a virtual lookup or no superclass
      if (n == 0)
	break;
      // look in superclass
      wclass = wclass->superClasses[--n];
    }
  return NULL;
}
#endif

// return 1 if two classes are compatible (if wclass is compatible
// with target). this function is not valid for checking to see if
// two arrays are compatible (see compatibleArray()).
// see page 135 of the book by Meyers and Downing for the basic algorithm
int compatible(WClass *source, WClass *target)
{
  int targetIsInterface;
  uint32 i, n;

  if (!source || !target)
    return 0; // source or target is array
  targetIsInterface = 0;
  if (WCLASS_isInterface(target))
    targetIsInterface = 1;
  n = source->numSuperClasses;
  while (1)
    {
      if (targetIsInterface)
	{
	  for (i = 0; i < WCLASS_numInterfaces(source); i++)
	    {
	      uint16 classIndex;
	      WClass *interfaceClass;

	      classIndex = WCLASS_interfaceIndex(source, i);
	      interfaceClass = getClassByIndex(source, classIndex);
				// NOTE: Either one of the interfaces in the source class can
				// equal the target interface or one of the interfaces
				// in the target interface (class) can equal one of the
				// interfaces in the source class for the two to be compatible
	      if (interfaceClass == target)
		return 1;
	      if (compatible(interfaceClass, target))
		return 1;
	    }
	}
      else if (source == target)
	return 1;
      if (n == 0)
	break;
      // look in superclass
      source = source->superClasses[--n];
    }
  return 0;
}

int compatibleArray(WObject obj, UtfString arrayName)
{
  WClass *wclass;

  wclass = WOBJ_class(obj);
  if (wclass != NULL)
    return 0; // source is not array

  // NOTE: this isn't a full check to see if the arrays
  // are the same type. Any two arrays of objects (or other
  // arrays since they are objects) will test equal here.
  if (WOBJ_arrayType(obj) != arrayType(arrayName.str[1]))
    return 0;
  return 1;
}

//
// Memory Management
//

// Here's the garbage collector. I implemented the mark and sweep below
// after testing out a few different ones and reading:
//
// Garbage Collection, Algorithms for Automatic Dynamic Memory Management
// by Richard Jones and Rafael Lins
//
// which is an excellent book. Also, this collector has gone through a
// lot of testing. It runs when the system is completely out of memory
// which can happen at any time.. for example during class loading.
//
// To test it out, tests were run where 1000's of random objects were
// loaded, constructed and random methods called on them over some
// period of days. This found a couple subtle bugs that were
// fixed like when the garbage collector ran in the middle of array
// allocation and moved pointers around from under the array allocator
// code (those have all been fixed).
//
// The heap is comprised of Hos objects (an array) that grows from
// the "right" of object memory and objects that take up the space on
// on the "left" side. The Hos array keeps track of where the objects
// are on the left.
//
// The Hos structure (strange, but aptly named) is used to keep
// track of handles (pointers to memory locations), order (order
// of handles with respect to memory) and temporary items (used
// during the scan phase).
//
// The 3 items in the Hos structure do not relate to each other. They
// are each a part of 3 conceptually distinct arrays that grow
// from the right of the heap while the objects grow from the left.
// So, when the Hos array is indexed, it is always negative (first
// element is 0, next is -1, next is -2, etc).

typedef struct
{
  Var *ptr;
  uint32 order;
  uint32 temp;
} Hos;

// NOTE: The total amount of memory used up at any given
// time in the heap is: objectSize + (numHandles * sizeof(Hos))

typedef struct
{
  Hos *hos; // handle, order and scan arrays (interlaced)
  uint32 numHandles;
  uint32 numFreeHandles;
  uchar *mem;
  uint32 memSize; // total size of memory (including free)
  uint32 objectSize; // size of all objects in heap
} ObjectHeap;

static ObjectHeap heap;

#define FIRST_OBJ 2244

#define VALID_OBJ(o) (o > FIRST_OBJ && o <= FIRST_OBJ + heap.numHandles)

// NOTE: this method is only for printing the status of memory
// and can be removed. Also note, there is no such thing as
// the "amount of free memory" because of garbage collection.
uint32 getUnusedMem()
{
  return heap.memSize - (heap.objectSize + (heap.numHandles * sizeof(Hos)));
}

int initObjectHeap(uint32 heapSize)
{
  // NOTE: we must intiailize all the variables since after
  // a freeObjectHeap() we get called again
  heap.numHandles = 0;
  heap.numFreeHandles = 0;
  heap.memSize = heapSize;

#if FIXED_OBJECT_HEAP_SIZE > 0
  // align to 4 byte boundry for correct alignment of the Hos array
  heap.memSize = sizeof(objectHeapArea);

  // allocate and zero out memory region
  heap.mem = &objectHeapArea[0];
#else
  // align to 4 byte boundry for correct alignment of the Hos array
  heap.memSize = (heap.memSize + 3) & ~3;

  // allocate and zero out memory region
  heap.mem = (uchar *)xmalloc(heap.memSize);
  if (heap.mem == NULL)
    return -1;
#endif

  xmemzero(heap.mem, heap.memSize);
  heap.hos = (Hos *)(&heap.mem[heap.memSize - sizeof(Hos)]);
  heap.objectSize = 0;
  return 0;
}

void freeObjectHeap()
{
#ifdef FREE_ON_EXIT
  {
    WObject obj;
    uint32 h;
    WClass *wclass;

    // call any native object destroy methods to free system resources
    for (h = 0; h < heap.numHandles; h++)
      {
	obj = h + FIRST_OBJ + 1;
	if (objectPtr(obj) != NULL)
	  {
	    wclass = WOBJ_class(obj);
	    if (wclass != NULL && wclass->objDestroyFunc)
	      wclass->objDestroyFunc(obj);
	  }
      }
  }
#endif
#if FIXED_OBJECT_HEAP_SIZE <= 0
  if (heap.mem)
    xfree(heap.mem);
#endif
}

// mark bits in the handle order array since it is not used during
// the mark object process (its used in the sweep phase)

#define MARK(o) heap.hos[- (int32)(o - FIRST_OBJ - 1)].order |= 0x80000000
#define IS_MARKED(o) (heap.hos[- (int32)(o - FIRST_OBJ - 1)].order & 0x80000000)

// mark this object and all the objects this object refers to and all
// objects those objects refer to, etc.
void markObject(WObject obj)
{
  WClass *wclass;
  WObject *arrayStart, o;
  uint32 i, len, type, numScan;

  if (!VALID_OBJ(obj) || objectPtr(obj) == NULL || IS_MARKED(obj))
    return;
  MARK(obj);
  numScan = 0;

 markinterior:
  wclass = WOBJ_class(obj);
  if (wclass == NULL)
    {
      // array - see if it contains object references
      type = WOBJ_arrayType(obj);
      if (type == 1 || type == 2)
	{
	  // for an array of arrays or object array
	  arrayStart = (WObject *)WOBJ_arrayStart(obj);
	  len = WOBJ_arrayLen(obj);
	  for (i = 0; i < len; i++)
	    {
	      o = arrayStart[i];
	      if (VALID_OBJ(o) && objectPtr(o) != NULL && !IS_MARKED(o))
		{
		  MARK(o);
		  heap.hos[- (int32)numScan].temp = o;
		  numScan++;
		}
	    }
	}
    }
  else
    {
      // object
      len = wclass->numVars;
      for (i = 0; i < len; i++)
	{
	  o = WOBJ_var(obj, i).obj;
	  if (VALID_OBJ(o) && objectPtr(o) != NULL && !IS_MARKED(o))
	    {
	      MARK(o);
	      heap.hos[- (int32)numScan].temp = o;
	      numScan++;
	    }
	}
    }
  if (numScan > 0)
    {
      // Note: we use goto since we want to avoid recursion here
      // since structures like linked links could create deep
      // stack calls
      --numScan;
      obj = heap.hos[- (int32)numScan].temp;
      goto markinterior;
    }
}

// NOTE: There are no waba methods that are called when objects are destroyed.
// This is because if a method was called, the object would be on its way to
// being GC'd and if we set another object (or static field) to reference it,
// after the GC, the reference would be stale.

void sweep()
{
  WObject obj;
  WClass *wclass;
  uint32 i, h, objSize, prevObjectSize, numUsedHandles;
  uchar *src, *dst;

  prevObjectSize = heap.objectSize;
  heap.objectSize = 0;

  // move all the marks over into the scan array so we don't have
  // to do lots of bit shifting
  for (i = 0; i < heap.numHandles; i++)
    {
      if (heap.hos[- (int32)i].order & 0x80000000)
	{
	  heap.hos[- (int32)i].order &= 0x7FFFFFFF; // clear mark bit
	  heap.hos[- (int32)i].temp = 1;
	}
      else
	{
	  heap.hos[- (int32)i].temp = 0;
	}
    }
  numUsedHandles = 0;
  for (i = 0; i < heap.numHandles; i++)
    {
      // we need to scan in memory order so we can compact things without
      // copying objects over each other
      h = heap.hos[- (int32)i].order;
      obj = h + FIRST_OBJ + 1;
      if (!heap.hos[- (int32)h].temp)
	{
	  // handle is free - dereference object
	  if (objectPtr(obj) != NULL)
	    {
	      wclass = WOBJ_class(obj);
				// for non-arrays, call objDestroy if present
	      if (wclass != NULL && wclass->objDestroyFunc)
		wclass->objDestroyFunc(obj);
	      heap.hos[- (int32)h].ptr = NULL;
	    }
	  continue;
	}
      wclass = WOBJ_class(obj);
      if (wclass == NULL)
	objSize = arraySize(WOBJ_arrayType(obj), WOBJ_arrayLen(obj));
      else
	objSize = WCLASS_objectSize(wclass);

      // copy object to new heap
      src = (uchar *)heap.hos[- (int32)h].ptr;
      dst = &heap.mem[heap.objectSize];
      if (src != dst)
	// NOTE: overlapping regions need to copy correctly
	xmemmove(dst, src, objSize);
      heap.hos[- (int32)h].ptr = (Var *)dst;
      heap.hos[- (int32)numUsedHandles].order = h;
      heap.objectSize += objSize;
      numUsedHandles++;
    }
  heap.numFreeHandles = heap.numHandles - numUsedHandles;
  for (i = 0; i < heap.numHandles; i++)
    if (!heap.hos[- (int32)i].temp)
      {
	// add free handle to free section of order array
	heap.hos[- (int32)numUsedHandles].order = i;
	numUsedHandles++;
      }
  // zero out the part of the heap that is now junk
  xmemzero(&heap.mem[heap.objectSize], prevObjectSize - heap.objectSize);
}

void gc()
{
  WClass *wclass;
  WObject obj;
  uint32 i, j;

  // mark objects on vm stack
  for (i = 0; i < vmStackPtr; i++)
    if (VALID_OBJ(vmStack[i].obj))
      markObject(vmStack[i].obj);
  // mark objects on native stack
  for (i = 0; i < nmStackPtr; i++)
    if (VALID_OBJ(nmStack[i]))
      markObject(nmStack[i]);
  // mark all static class objects
  for (i = 0; i < CLASS_HASH_SIZE; i++)
    {
      wclass = classHashList[i];
      while (wclass != NULL)
	{
	  for (j = 0; j < wclass->numFields; j++)
	    {
	      WClassField *field;

	      field = &wclass->fields[j];
	      if (!FIELD_isStatic(field))
		continue;
	      obj = field->var.staticVar.obj;
	      if (VALID_OBJ(obj))
		markObject(obj);
	    }
	  wclass = wclass->nextClass;
	}
    }
  sweep();
#ifdef DEBUGMEMSIZE
  debugMemSize();
#endif
}

// NOTE: size passed must be 4 byte aligned (see arraySize())
WObject allocObject(int32 size)
{
  uint32 i, sizeReq, hosSize;

  if (size <= 0)
    return 0;
  sizeReq = size;
  if (!heap.numFreeHandles)
    sizeReq += sizeof(Hos);
  hosSize = heap.numHandles * sizeof(Hos);
  if (sizeReq + hosSize + heap.objectSize > heap.memSize)
    {
      gc();
      // heap.objectSize changed or we are out of memory
      if (sizeReq + hosSize + heap.objectSize > heap.memSize)
	{
	  VmQuickError(ERR_OutOfObjectMem);
	  return 0;
	}
    }
  if (heap.numFreeHandles)
    {
      i = heap.hos[- (int32)(heap.numHandles - heap.numFreeHandles)].order;
      heap.numFreeHandles--;
    }
  else
    {
      // no free handles, get a new one
      i = heap.numHandles;
      heap.hos[- (int32)i].order = i;
      heap.numHandles++;
    }

  heap.hos[- (int32)i].ptr = (Var *)&heap.mem[heap.objectSize];
  heap.objectSize += size;
  return FIRST_OBJ + i + 1;
}

// NOTE: we made this function a #define and it showed no real performance
// gain over having it a function on either PalmOS or Windows when
// optimization was turned on.

Var *objectPtr(WObject obj)
{
  return heap.hos[- (int32)(obj - FIRST_OBJ - 1)].ptr;
}

//
// Native Method Stack
//

int pushObject(WObject obj)
{
  // prevent the pushed object from being freed by the garbage
  // collector. Used in native methods and with code calling
  // the VM. For example, if you have the following code
  //
  // obj1 = createObject(...);
  // obj2 = createObject(...);
  //
  // or..
  //
  // obj1 = createObject(...);
  // m = getMethod(..)
  //
  // since the second statement can cause a memory allocation
  // resulting in garbage collection (in the latter a class
  // load that allocates static class variables), obj1
  // would normally be freed. Pushing obj1 onto the "stack"
  // (which is a stack for this purpose) prevents that
  //
  // the code above should be change to:
  //
  // obj1 = createObject(...);
  // pushObject(obj1);
  // obj2 = createObject(...);
  // pushObject(obj2);
  // ..
  // if (popObject() != obj2)
  //   ..error..
  // if (popObject() != obj1)
  //   ..error..
  //

	// NOTE: Running out of Native Stack space can cause serious program
	// failure if any code doesn't check the return code of pushObject().
	// Any code that does a pushObject() should check for failure and if
	// failure occurs, then abort.
  if (nmStackPtr >= nmStackSize)
    {
      VmQuickError(ERR_NativeStackOverflow);
      return -1;
    }
  nmStack[nmStackPtr++] = obj;
  return 0;
}

WObject popObject()
{
  if (nmStackPtr == 0)
    {
      VmQuickError(ERR_NativeStackUnderflow);
      return 0;
    }
  return nmStack[--nmStackPtr];
}

/*  #if defined(PALMOS) */
/*  #include "palm/nmpalm_c.c" */
/*  #elif defined(WIN32) */
/*  #include "win32/nmwin32_c.c" */
/*  #elif defined(LINUX) */
/*  #include "linux/nm_linux_c.c" */
/*  #endif */

#ifndef WITH_DOUBLE_BUFFER

/* 
 * Default definition of MainWindow.paint(), when double-buffering is not used.
 * 
 * Ideally, we would just #define out the MainWinPaint reference below in the
 * nativeMethods array, but that does not work because the MainWindow.java file
 * includes a declaration of a native paint() method, which can't be #defined out
 * (unless we include a preprocessing step in the building of the Waba classes,
 * which seems heavy).
 */

Var MainWinPaint (Var stack [])
{
  Var obj;

  obj.intValue = 0;
  return obj;
}
 
#endif /* WITH_DOUBLE_BUFFER */

// This array is used to map a hash value to a corresponding native function.
// It must remain sorted by hash value because a binary search is performed
// to find a method by its hash value. There is a small chance of collision
// if two hashes match and if one occurs, the function name should be changed
// to avoid collision. To prevent users from creating invalid native methods
// that hash to a valid value, native methods could be restricted to a specific
// set of classes (this is probably not necessary since any verifier probably
// wouldn't allow native methods to get by anyway).

NativeMethod nativeMethods[] =
{
  // waba/sys/Vm_exec_(Ljava/lang/String;Ljava/lang/String;IZ)I
  { 113969325ul, VmExec },
  // waba/sys/Vm_getTimeStamp_()I
  { 113990543ul, VmGetTimeStamp },
  // waba/sys/Vm_copyArray_(Ljava/lang/Object;ILjava/lang/Object;II)Z
  { 114004019ul, copyArray },
  // waba/sys/Vm_exit_(I)V
  { 114010760ul, VmExit },
  // waba/sys/Vm_sleep_(I)V
  { 114016841ul, VmSleep },
  // waba/sys/Vm_setDeviceAutoOff_(I)I
  { 114019540ul, VmSetDeviceAutoOff },
  // waba/sys/Vm_getUserName_()Ljava/lang/String;
  { 114021471ul, VmGetUserName },
  // waba/sys/Vm_getPlatform_()Ljava/lang/String;
  { 114023839ul, VmGetPlatform },
  // waba/sys/Vm_isColor_()Z
  { 114024842ul, VmIsColor },

#ifdef WITH_WABAJTRON_CLASSES
  { 1239586823ul, CyclicHandler_stop  },// org/wabajtron/attach/CyclicHandler_stop_()I
  { 1239593480ul, CyclicHandler_start },// org/wabajtron/attach/CyclicHandler_start_()I
#endif

#ifdef WITH_WABAJTRON_CLASSES
  { 1767444054ul, Task_raiseTaskException     },// org/wabajtron/attach/Task_raiseTaskException_(I)I
  { 1767446732ul, Task_terminate              },// org/wabajtron/attach/Task_terminate_()I
  { 1767457806ul, Task_releaseWait            },// org/wabajtron/attach/Task_releaseWait_()I
  { 1767458446ul, Task_forceResume            },// org/wabajtron/attach/Task_forceResume_()I
  { 1767460622ul, Task_getPriority            },// org/wabajtron/attach/Task_getPriority_()I
  { 1767462233ul, Task_getCurrentThreadTaskId },// org/wabajtron/attach/Task_getCurrentThreadTaskId_()I
  { 1767463759ul, Task_cancelWakeup           },// org/wabajtron/attach/Task_cancelWakeup_()I
  { 1767476305ul, Task_cancelActivate         },// org/wabajtron/attach/Task_cancelActivate_()I
  { 1767484114ul, Task_changePriority         },// org/wabajtron/attach/Task_changePriority_(I)I
  { 1767484616ul, Task_sleep                  },// org/wabajtron/attach/Task_sleep_()I
  { 1767488649ul, Task_delay                  },// org/wabajtron/attach/Task_delay_(I)I
  { 1767489289ul, Task_sleepT                 },// org/wabajtron/attach/Task_sleep_(I)I
  { 1767490792ul, Task_currentTask            },// org/wabajtron/attach/Task_currentTask_()Lorg/wabajtron/attach/Task;
  { 1767492041ul, Task_wakeup                 },// org/wabajtron/attach/Task_wakeup_()I
  { 1767492297ul, Task_resume                 },// org/wabajtron/attach/Task_resume_()I
  { 1767499530ul, Task_suspend                },// org/wabajtron/attach/Task_suspend_()I
  { 1767504587ul, Task_activate               },// org/wabajtron/attach/Task_activate_()I
#endif

#ifdef WITH_WABAJTRON_CLASSES
  { 2631644883ul, Kernel_getRunningTaskID },// org/wabajtron/attach/Kernel_getRunningTaskID_()I
  { 2631653962ul, Kernel_getTime          },// org/wabajtron/attach/Kernel_getTime_()I
  { 2631654164ul, Kernel_rotateReadyQueue },// org/wabajtron/attach/Kernel_rotateReadyQueue_(I)I
  { 2631659403ul, Kernel_setTime          },// org/wabajtron/attach/Kernel_setTime_(I)I
#endif

#ifdef WITH_FILE_CLASS
  // waba/io/File_getLength_()I
  { 340528908ul, FileGetLength },
  // waba/io/File_createDir_()Z
  { 340529036ul, FileCreateDir },
  // waba/io/File_readBytes_([BII)I
  { 340548368ul, FileReadBytes },
  // waba/io/File_rename_(Ljava/lang/String;)Z
  { 340553947ul, FileRename },
  // waba/io/File__nativeCreate_()V
  { 340555856ul, FileCreate },
  // waba/io/File_writeBytes_([BII)I
  { 340557521ul, FileWriteBytes },
  // waba/io/File_listDir_()[Ljava/lang/String;
  { 340560348ul, FileListDir },
  // waba/io/File_seek_(I)Z
  { 340567816ul, FileSeekWaba },
  // waba/io/File_isDir_()Z
  { 340568456ul, FileIsDir },
  // waba/io/File_close_()Z
  { 340570184ul, FileCloseWaba },
  // waba/io/File_isOpen_()Z
  { 340575817ul, FileIsOpen },
  // waba/io/File_delete_()Z
  { 340576137ul, FileDeleteWaba },
  // waba/io/File_exists_()Z
  { 340579017ul, FileExists },
  // waba/fx/Image__nativeCreate_()V
#endif

#ifdef WITH_WABAJTRON_CLASSES
  { 3818788557ul, EventFlag_waitFlag  },// org/wabajtron/attach/EventFlag_waitFlag_(II)I
  { 3818793230ul, EventFlag_waitFlagT },// org/wabajtron/attach/EventFlag_waitFlag_(III)I
  { 3818818503ul, EventFlag_set       },// org/wabajtron/attach/EventFlag_set_(I)I
  { 3818830025ul, EventFlag_poll      },// org/wabajtron/attach/EventFlag_poll_(II)I
  { 3818830473ul, EventFlag_clear     },// org/wabajtron/attach/EventFlag_clear_(I)I
#endif

#ifdef WITH_WABAJTRON_CLASSES
  { 3986586832ul, Semaphore_waitSemaphore  },// org/wabajtron/attach/Semaphore_waitSemaphore_()I
  { 3986591505ul, Semaphore_waitSemaphoreT },// org/wabajtron/attach/Semaphore_waitSemaphore_(I)I
  { 3986592839ul, Semaphore_poll           },// org/wabajtron/attach/Semaphore_poll_()I
  { 3986605577ul, Semaphore_signal         },// org/wabajtron/attach/Semaphore_signal_()I
#endif

#ifdef WITH_GRAPHICS_CLASS
  { 781023312ul, ImageCreate },
  // waba/fx/Image_free_()V
  { 781029959ul, ImageFree },
  // waba/fx/Image_setPixels_(I[IIII[B)V
  { 781038420ul, ImageSetPixels },
  // waba/fx/Image_useImagePalette_(Z)V
  { 781041683ul, ImageUseImagePalette },
  // waba/fx/Image__nativeLoad_(Ljava/lang/String;)V
  { 781052768ul, ImageLoad },
#endif

  // waba/fx/Sound_beep_()V
  { 940413127ul, SoundBeep },
  // waba/fx/Sound_tone_(II)V
  { 940424137ul, SoundTone },

  // waba/sys/Time__nativeCreate_()V
  { 969766992ul, TimeCreate },

#ifdef WITH_SOCKET_CLASS
  // waba/io/Socket__nativeCreate_(Ljava/lang/String;I)V
  { 1317941923ul, SocketCreate },
  // waba/io/Socket_readBytes_([BII)I
  { 1317952272ul, SocketRead },
  // waba/io/Socket_writeBytes_([BII)I
  { 1317961425ul, SocketWrite },
  // waba/io/Socket_setReadTimeout_(I)Z
  { 1317972178ul, SocketSetReadTimeout },
  // waba/io/Socket_close_()Z
  { 1317974088ul, SocketClose },
  // waba/io/Socket_isOpen_()Z
  { 1317979721ul, SocketIsOpen },
#endif //WITH_SOCKET

#ifdef WITH_GUI_CLASS
  // waba/ui/Window__nativeCreate_()V
  { 1406040144ul, WindowCreate },
#endif

#ifdef WITH_CATALOG_CLASS
  // waba/io/Catalog_listCatalogs_()[Ljava/lang/String;
  { 1661930913ul, CatalogListCatalogs },
  // waba/io/Catalog_addRecord_(I)I
  { 1661934285ul, CatalogAddRecord },
  // waba/io/Catalog_skipBytes_(I)I
  { 1661937741ul, CatalogSkipBytes },
  // waba/io/Catalog__nativeCreate_(Ljava/lang/String;I)V
  { 1661940387ul, CatalogCreate },
  // waba/io/Catalog_readBytes_([BII)I
  { 1661950736ul, CatalogRead },
  // waba/io/Catalog_deleteRecord_()Z
  { 1661951823ul, CatalogDeleteRecord },
  // waba/io/Catalog_setRecordPos_(I)Z
  { 1661957200ul, CatalogSetRecordPos },
  // waba/io/Catalog_getRecordSize_()I
  { 1661957392ul, CatalogGetRecordSize },
  // waba/io/Catalog_resizeRecord_(I)Z
  { 1661958480ul, CatalogResizeRecord },
  // waba/io/Catalog_writeBytes_([BII)I
  { 1661959889ul, CatalogWrite },
  // waba/io/Catalog_getRecordCount_()I
  { 1661964433ul, CatalogGetRecordCount },
  // waba/io/Catalog_close_()Z
  { 1661972552ul, CatalogClose },
  // waba/io/Catalog_isOpen_()Z
  { 1661978185ul, CatalogIsOpen },
  // waba/io/Catalog_delete_()Z
  { 1661978505ul, CatalogDelete },
#endif

#ifdef WITH_THREAD
  // waba/sys/Thread_signalAll_()V
  { 1812926348ul, ThreadSignalAll },
  // waba/sys/Thread_waitForSignal_()V
  { 1812953168ul, ThreadWaitForSignal },
  // waba/sys/Thread_stop_()V
  { 1812962119ul, ThreadStop },
  // waba/sys/Thread_start_()V
  { 1812968776ul, ThreadStart },
  // waba/sys/Thread_sleep_(I)V
  { 1812972105ul, ThreadSleep },
  // waba/sys/Thread_currentThread_()Lwaba/sys/Thread
  { 1812986080ul, ThreadCurrentThread },
#endif

#ifdef WITH_GRAPHICS_CLASS
  // waba/fx/Graphics_copyRect_(Lwaba/fx/ISurface;IIIIII)V
  { 2182088099ul, GraphicsCopyRect },
  // waba/fx/Graphics_clearClip_()V
  { 2182090124ul, GraphicsClearClip },
  // waba/fx/Graphics_setFont_(Lwaba/fx/Font;)V
  { 2182094808ul, GraphicsSetFont },
  // waba/fx/Graphics_setDrawOp_(I)V
  { 2182095437ul, GraphicsSetDrawOp },
  // waba/fx/Graphics_setClip_(IIII)V
  { 2182096846ul, GraphicsSetClip },
  // waba/fx/Graphics_setColor_(III)V
  { 2182099790ul, GraphicsSetColor },
  // waba/fx/Graphics_getClip_(Lwaba/fx/Rect;)Lwaba/fx/Rect;
  { 2182102117ul, GraphicsGetClip },
  // waba/fx/Graphics_fillRect_(IIII)V
  { 2182103055ul, GraphicsFillRect },
  // waba/fx/Graphics_drawLine_(IIII)V
  { 2182103119ul, GraphicsDrawLine },
  // waba/fx/Graphics_translate_(II)V
  { 2182103502ul, GraphicsTranslate },
  // waba/fx/Graphics_drawDots_(IIII)V
  { 2182104271ul, GraphicsDrawDots },
  // waba/fx/Graphics_drawText_([CIIII)V
  { 2182115089ul, GraphicsDrawChars },
  // waba/fx/Graphics__nativeCreate_()V
  { 2182117456ul, GraphicsCreate },
  // waba/fx/Graphics_drawCursor_(IIII)V
  { 2182118865ul, GraphicsDrawCursor },
  // waba/fx/Graphics_setClipRect_(IIII)V
  { 2182122322ul, GraphicsSetClip },
  // waba/fx/Graphics_setBackColor_(III)V
  { 2182123410ul, GraphicsSetBackColor },
  // waba/fx/Graphics_free_()V
  { 2182124103ul, GraphicsFree },
  // waba/fx/Graphics_setForeColor_(III)V
  { 2182125138ul, GraphicsSetForeColor },
  // waba/fx/Graphics_setTextColor_(III)V
  { 2182126738ul, GraphicsSetTextColor },
  // waba/fx/Graphics_fillPolygon_([I[II)V
  { 2182132179ul, GraphicsFillPolygon },
  // waba/fx/Graphics_drawText_(Ljava/lang/String;II)V
  { 2182138655ul, GraphicsDrawString },
#endif

  // waba/sys/VmShell_print_(Ljava/lang/String;)V   // SD
  { 2228247834ul, VmPrint },                        // SD
  // waba/sys/VmShell_println_(Ljava/lang/String;)V // SD
  { 2228261788ul, VmPrintLn },                      // SD

#ifdef WITH_REGISTRY_CLASS
  // waba/io/Registry_skipBytes_(I)I
  { 2324703309ul, RegistrySkipBytes },
  // waba/io/Registry_readBytes_([BII)I
  { 2324716304ul, RegistryReadBytes },
  // waba/io/Registry__nativeClose_()Z
  { 2324718031ul, RegistryClose },
  // waba/io/Registry__nativeDelete_()Z
  { 2324723984ul, RegistryDelete },
  // waba/io/Registry__nativeCreate_()Z
  { 2324724048ul, RegistryCreate },
  // waba/io/Registry_writeBytes_([BII)I
  { 2324725457ul, RegistryWriteBytes },
  // waba/io/Registry_spaceAvailable_()I
  { 2324728273ul, RegistrySpaceAvailable },
#endif

  // waba/sys/Convert_toInt_(Ljava/lang/String;)I
  { 2387628570ul, ConvertStringToInt },
  // waba/sys/Convert_toIntBitwise_(F)I
  { 2387636560ul, ConvertFloatToIntBitwise },
  // waba/sys/Convert_toString_(C)Ljava/lang/String;
  { 2387649437ul, ConvertCharToString },
  // waba/sys/Convert_toString_(D)Ljava/lang/String;
  { 2387649501ul, ConvertDoubleToString },
  // waba/sys/Convert_toFloatBitwise_(I)F
  { 2387649554ul, ConvertIntToFloatBitwise },
  // waba/sys/Convert_toString_(F)Ljava/lang/String;
  { 2387649629ul, ConvertFloatToString },
  // waba/sys/Convert_toString_(I)Ljava/lang/String;
  { 2387649821ul, ConvertIntToString },
  // waba/sys/Convert_toString_(J)Ljava/lang/String;
  { 2387649885ul, ConvertLongToString },
  // waba/sys/Convert_toString_(Z)Ljava/lang/String;
  { 2387650909ul, ConvertBooleanToString },

#ifdef WITH_SOUNDCLIP_CLASS
  // waba/fx/SoundClip_play_()Z
  { 2584844359ul, SoundClipPlay },
#endif

#if defined(PALMOS)
  // waba/ui/PalmOsPref_setPalmOsPref_([B)V
  { 2886900306ul, PalmOsPrefSetPalmOsPref },
  // waba/ui/PalmOsPref_getPalmOsPref_([BII)I
  { 2886908052ul, PalmOsPrefGetPalmOsPref },
#endif

#ifdef WITH_GUI_CLASS
  // waba/ui/MainWindow__nativeCreate_()V
  { 3037886544ul, MainWinCreate },
  // waba/ui/MainWindow_exit_(I)V
  { 3037899400ul, MainWinExit },
  // waba/ui/MainWindow_paint_()V
  { 3037901000ul, MainWinPaint },
  // waba/ui/MainWindow__setTimerInterval_(I)V
  { 3037919317ul, MainWinSetTimerInterval },
#endif

#ifdef WITH_SERIALPORT_CLASS
  // waba/io/SerialPort_readCheck_()I
  { 3046245644ul, SerialPortReadCheck },
  // waba/io/SerialPort_readBytes_([BII)I
  { 3046267664ul, SerialPortRead },
  // waba/io/SerialPort_writeBytes_([BII)I
  { 3046276817ul, SerialPortWrite },
  // waba/io/SerialPort_setReadTimeout_(I)Z
  { 3046287570ul, SerialPortSetReadTimeout },
  // waba/io/SerialPort_close_()Z
  { 3046289480ul, SerialPortClose },
  // waba/io/SerialPort_setFlowControl_(Z)Z
  { 3046290066ul, SerialPortSetFlowControl },
  // waba/io/SerialPort_isOpen_()Z
  { 3046295113ul, SerialPortIsOpen },
  // waba/io/SerialPort__nativeCreate_(IIIZI)V
  { 3046299605ul, SerialPortCreate },
#endif

#ifdef WITH_FONTMETRICS_CLASS
  // waba/fx/FontMetrics_getTextWidth_(Ljava/lang/String;)I
  { 3511879649ul, FontMetricsGetStringWidth },
  // waba/fx/FontMetrics_getCharWidth_(C)I
  { 3511903952ul, FontMetricsGetCharWidth },
  // waba/fx/FontMetrics__nativeCreate_()V
  { 3511908432ul, FontMetricsCreate },
  // waba/fx/FontMetrics_getTextWidth_([CII)I
  { 3511921619ul, FontMetricsGetCharArrayWidth },
#endif

#ifdef WITH_SMARTDATA_CLASSES
  // ch/smartdata/ui/Edit__nativeDraw_(Lwaba/fx/Graphics;Z)V
  { 3746824481ul, EditDraw },
  // ch/smartdata/ui/Edit__nativePenDrag_(Lwaba/ui/PenEvent;)V
  { 3746835555ul, EditPenDrag },
  // ch/smartdata/ui/Edit__nativePenDown_(Lwaba/ui/PenEvent;)V
  { 3746837219ul, EditPenDown },
  // ch/smartdata/ui/Edit__nativeSetText_(Ljava/lang/String;)V
  { 3746840483ul, EditSetText },
  // ch/smartdata/ui/Edit__nativeEvent_()V
  { 3746849743ul, EditEvent },
  // ch/smartdata/ui/Edit_getText_()Ljava/lang/String;
  { 3746854299ul, EditGetText },
  // ch/smartdata/ui/Edit__nativeCreate_()V
  { 3746854992ul, EditCreate },
  // ch/smartdata/ui/Edit_onPaint_(Lwaba/fx/Graphics;)V
  { 3746857948ul, EditPaint },
  // ch/smartdata/ui/Edit__nativeFocusIn_()V
  { 3746861329ul, EditFocusIn },
  // ch/smartdata/ui/Edit__nativeFocusOut_()V
  { 3746869586ul, EditFocusOut },
  // ch/smartdata/ui/Edit__nativeKeyPress_(II)V
  { 3746878804ul, EditKeyPress }
#endif
};

NativeFunc getNativeMethod(WClass *wclass, UtfString methodName, UtfString methodDesc)
{
  UtfString className;
  NativeMethod *nm;
  uint32 hash, classHash, methodHash;
  uint16 top, bot, mid;

  className = getUtfString(wclass, wclass->classNameIndex);
  classHash = genHashCode(className) % 65536;
  methodHash = (genHashCode(methodName) + genHashCode(methodDesc)) % 65536;
  hash = (classHash << 16) + methodHash;

/*
  DPRINTF(">>> nativeExecute: n[%s.%s] (hash=%ul)\n",
	  UTF2CSTR(&className),
	  UTF2CSTR(&methodName),
	  hash);
*/

  // binary search to find matching hash value
  top = 0;
  bot = sizeof(nativeMethods) / sizeof(NativeMethod);
  if (bot == 0)
    return NULL;
  while (1)
    {
      mid = (bot + top) / 2;
      nm = &nativeMethods[mid];
      if (hash == nm->hash) {
	return nm->func;
      }
      if (mid == top)
	break; // not found
      if (hash < nm->hash)
	bot = mid;
      else
	top = mid;
    }

#if defined(LINUX)
  {
    uint16 i;

    printf("** Native Method Missing:\n");
    printf("// %.*s_%.*s_%.*s\n{ %u, func },\n",
	className.len, className.str,
	methodName.len, methodName.str,
	methodDesc.len, methodDesc.str,
	hash);
  }
#endif
  return NULL;
}

// Hooks are used for objects that access system resources. Classes
// that are "hooked" may allocate extra variables so system resource
// pointers can be stored in the object. All hooked objects
// have an object destroy function that is called just before they
// are garbage collected allowing system resources to be deallocated.
void setClassHooks(WClass *wclass)
{
  UtfString className;
  ClassHook *hook;
  uint16 i, nameLen;

  // NOTE: Like native methods, we could hash the hook class names into
  // a value if we make sure that the only time we'd check an object for
  // hooks is if it was in the waba package. This would make lookup
  // faster and take up less space. If the hook table is small, though,
  // it doesn't make much difference.
  className = getUtfString(wclass, wclass->classNameIndex);
  if (className.len < 6)
    return; // all hook classes are >= 6 character names
  i = 0;
  while (1)
    {
      hook = &classHooks[i++];
      if (hook->className == NULL)
	break;
      if (className.str[5] != hook->className[5])
	continue; // quick check to see if we should compare at all
      nameLen = xstrlen(hook->className);
      if (className.len == nameLen &&
	  !xstrncmp(className.str, hook->className, nameLen))
	{
	  wclass->objDestroyFunc = hook->destroyFunc;
	  wclass->numVars += hook->varsNeeded;
	  return;
	}
    }
}

/*
 "Thirty spokes join at the hub;
  their use for the cart is where they are not.
  When the potter's wheel makes a pot,
  the use of the pot is precisely where there is nothing.
  When you open doors and windows for a room,
  it is where there is nothing that they are useful to the room.
  Therefore being is for benefit,
  Nonbeing is for usefulness."
*/

//
// Method Execution
//

//
// This is the interpreter code. Each method call pushes a frame on the
// stack. The structure of a stack frame is:
//
// local var 1
// local var 2
// local var N
// local stack 1
// local stack 2
// local stack N
//
// when a method is called, the following is pushed on the stack before
// the next frame:
//
// wclass
// method
// pc
// var
// stack
//
// NOTE: You can, of course, increase the performance of the interpreter
// in a number of ways. I've found a good book on assembly optimization
// to be:
//
// Inner Loops - A sourcebook for fast 32-bit software development
// by Rick Booth
//

#ifdef WITH_64BITS

// guich@200 - transforms 2 consecutive vars in one double;
//             the first var stores the lower 32 bits and
//             the 2nd var stores the upper 32 bits of the double
float64 vars2double(Var *v)
{
  float64HiLo fhl;

  fhl.ul.lo = v->float64ValueHalf;
  fhl.ul.hi = (v+1)->float64ValueHalf;

  return fhl.value;
}
// guich@200 - transforms one double or in 2 consecutive vars
/* also used with ldc2_w, so d can be an "long"...           */
/* warning : seem to be "ENDIAN-sensible" (hi-lo or lo-hi ?) */
/* NOT CORRESPONDING TO lo-hi on Linux/intel, but to hi-lo   */
static void double2vars(float64 d, Var *v)
{
  float64HiLo fhl;

  fhl.value = d;

  v->float64ValueHalf = fhl.ul.hi;
  (v+1)->float64ValueHalf = fhl.ul.lo;
}
// guich@200 - transforms 2 consecutive vars in one long
//             the first var stores the lower 32 bits and
//             the 2nd var stores the upper 32 bits of the double
int64 vars2int64(Var *v)
{
  int64HiLo ihl;

  ihl.ul.lo = v->int64ValueHalf;
  ihl.ul.hi = (v+1)->int64ValueHalf;

  return ihl.value;
}
// guich@200 - transforms one double in 2 consecutive vars
static void int642vars(int64 l, Var *v)
{
  int64HiLo ihl;

  ihl.value = l;

  v->int64ValueHalf = ihl.ul.lo;
  (v+1)->int64ValueHalf = ihl.ul.hi;
}
// guich@200 - copy 2 variables of 64 bits
static void copy64(Var *src, Var *dest)
{
   *dest = *src;
   *(dest+1) = *(src+1);
}

#endif   /* WITH_64BITS */

/*
 * Shift functions
 */

/* Mask for sign extension */

const uint32 EXTEND_SIGN_MASK_32 [] =
  {
    0x00000000,
    0x80000000,
    0xC0000000,
    0xE0000000,
    0xF0000000,
    0xF8000000,
    0xFC000000,
    0xFE000000,
    0xFF000000,
    0xFF800000,
    0xFFC00000,
    0xFFE00000,
    0xFFF00000,
    0xFFF80000,
    0xFFFC0000,
    0xFFFE0000,
    0xFFFF0000,
    0xFFFF8000,
    0xFFFFC000,
    0xFFFFE000,
    0xFFFFF000,
    0xFFFFF800,
    0xFFFFFC00,
    0xFFFFFE00,
    0xFFFFFF00,
    0xFFFFFF80,
    0xFFFFFFC0,
    0xFFFFFFE0,
    0xFFFFFFF0,
    0xFFFFFFF8,
    0xFFFFFFFC,
    0xFFFFFFFE
  };

#ifdef WITH_64BITS

const uint64 EXTEND_SIGN_MASK_64 [] =
  {
    0x0000000000000000L,
    0x8000000000000000L,
    0xC000000000000000L,
    0xE000000000000000L,
    0xF000000000000000L,
    0xF800000000000000L,
    0xFC00000000000000L,
    0xFE00000000000000L,
    0xFF00000000000000L,
    0xFF80000000000000L,
    0xFFC0000000000000L,
    0xFFE0000000000000L,
    0xFFF0000000000000L,
    0xFFF8000000000000L,
    0xFFFC000000000000L,
    0xFFFE000000000000L,
    0xFFFF000000000000L,
    0xFFFF800000000000L,
    0xFFFFC00000000000L,
    0xFFFFE00000000000L,
    0xFFFFF00000000000L,
    0xFFFFF80000000000L,
    0xFFFFFC0000000000L,
    0xFFFFFE0000000000L,
    0xFFFFFF0000000000L,
    0xFFFFFF8000000000L,
    0xFFFFFFC000000000L,
    0xFFFFFFE000000000L,
    0xFFFFFFF000000000L,
    0xFFFFFFF800000000L,
    0xFFFFFFFC00000000L,
    0xFFFFFFFE00000000L,
    0xFFFFFFFF00000000L,
    0xFFFFFFFF80000000L,
    0xFFFFFFFFC0000000L,
    0xFFFFFFFFE0000000L,
    0xFFFFFFFFF0000000L,
    0xFFFFFFFFF8000000L,
    0xFFFFFFFFFC000000L,
    0xFFFFFFFFFE000000L,
    0xFFFFFFFFFF000000L,
    0xFFFFFFFFFF800000L,
    0xFFFFFFFFFFC00000L,
    0xFFFFFFFFFFE00000L,
    0xFFFFFFFFFFF00000L,
    0xFFFFFFFFFFF80000L,
    0xFFFFFFFFFFFC0000L,
    0xFFFFFFFFFFFE0000L,
    0xFFFFFFFFFFFF0000L,
    0xFFFFFFFFFFFF8000L,
    0xFFFFFFFFFFFFC000L,
    0xFFFFFFFFFFFFE000L,
    0xFFFFFFFFFFFFF000L,
    0xFFFFFFFFFFFFF800L,
    0xFFFFFFFFFFFFFC00L,
    0xFFFFFFFFFFFFFE00L,
    0xFFFFFFFFFFFFFF00L,
    0xFFFFFFFFFFFFFF80L,
    0xFFFFFFFFFFFFFFC0L,
    0xFFFFFFFFFFFFFFE0L,
    0xFFFFFFFFFFFFFFF0L,
    0xFFFFFFFFFFFFFFF8L,
    0xFFFFFFFFFFFFFFFCL,
    0xFFFFFFFFFFFFFFFEL
  };

#endif /* WITH_64BITS */

/*
 * Left shift 
 */
inline int32 ishl (int32 value, unsigned short shift) {
  /* Mask the shift value to 5 bits, according to JLS */
  shift &= 0x1f;

  return value << shift;
}

#ifdef WITH_64BITS

inline int64 lshl (int64 value, unsigned short shift) {
  /* Mask the shift value to 6 bits, according to JLS */
  shift &= 0x3f;

  return value << shift;
}

#endif /* WITH_64BITS */

/*
 * Right shift
 *
 * According to K&R, as well as 'http://www-ccs.ucsd.edu/c/express.html#Right%20Shift',
 * "If X is nonnegative, then zeros fill the vacated bit positions; 
 *  otherwise, the result is implementation defined."
 */
/* Arithmetic right shift */
inline int32 ishr (int32 value, unsigned short shift) {
  /* Mask the shift value to 5 bits, according to JLS */
  shift &= 0x1f;

  if (value >= 0) {
    return (value >> shift);
  } else {
    return (value >> shift) | EXTEND_SIGN_MASK_32 [shift];
  }
}

#ifdef WITH_64BITS

inline int64 lshr (int64 value, unsigned short shift) {
  /* Mask the shift value to 6 bits, according to JLS */
  shift &= 0x3f;

  if (value >= 0) {
    return (value >> shift);
  } else {
    return (value >> shift) | EXTEND_SIGN_MASK_64 [shift];
  }
}

#endif /* WITH_64BITS */

/* Unsigned right shift */
inline int32 iushr (int32 value, unsigned short shift) {
  /* Mask the shift value to 5 bits, according to JLS */
  shift &= 0x1f;

  /* Force the value to unsigned (and thus non-negative) */
  return (uint32) value >> shift;
}

#ifdef WITH_64BITS

inline int64 lushr (int64 value, unsigned short shift) {
  /* Mask the shift value to 6 bits, according to JLS */
  shift &= 0x3f;

  /* Force the value to unsigned (and thus non-negative) */
  return (uint64) value >> shift;
}

#endif /* WITH_64BITS */


#ifdef WITH_THREAD_NATIVE
DWORD WINAPI executeMethod(void* passedWParams)
{
    WClass *wclass;
    WClassMethod *method;
    Var params[7];
    uint32 numParams;

    Var *var;
    Var *stack;
    uchar *pc;

    uint32 baseFramePtr;

    // virtual method stack for each instance of thread.
    uint32  vmStackPtr;
    uint32  vmStackSize;
    Var*    vmStack;

    // native method stack for each instance of thread.
    uint32    nmStackPtr;
    uint32    nmStackSize;
    WObject*  nmStack;

    vmStackPtr = 0;
    vmStackSize = getVmStackSizeInBytes(); // in Var units
    vmStack = (Var*)xmalloc(vmStackSize); //Don't forget to free this on method return.

    nmStackPtr = 0;
    nmStackSize = getNmStackSizeInBytes(); // in WObject units
    nmStack = (WObject*)xmalloc(nmStackSize); //Don't forget to free this on method return.

    // zero out memory areas
    xmemzero((uchar*)vmStack, vmStackSize);
    xmemzero((uchar*)nmStack, nmStackSize);

    //Copy vmStack[] & nmStack[] from the main stacks.
    xmemmove(vmStack, getVmStack(), vmStackSize);
    xmemmove(nmStack, getNmStack(), nmStackSize);

    //This trick is necessary to bridge with "CreateThread()" Win32 API.
    //Also useful to bridge with POSIX pthreads.
    wclass = ((WVmParam*)passedWParams)->vclass;
    method = ((WVmParam*)passedWParams)->method;
    numParams = ((WVmParam*)passedWParams)->paramSize;
    xmemmove(params, ((WVmParam*)passedWParams)->params, (sizeof(Var)*numParams));
#else
void executeMethod( WClass *wclass,
                    WClassMethod *method,
                    Var params[],
                    uint32 numParams)
{
  register Var *var;
  register Var *stack;
  register uchar *pc;

  uint32 baseFramePtr;
#endif

#ifdef DEBUG
  {
    UtfString c, m;
    c = getUtfString(wclass, wclass->classNameIndex);
    m = getUtfString(wclass, METH_nameIndex(method));


    printf("executeMethod: m[%s.%s]  #par=%d\r\n",
	   c.str,
	   m.str,
	   numParams
	   );
  }
#endif



  // the variables wclass, method, var, stack, and pc need to be
  // pushed and restored when calling methods using "goto methoinvoke"

  // also note that this method does recurse when we hit static class
  // initializers which execute within a class load operation and this
  // is why we exit when we keep trace of the baseFramePtr.

#ifdef WITH_THREAD
      if((METH_accessFlags(method) & ACCESS_SYNCHRONIZED) > 0)
      {
	if (!registerAsSyncronized(method))
	  {
	    // Yes, since the method ID doesn't match.
	    // So just make the flow of code return.
	    goto methodreturn;
	  }
	// When the flow of the code reaches to this point,
	// the lock is acquired, so let it flow 'till the end.
      }
#endif

    if((METH_accessFlags(method) & ACCESS_NATIVE) > 0)
    {
#ifdef WITH_THREAD_NATIVE
        return 0;
#else
        return; // can't execute native methods directly
#endif
    }
    if(method->code.codeAttr == NULL)
    {
#ifdef WITH_THREAD_NATIVE
        return 0;
#else
        return; // method has no code code attribute - compiler is broken
#endif
    }

    baseFramePtr = vmStackPtr;

    if(vmStackPtr + 3 + METH_maxLocals(method) + METH_maxStack(method) + (uint32)2 >= vmStackSize)
        goto stack_overflow_error;

  // push an unused return stack frame. This is important since all stack
  // frames need to look the same. Stack frames that are pushed by invoke
  // need to look the same as stack frames that are pushed when a static
  // class initialzer method is executed or the stack could not be walked.
  vmStack[vmStackPtr++].pc = 0;
  vmStack[vmStackPtr++].refValue = 0;
  vmStack[vmStackPtr++].refValue = 0;

  // push params into local vars of frame
  while (numParams > 0)
    {
      numParams--;
      vmStack[vmStackPtr + numParams] = params[numParams];
    }

 methodinvoke:
  // push active stack frame:
  //
  // local var 1
  // ...
  // local var N
  // local stack 1
  // ...
  // local stack N
  // method pointer
  // class pointer
  var = &vmStack[vmStackPtr];
  vmStackPtr += METH_maxLocals(method);
  stack = &vmStack[vmStackPtr];
  vmStackPtr += METH_maxStack(method);
  vmStack[vmStackPtr++].refValue = method;
  vmStack[vmStackPtr++].refValue = wclass;
  pc = METH_code(method);

#ifdef DEBUG
  {
    UtfString c, m;
    struct timeval tv;
    c = getUtfString(wclass, wclass->classNameIndex);
    m = getUtfString(wclass, METH_nameIndex(method));

    gettimeofday (&tv, NULL);
    DPRINTF (">>> [%d.%d] ", tv.tv_sec, tv.tv_usec);
    DPRINTF("invoking method: m[%s.%s]\n",
	    UTF2CSTR(&c),
	    UTF2CSTR(&m)
	    );
  }
#endif

 step:

#ifdef DEBUG_OPCODE

  printf( "[OPCODE] : %s\n", *pc > OP_MAX_OP ? "WRONG OP" : _OP_name[ *pc ] );

#endif  /* DEBUG_OPCODE */

  switch (*pc)
    {
    case OP_nop:
      pc++;
      break;
    case OP_aconst_null:
      stack[0].obj = 0;
      stack++;
      pc++;
      break;
    case OP_iconst_m1:
    case OP_iconst_0:
    case OP_iconst_1:
    case OP_iconst_2:
    case OP_iconst_3:
    case OP_iconst_4:
    case OP_iconst_5:
      // NOTE: testing shows there is no real performance gain to
      // splitting these out into seperate case statements
      stack[0].intValue = (*pc - OP_iconst_0);
      stack++;
      pc++;
      break;
    case OP_fconst_0:
      stack[0].floatValue = 0.0f;
      stack++;
      pc++;
      break;
    case OP_fconst_1:
      stack[0].floatValue = 1.0f;
      stack++;
      pc++;
      break;
    case OP_fconst_2:
      stack[0].floatValue = 2.0f;
      stack++;
      pc++;
      break;
    case OP_bipush:
      stack[0].intValue = ((signed char *)pc)[1];
      stack++;
      pc += 2;
      break;
    case OP_sipush:
      stack[0].intValue = getInt16(&pc[1]);
      stack++;
      pc += 3;
      break;
    case OP_ldc:
      *stack = constantToVar(wclass, (uint16)pc[1]);
      stack++;
      pc += 2;
      break;
    case OP_ldc_w:
      *stack = constantToVar(wclass, getUInt16(&pc[1]));
      stack++;
      pc += 3;
      break;
    case OP_iload:
    case OP_fload:
    case OP_aload:
      *stack = var[pc[1]];
      stack++;
      pc += 2;
      break;
    case OP_iload_0:
    case OP_iload_1:
    case OP_iload_2:
    case OP_iload_3:
      *stack = var[*pc - OP_iload_0];
      stack++;
      pc++;
      break;
    case OP_fload_0:
    case OP_fload_1:
    case OP_fload_2:
    case OP_fload_3:
      *stack = var[*pc - OP_fload_0];
      stack++;
      pc++;
      break;
    case OP_aload_0:
    case OP_aload_1:
    case OP_aload_2:
    case OP_aload_3:
      *stack = var[*pc - OP_aload_0];
      stack++;
      pc++;
      break;
    case OP_iaload:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-2].obj;
	i = stack[-1].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	stack[-2].intValue = ((int32 *)WOBJ_arrayStartP(objPtr))[i];
	stack--;
	pc++;
	break;
      }
    case OP_saload:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-2].obj;
	i = stack[-1].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	stack[-2].intValue = (int32)(((int16 *)WOBJ_arrayStartP(objPtr))[i]);
	stack--;
	pc++;
	break;
      }
    case OP_faload:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-2].obj;
	i = stack[-1].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	stack[-2].floatValue = ((float32 *)WOBJ_arrayStartP(objPtr))[i];
	stack--;
	pc++;
	break;
      }
    case OP_aaload:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-2].obj;
	i = stack[-1].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	stack[-2].obj = ((WObject *)WOBJ_arrayStartP(objPtr))[i];
	stack--;
	pc++;
	break;
      }
    case OP_baload:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-2].obj;
	i = stack[-1].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	stack[-2].intValue = (int32)(((char *)WOBJ_arrayStartP(objPtr))[i]);
	stack--;
	pc++;
	break;
      }
    case OP_caload:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-2].obj;
	i = stack[-1].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	stack[-2].intValue = (int32)(((uint16 *)WOBJ_arrayStartP(objPtr))[i]);
	stack--;
	pc++;
	break;
      }
    case OP_astore:
    case OP_istore:
    case OP_fstore:
      stack--;
      var[pc[1]] = *stack;
      pc += 2;
      break;
    case OP_istore_0:
    case OP_istore_1:
    case OP_istore_2:
    case OP_istore_3:
      stack--;
      var[*pc - OP_istore_0] = *stack;
      pc++;
      break;
    case OP_fstore_0:
    case OP_fstore_1:
    case OP_fstore_2:
    case OP_fstore_3:
      stack--;
      var[*pc - OP_fstore_0] = *stack;
      pc++;
      break;
    case OP_astore_0:
    case OP_astore_1:
    case OP_astore_2:
    case OP_astore_3:
      stack--;
      var[*pc - OP_astore_0] = *stack;
      pc++;
      break;
    case OP_iastore:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-3].obj;
	i = stack[-2].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	((int32 *)WOBJ_arrayStartP(objPtr))[i] = stack[-1].intValue;
	stack -= 3;
	pc++;
	break;
      }
    case OP_sastore:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-3].obj;
	i = stack[-2].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	((int16 *)WOBJ_arrayStartP(objPtr))[i] = (int16)stack[-1].intValue;
	stack -= 3;
	pc++;
	break;
      }
    case OP_fastore:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-3].obj;
	i = stack[-2].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	((float32 *)WOBJ_arrayStartP(objPtr))[i] = stack[-1].floatValue;
	stack -= 3;
	pc++;
	break;
      }
    case OP_aastore:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-3].obj;
	i = stack[-2].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	((WObject *)WOBJ_arrayStartP(objPtr))[i] = stack[-1].obj;
	stack -= 3;
	pc++;
	break;
      }
    case OP_bastore:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-3].obj;
	i = stack[-2].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	((char *)WOBJ_arrayStartP(objPtr))[i] = (char)stack[-1].intValue;
	stack -= 3;
	pc++;
	break;
      }
    case OP_castore:
      {
	WObject obj;
	int32 i;
	Var *objPtr;

	obj = stack[-3].obj;
	i = stack[-2].intValue;
	if (obj == 0) goto null_array_error;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	((uint16 *)WOBJ_arrayStartP(objPtr))[i] = (uint16)stack[-1].intValue;
	stack -= 3;
	pc++;
	break;
      }
    case OP_pop:
      stack--;
      pc++;
      break;
    case OP_pop2:
      stack -= 2;
      pc++;
      break;
    case OP_dup:
      stack[0] = stack[-1];
      stack++;
      pc++;
      break;
    case OP_dup_x1:
      stack[0] = stack[-1];
      stack[-1] = stack[-2];
      stack[-2] = stack[0];
      stack++;
      pc++;
      break;
    case OP_dup_x2:
      stack[0] = stack[-1];
      stack[-1] = stack[-2];
      stack[-2] = stack[-3];
      stack[-3] = stack[0];
      stack++;
      pc++;
      break;
    case OP_dup2:
      stack[1] = stack[-1];
      stack[0] = stack[-2];
      stack += 2;
      pc++;
      break;
    case OP_dup2_x1:
      stack[1] = stack[-1];
      stack[0] = stack[-2];
      stack[-1] = stack[-3];
      stack[-2] = stack[1];
      stack[-3] = stack[0];
      stack += 2;
      pc++;
      break;
    case OP_dup2_x2:
      stack[1] = stack[-1];
      stack[0] = stack[-2];
      stack[-1] = stack[-3];
      stack[-2] = stack[-4];
      stack[-3] = stack[1];
      stack[-4] = stack[0];
      stack += 2;
      pc++;
      break;
    case OP_swap:
      {
	Var v;

	v = stack[-2];
	stack[-2] = stack[-1];
	stack[-1] = v;
	pc++;
	break;
      }
    case OP_iadd:
      stack[-2].intValue += stack[-1].intValue;
      stack--;
      pc++;
      break;
    case OP_fadd:
      stack[-2].floatValue += stack[-1].floatValue;
      stack--;
      pc++;
      break;
    case OP_isub:
      stack[-2].intValue -= stack[-1].intValue;
      stack--;
      pc++;
      break;
    case OP_fsub:
      stack[-2].floatValue -= stack[-1].floatValue;
      stack--;
      pc++;
      break;
    case OP_imul:
      stack[-2].intValue *= stack[-1].intValue;
      stack--;
      pc++;
      break;
    case OP_fmul:
      stack[-2].floatValue *= stack[-1].floatValue;
      stack--;
      pc++;
      break;
    case OP_idiv:
      if (stack[-1].intValue == 0)
	goto div_by_zero_error;
      stack[-2].intValue /= stack[-1].intValue;
      stack--;
      pc++;
      break;
    case OP_fdiv:
      if (stack[-1].floatValue == 0.0f)
	goto div_by_zero_error;
      stack[-2].floatValue /= stack[-1].floatValue;
      stack--;
      pc++;
      break;
    case OP_irem:
      if (stack[-1].intValue == 0)
	goto div_by_zero_error;
      stack[-2].intValue = stack[-2].intValue % stack[-1].intValue;
      stack--;
      pc++;
      break;
    case OP_frem:
      {
	float32 f;

	if (stack[-1].floatValue == 0.0f)
	  goto div_by_zero_error;
	f = stack[-2].floatValue / stack[-1].floatValue;
	f = (float32)((int32)f);
	f *= stack[-1].floatValue;
	stack[-2].floatValue = stack[-2].floatValue - f;
	stack--;
	pc++;
	break;
      }
    case OP_ineg:
      stack[-1].intValue = - stack[-1].intValue;
      pc++;
      break;
    case OP_fneg:
      stack[-1].floatValue = - stack[-1].floatValue;
      pc++;
      break;
    case OP_ishl:
      stack [-2].intValue = ishl (stack [-2].intValue, stack [-1].intValue);
      stack--;
      pc++;
      break;
    case OP_ishr:
      stack [-2].intValue = ishr (stack [-2].intValue, stack [-1].intValue);
      stack--;
      pc++;
      break;
    case OP_iushr:
      stack [-2].intValue = iushr (stack [-2].intValue, stack [-1].intValue);
      stack--;
      pc++;
      break;
    case OP_iand:
      stack[-2].intValue &= stack[-1].intValue;
      stack--;
      pc++;
      break;
    case OP_ior:
      stack[-2].intValue |= stack[-1].intValue;
      stack--;
      pc++;
      break;
    case OP_ixor:
      stack[-2].intValue ^= stack[-1].intValue;
      stack--;
      pc++;
      break;
    case OP_iinc:
      var[pc[1]].intValue += (signed char)pc[2];
      pc += 3;
      break;
    case OP_i2f:
      stack[-1].floatValue = (float32)stack[-1].intValue;
      pc++;
      break;
    case OP_f2i:
      {
	float32 f;

	f = stack[-1].floatValue;
	if (f > 2147483647.0)
	  stack[-1].intValue = 0x7FFFFFFF;
	else if (f < -2147483648.0)
	  stack[-1].intValue = 0x80000000;
	else
	  stack[-1].intValue = (int32)f;
	pc++;
	break;
      }
    case OP_i2b:
      stack[-1].intValue = (int32)((signed char)(stack[-1].intValue & 0xFF));
      pc++;
      break;
    case OP_i2c:
      stack[-1].intValue = (int32)((uint16)(stack[-1].intValue & 0xFFFF));
      pc++;
      break;
    case OP_i2s:
      stack[-1].intValue = (int32)((int16)(stack[-1].intValue & 0xFFFF));
      pc++;
      break;
    case OP_fcmpl:
    case OP_fcmpg:
      {
	float32 f;

	// NOTE: NaN values are currently not supported - NaN in either
	// value should return 1 or 0 depending on the opcode
	f = stack[-2].floatValue - stack[-1].floatValue;
	if (f > 0.0f)
	  stack[-2].intValue = 1;
	else if (f < 0.0f)
	  stack[-2].intValue = -1;
	else
	  stack[-2].intValue = 0;
	stack--;
	pc++;
	break;
      }
    case OP_ifeq:
      if (stack[-1].intValue == 0)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack--;
      break;
    case OP_ifne:
      if (stack[-1].intValue != 0)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack--;
      break;
    case OP_iflt:
      if (stack[-1].intValue < 0)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack--;
      break;
    case OP_ifge:
      if (stack[-1].intValue >= 0)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack--;
      break;
    case OP_ifgt:
      if (stack[-1].intValue > 0)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack--;
      break;
    case OP_ifle:
      if (stack[-1].intValue <= 0)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack--;
      break;
    case OP_if_icmpeq:
      if (stack[-2].intValue == stack[-1].intValue)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack -= 2;
      break;
    case OP_if_icmpne:
      if (stack[-2].intValue != stack[-1].intValue)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack -= 2;
      break;
    case OP_if_icmplt:
      if (stack[-2].intValue < stack[-1].intValue)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack -= 2;
      break;
    case OP_if_icmpge:
      if (stack[-2].intValue >= stack[-1].intValue)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack -= 2;
      break;
    case OP_if_icmpgt:
      if (stack[-2].intValue > stack[-1].intValue)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack -= 2;
      break;
    case OP_if_icmple:
      if (stack[-2].intValue <= stack[-1].intValue)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack -= 2;
      break;
    case OP_if_acmpeq:
      if (stack[-2].obj == stack[-1].obj)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack -= 2;
      break;
    case OP_if_acmpne:
      if (stack[-2].obj != stack[-1].obj)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack -= 2;
      break;
    case OP_goto:
      pc += getInt16(&pc[1]);
      break;
    case OP_jsr:
      stack[0].pc = pc + 3;
      stack++;
      pc += getInt16(&pc[1]);
      break;
    case OP_ret:
      pc = var[pc[1]].pc;
      break;
    case OP_tableswitch:
      {
	int32 key, low, high, defaultOff;
	uchar *npc;

	key = stack[-1].intValue;
	npc = pc + 1;
	npc += (4 - ((npc - METH_code(method)) % 4)) % 4;
	defaultOff = getInt32(npc);
	npc += 4;
	low = getInt32(npc);
	npc += 4;
	high = getInt32(npc);
	npc += 4;
	if (key < low || key > high)
	  pc += defaultOff;
	else
	  pc += getInt32(&npc[(key - low) * 4]);
	stack--;
	break;
      }
    case OP_lookupswitch:
      {
	int32 i, key, low, mid, high, npairs, defaultOff;
	uchar *npc;

	key = stack[-1].intValue;
	npc = pc + 1;
	npc += (4 - ((npc - METH_code(method)) % 4)) % 4;
	defaultOff = getInt32(npc);
	npc += 4;
	npairs = getInt32(npc);
	npc += 4;

	// binary search
	if (npairs > 0)
	  {
	    low = 0;
	    high = npairs;
	    while (1)
	      {
		mid = (high + low) / 2;
		i = getInt32(npc + (mid * 8));
		if (key == i)
		  {
		    pc += getInt32(npc + (mid * 8) + 4); // found
		    break;
		  }
		if (mid == low)
		  {
		    pc += defaultOff; // not found
		    break;
		  }
		if (key < i)
		  high = mid;
		else
		  low = mid;
	      }
	  }
	else
	  pc += defaultOff; // no pairs
	stack--;
	break;
      }
    case OP_ireturn:
    case OP_freturn:
    case OP_areturn:
    case OP_return:
      if (*pc != OP_return)
      {
	returnedValue = stack[-1];
	pushReturnedValue = 1;
      }
      else
	pushReturnedValue = 0;
      goto methodreturn;
    case OP_getfield:
      {
	WClassField *field;
	WObject obj;

	// guich@200: modified to handle 64bit numbers

	field = getFieldByIndex(wclass, getUInt16(&pc[1]));
	if (!field)
	  goto error;
	stack--;
	//obj = stack[-1].obj;
	obj = stack[0].obj;
	if (obj == 0)
	  goto null_obj_error;

#ifdef WITH_64BITS
	if (fieldIs64wide(wclass, field))
	{
	  stack[0] = WOBJ_var(obj, field->var.varOffset);
	  stack[1] = WOBJ_var(obj, field->var2.varOffset);
	  stack+=2;
	}
	else
#endif   /* WITH_64BITS */
	{
	  //stack[-1] = WOBJ_var(obj, field->var.varOffset);
	  stack[0] = WOBJ_var(obj, field->var.varOffset);
	  stack++;
	}
	pc += 3;
	break;
      }
    case OP_putfield:
      {
	WClassField *field;
	WObject obj;

	field = getFieldByIndex(wclass, getUInt16(&pc[1]));
	if (!field)
	  goto error;

#ifdef WITH_64BITS

	// guich@200: modified to handle 64bit numbers

	if (fieldIs64wide(wclass, field))
	{
	  stack -= 3;
	  obj = stack[0].obj;
	  if (obj == 0)
	    goto null_obj_error;
	  WOBJ_var(obj, field->var.varOffset) = stack[1];
	  WOBJ_var(obj, field->var2.varOffset) = stack[2];
	}
	else

#endif    /* WITH_64BITS */

	{
	  stack -= 2;
	  obj = stack[0].obj;
	  if (obj == 0)
	    goto null_obj_error;
	  WOBJ_var(obj, field->var.varOffset) = stack[1];
	}
	pc += 3;
	break;
      }
    case OP_getstatic:
      {
	WClassField *field;

	field = getFieldByIndex(wclass, getUInt16(&pc[1]));
	if (!field)
	  goto error;

#ifdef WITH_64BITS

	// guich@200: modified to handle 64bit numbers

	if (fieldIs64wide(wclass, field))
	{
	  stack[0] = field->var2.staticVar;
	  stack[1] = field->var.staticVar;
	  stack+=2;
	}
	else

#endif   /* WITH_64BITS */

	{
	  stack[0] = field->var.staticVar;
	  stack++;
	}
	pc += 3;
	break;
      }
    case OP_putstatic:
      {
	WClassField *field;

	field = getFieldByIndex(wclass, getUInt16(&pc[1]));
	if (!field)
	  goto error;

	LOCK_CLASS_HEAP

#ifdef WITH_64BITS

	  // guich@200: modified to handle 64bit numbers

	  if (fieldIs64wide(wclass, field))
	  {
	    field->var.staticVar = stack[-1];
	    field->var2.staticVar = stack[-2];
	    stack-=2;
	  }
	  else

#endif    /* WITH_64BITS */

	  {
	    field->var.staticVar = stack[-1];
	    stack--;
	  }
	UNLOCK_CLASS_HEAP
	  pc += 3;
	break;
      }
    case OP_invokeinterface:
    case OP_invokestatic:
    case OP_invokevirtual:
    case OP_invokespecial:
      {
	int32 i;
	uint16 nparam, classIndex, methodIndex, nameAndTypeIndex;
	int methodNameValid;
	WObject obj;
	WClass *iclass;
	WClassMethod *imethod;
	UtfString methodName, methodDesc;
#ifdef QUICKBIND
	ConsOffset offset;
	int32 methodMapNum;
	int searchType;
#endif

	methodNameValid = 0;
	methodIndex = getUInt16(&pc[1]);
#ifdef QUICKBIND
	offset = CONS_offset(wclass, methodIndex);
	if (offset & CONS_boundBit)
	  {
	    offset &= CONS_boundOffsetMask;
	    methodMapNum = offset >> CONS_boundMethodShift;
	    classIndex = offset & CONS_boundClassMask;
	    iclass = getClassByIndex(wclass, classIndex);
	    if (iclass == NULL)
	      goto methoderror;
	  }
	else
	  {
	    classIndex = CONS_classIndex(wclass, methodIndex);
	    iclass = getClassByIndex(wclass, classIndex);
	    if (iclass == NULL)
	      goto methoderror;
	    nameAndTypeIndex = CONS_nameAndTypeIndex(wclass, methodIndex);
	    methodName = getUtfString(wclass, CONS_nameIndex(wclass, nameAndTypeIndex));
	    methodDesc = getUtfString(wclass, CONS_typeIndex(wclass, nameAndTypeIndex));
	    methodNameValid = 1;
	    if (*pc == OP_invokevirtual)
	      searchType = SEARCH_ALL;
	    else
	      searchType = SEARCH_THISCLASS;
	    methodMapNum = getMethodMapNum(iclass, methodName, methodDesc, searchType);
	    if (methodMapNum < 0)
	      goto methoderror;
	    // adaptive quickbind for methods - bind the constant to the
	    // method num and class index if it fits in the constant and if
	    // the class is not an interface. If the class is an interface, we
	    // need the name and desc of the method later, so we can't bind over
	    // the constant that contains the nameAndTypeIndex
	    if (!WCLASS_isInterface(iclass) && methodMapNum <= MAX_boundMethodNum &&
		classIndex <= MAX_boundClassIndex)
	      {
		LOCK_CLASS_HEAP
		  CONS_offset(wclass, methodIndex) = CONS_boundBit |
		  (methodMapNum << CONS_boundMethodShift) | classIndex;
		UNLOCK_CLASS_HEAP
		  }
	  }
	imethod = getMethodByMapNum(iclass, NULL, (uint16)methodMapNum);
	if (imethod == NULL)
	  goto methoderror;
#else
	classIndex = CONS_classIndex(wclass, methodIndex);
	iclass = getClassByIndex(wclass, classIndex);
	if (iclass == NULL)
	  goto methoderror;
	nameAndTypeIndex = CONS_nameAndTypeIndex(wclass, methodIndex);
	methodName = getUtfString(wclass, CONS_nameIndex(wclass, nameAndTypeIndex));
	methodDesc = getUtfString(wclass, CONS_typeIndex(wclass, nameAndTypeIndex));
	methodNameValid = 1;
	imethod = getMethod(iclass, methodName, methodDesc, NULL);
	if (imethod == NULL)
	  goto methoderror;
#endif
	// get object reference and inc param count (if needed)
	nparam = imethod->numParams;
	obj = 0;
	// FIX - double-check sometime, can we do an interface invoke on
	// a static method? if so, we should check the method for static,
	// not the invoke type here
	if (*pc != OP_invokestatic)
	  {
	    nparam++;
	    obj = stack[- (int32)nparam].obj;
	    if (obj == 0)
	      goto null_obj_error;
	  }

	// skip Object <init> method (and pop off object reference)
	if (iclass->numSuperClasses == 0 && method->isInit)
	  {
	    stack -= nparam;
	    pc += 3;
	    break;
	  }

#ifdef QUICKBIND
	if (*pc == OP_invokevirtual)
	  {
	    iclass = (WClass *)WOBJ_class(obj);
	    imethod = getMethodByMapNum(iclass, &iclass, (uint16)methodMapNum);
	    if (imethod == NULL)
	      goto methoderror; //classes are out of sync/corrupt
	    pc += 3;
	  }
	else if (*pc == OP_invokeinterface)
	  {
	    iclass = (WClass *)WOBJ_class(obj);
	    nameAndTypeIndex = CONS_nameAndTypeIndex(wclass, methodIndex);
	    methodName = getUtfString(wclass, CONS_nameIndex(wclass, nameAndTypeIndex));
	    methodDesc = getUtfString(wclass, CONS_typeIndex(wclass, nameAndTypeIndex));
	    methodMapNum = getMethodMapNum(iclass, methodName, methodDesc, SEARCH_ALL);
	    if (methodMapNum < 0)
	      goto methoderror;
	    imethod = getMethodByMapNum(iclass, &iclass, (uint16)methodMapNum);
	    if (imethod == NULL)
	      goto methoderror; //classes are out of sync/corrupt
	    pc += 5;
	  }
	else
	  pc += 3;
#else
	if (*pc == OP_invokevirtual || *pc == OP_invokeinterface)
	  {
	    iclass = (WClass *)WOBJ_class(obj);
				// get method (and class if virtual)
	    imethod = getMethod(iclass, methodName, methodDesc, &iclass);
	    if (imethod == NULL)
	      goto methoderror; //classes are out of sync/corrupt
	  }

	if (*pc == OP_invokeinterface)
	  pc += 5;
	else
	  pc += 3;
#endif

	// push return stack frame:
	//
	// program counter pointer
	// local var pointer
	// local stack pointer

	if ((METH_accessFlags(imethod) & ACCESS_NATIVE) > 0)
	  {
	    if (imethod->code.nativeFunc == NULL)
	      goto methoderror;
				// return stack frame plus native method active frame
	    if (vmStackPtr + 3 + nparam + 3 >= vmStackSize)
	      goto stack_overflow_error;
	  }
	else
	  {
	    if (imethod->code.codeAttr == NULL)
	      goto methoderror;
				// return stack frame plus active frame
	    if (vmStackPtr + 3 + METH_maxLocals(imethod) +
		METH_maxStack(imethod) + 2 >= vmStackSize)
	      goto stack_overflow_error;
	  }

	vmStack[vmStackPtr++].pc = pc;
	vmStack[vmStackPtr++].refValue = var;
	vmStack[vmStackPtr++].refValue = stack - nparam;

	// push params into local vars of next frame
	for (i = 0; i < nparam; i++)
	  {
	    vmStack[vmStackPtr + nparam - i - 1] = stack[-1];
	    stack--;
	  }

	wclass = iclass;
	method = imethod;

	// execute native method
	if ((METH_accessFlags(method) & ACCESS_NATIVE) > 0)
	  {
				// the active frame for a native method is:
				//
				// param 1
				// ...
				// param N
				// num params
				// method pointer
				// class pointer
	    vmStackPtr += nparam;
	    vmStack[vmStackPtr++].intValue = nparam;
	    vmStack[vmStackPtr++].refValue = method;
	    vmStack[vmStackPtr++].refValue = wclass;

#ifdef DEBUG
	    {
	      UtfString c, m;
	      struct timeval tv;
	      c = getUtfString(wclass, wclass->classNameIndex);
	      m = getUtfString(wclass, METH_nameIndex(method));

	      gettimeofday (&tv, NULL);
	      DPRINTF (">>> [%d.%d] ", tv.tv_sec, tv.tv_usec);
	      DPRINTF("invoking native method: m[%s.%s]\n",
		      UTF2CSTR(&c),
		      UTF2CSTR(&m)
		      );
	    }
#endif

	    // guich@200 - if the native func returns long or double,
	    // it sets the pushReturnedValue to 2 and the returnedValue2 var
	    returnedValue = method->code.nativeFunc(stack);

	    if (vmStatus.errNum != 0)
	      goto error; // error occured during native method

#ifdef WITH_64BITS
	    // add by guich@200 for long/double support
	    if (pushReturnedValue != 2)
#endif    /* WITH_64BITS */
	    {
	      if (method->returnsValue)
		pushReturnedValue = 1;
	      else
		pushReturnedValue = 0;
	    }
 	    goto methodreturn;
	  }

	goto methodinvoke;
      methoderror:
	if (methodNameValid)
	  VmError(ERR_CantFindMethod, iclass, &methodName, &methodDesc);
	else
	  VmQuickError(ERR_CantFindMethod);
	goto error;
      }
    case OP_new:
      {
	uint16 classIndex;

	classIndex = getUInt16(&pc[1]);
	stack[0].obj = createObject(getClassByIndex(wclass, classIndex));
	stack++;
	pc += 3;
	break;
      }
    case OP_newarray:
      stack[-1].obj = createArrayObject((int32)pc[1], stack[-1].intValue);
      pc += 2;
      break;
    case OP_anewarray:
      stack[-1].obj = createArrayObject(1, stack[-1].intValue);
      pc += 3;
      break;
    case OP_arraylength:
      {
	WObject obj;

	obj = stack[-1].obj;
	if (obj == 0)
	  goto null_array_error;
	stack[-1].intValue = WOBJ_arrayLen(obj);
	pc++;
	break;
      }
    case OP_instanceof:
    case OP_checkcast:
      {
	WObject obj;
	uint16 classIndex;
	UtfString className;
	WClass *source, *target;
	int comp;

	obj = stack[-1].obj;
	if (obj == 0)
	  {
	    if (*pc == OP_instanceof)
	      stack[-1].intValue = 0;
	    pc += 3;
	    break;
	  }
	source = WOBJ_class(obj);
	classIndex = getUInt16(&pc[1]);
	target = getClassByIndex(wclass, classIndex);
	if (target)
	  {
	    className = getUtfString(target, target->classNameIndex);
	    comp = compatible(source, target); // target is not array
	  }
	else
	  {
				// target is either array or target class was not found
				// if either of these cases is true, the index couldn't be
				// bound to a pointer, so we still have a pointer into the
				// constant pool and can use the string reference in the constant
	    className = getUtfString(wclass, CONS_nameIndex(wclass, classIndex));
	    if (className.len > 1 && className.str[0] == '[')
	      comp = compatibleArray(obj, className); // target is array
	    else
	      goto error; // target class not found
	  }
	if (*pc == OP_checkcast)
	  {
	    if (!comp)
	      {
		VmError(ERR_ClassCastException, source, &className, NULL);
		goto error;
	      }
	  }
	else
	  stack[-1].intValue = comp;
	pc += 3;
	break;
      }
    case OP_wide:
      pc++;
      switch (*pc)
	{
	case OP_iload:
	case OP_fload:
	case OP_aload:
	  stack[0] = var[getUInt16(&pc[1])];
	  stack++;
	  pc += 3;
	  break;
	case OP_astore:
	case OP_istore:
	case OP_fstore:
	  var[getUInt16(&pc[1])] = stack[-1];
	  stack--;
	  pc += 3;
	  break;
	case OP_iinc:
	  var[getUInt16(&pc[1])].intValue += getInt16(&pc[3]);
	  pc += 5;
	  break;
	case OP_ret:
	  pc = var[getUInt16(&pc[1])].pc;
	  break;
	}
      break;
    case OP_multianewarray:
      {
	uint16 classIndex;
	UtfString className;
	int32 ndim;
	char *cstr;

	classIndex = getUInt16(&pc[1]);
	// since arrays do not have associated classes which could be bound
	// to the class constant, we can safely access the name string in
	// the constant
	className = getUtfString(wclass, CONS_nameIndex(wclass, classIndex));
	ndim = (int32)pc[3];
	cstr = &className.str[1];
	stack -= ndim;
	stack[0].obj = createMultiArray(ndim, cstr, stack);
	stack++;
	pc += 4;
	break;
      }
    case OP_ifnull:
      if (stack[-1].obj == 0)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack--;
      break;
    case OP_ifnonnull:
      if (stack[-1].obj != 0)
	pc += getInt16(&pc[1]);
      else
	pc += 3;
      stack--;
      break;
    case OP_goto_w:
      pc += getInt32(&pc[1]);
      break;
    case OP_jsr_w:
      stack[0].pc = pc + 5;
      pc += getInt32(&pc[1]);
      stack++;
      break;

#ifdef WITH_THREAD
      // *** Isao's Multithread implementation START ***

      //"OP_monitorenter" & "OP_monitorexit" are used by "sync blocks",
      //not by "synch methods".
      case OP_monitorenter: // unsupported (yet)
      {
	WObject obj;

	obj = stack[-1].obj;
	if(obj == 0)
	  goto null_obj_error;

	if (!enterMonitor(obj))
	  {
	    goto methodreturn;
	  }

	stack--;
	pc++;
	break;
      }

      case OP_monitorexit: // unsupported (yet)
      {
	WObject obj;

	obj = stack[-1].obj;
	if(obj == 0)
	  goto null_obj_error;

	if (!exitMonitor(obj))
	  {
	    goto methodreturn;
	  }

	stack--;
	pc++;
	break;
        }
        // *** Isao's Multithread implementation END ***
#endif

#ifdef WITH_64BITS

      /*
	BEGIN From SuperWaba 2.0beta1
	guich@200 - added support of double types natively
      */
      case OP_dconst_0:
	double2vars(0.0,stack);
	stack+=2;
	pc++;
	break;
      case OP_dconst_1:
	double2vars(1.0,stack);
	stack+=2;
	pc++;
	break;
      case OP_dload:
      case OP_lload:
      {
	Var *vptr;
	vptr = &var[pc[1]];
	copy64(vptr,stack);
	stack+=2;
	pc += 2;
	break;
      }
      case OP_dload_0:
      case OP_dload_1:
      case OP_dload_2:
      case OP_dload_3:
      {
	Var *vptr;
	vptr = &var[*pc - OP_dload_0];
	copy64(vptr,stack);
	stack+=2;
	pc++;
	break;
      }
      case OP_daload:
      case OP_laload:
      {
	WObject obj;
	int32 i;
	Var *objPtr;
	Var *vptr;

	stack -= 2;
	obj = stack[0].obj;
	if (obj == 0) goto null_array_error;
	i = stack[1].intValue;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	vptr = (WOBJ_arrayStartP(objPtr))+(i*2);
	copy64(vptr,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_dastore:
      case OP_lastore:
      {
	WObject obj;
	int32 i;
	Var *objPtr;
	Var *vptr;

	stack -= 4;
	obj = stack[0].obj;
	if (obj == 0) goto null_array_error;
	i = stack[1].intValue;
	objPtr = objectPtr(obj);
	if (i < 0 || i >= WOBJ_arrayLenP(objPtr)) goto index_range_error;
	vptr = (WOBJ_arrayStartP(objPtr))+(i*2);
	copy64(stack+2,vptr);
	pc++;
	break;
      }
      case OP_dstore:
      case OP_lstore:
      {
	Var *vptr;
	vptr = &var[pc[1]];
	stack-=2;
	copy64(stack,vptr);
	pc += 2;
	break;
      }
      case OP_dstore_0:
      case OP_dstore_1:
      case OP_dstore_2:
      case OP_dstore_3:
      {
	Var *vptr;
	vptr = &var[*pc - OP_dstore_0];
	stack -= 2;
	copy64(stack,vptr);
	pc++;
	break;
      }
      case OP_dadd:
      {
	float64 d1, d2, d;
	stack -= 4;
	d1 = vars2double(stack+2);
	d2 = vars2double(stack);
	d = d1 + d2;
	double2vars(d,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_dsub:
      {
	float64 d1, d2, d;
	stack -= 4;
	d1 = vars2double(stack+2);
	d2 = vars2double(stack);
	d = d2 - d1;
	double2vars(d,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_dmul:
      {
	float64 d1, d2, d;
	stack -= 4;
	d1 = vars2double(stack+2);
	d2 = vars2double(stack);
	d = d2 * d1;
	double2vars(d,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_ddiv:
      {
	float64 d1, d2, d;
	stack -= 4;
	d1 = vars2double(stack+2);
	d2 = vars2double(stack);
	if (d1 == 0.0)
	  goto div_by_zero_error;
	d = d2 / d1;
	double2vars(d,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_drem:
      {
	float64 d1, d2, d;
	//float64HiLo fhl;
	stack -= 4;
	d1 = vars2double(stack+2);
	d2 = vars2double(stack);
	if (d1 == 0.0)
	  goto div_by_zero_error;
	d = d2 - (int32)( d2 / d1 ) * d1;

	double2vars( d, stack );
	stack += 2;
	pc++;
	break;
      }
      case OP_dneg:
      {
	float64 d;
	stack -= 2;
	d = vars2double(stack);
	if (d != 0.0)
	  d = -d;
	double2vars(d,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_i2d:
      {
	stack--;
	double2vars( (float64)(stack[0].intValue), stack );
	stack += 2;
	pc++;
	break;
      }
      case OP_f2d:
	stack--;
	double2vars( (float64)(stack[0].floatValue), stack );
	stack+=2;
	pc++;
	break;
      case OP_d2i:
	stack-=2;
	stack[0].intValue = (int32)vars2double(stack);
	stack++;
	pc++;
	break;
      case OP_d2f:
	stack-=2;
	stack[0].floatValue = (float)vars2double(stack); //To avoid compiler warning.
	stack++;
	pc++;
	break;
      case OP_dcmpl:
      case OP_dcmpg:
      {
	float64 d1, d2;
	// NOTE: NaN values are currently not supported - NaN in either
	// value should return 1 or 0 depending on the opcode
	stack-=4;
	d1 = vars2double(stack);
	d2 = vars2double(stack+2);
	if( d1 > d2 ) {
	  stack[0].intValue =  1;
	} else if( d1 < d2 ) {
	  stack[0].intValue =  -1;
	} else {
	  stack[0].intValue =  0;
	}
	stack++;
	pc++;
	break;
      }
      case OP_dreturn:
      case OP_lreturn:
	returnedValue = stack[-2];
	returnedValue2 = stack[-1];
	pushReturnedValue = 2;
	goto methodreturn;
      case OP_ldc2_w:
	double2vars( CONS_double( wclass, getUInt16(&pc[1])), stack );
	stack+=2;
	pc += 3;
	break;
/// long 64 bits support
      case OP_lconst_0:
	int642vars( 0L, stack);
	stack+=2;
	pc++;
	break;
      case OP_lconst_1:
	int642vars( 1L,stack);
	stack+=2;
	pc++;
	break;
      case OP_lload_0:
      case OP_lload_1:
      case OP_lload_2:
      case OP_lload_3:
      {
	Var *vptr;
	vptr = &var[*pc - OP_lload_0];
	copy64(vptr,stack);
	stack+=2;
	pc++;
	break;
      }
      case OP_lstore_0:
      case OP_lstore_1:
      case OP_lstore_2:
      case OP_lstore_3:
      {
	Var *vptr;
	vptr = &var[*pc - OP_lstore_0];
	stack -= 2;
	copy64(stack,vptr);
	pc++;
	break;
      }
      case OP_ladd:
      {
	int64 ll1, ll2, ll;
	stack -= 4;
	ll1 = vars2int64(stack+2);
	ll2 = vars2int64(stack);
	ll = ll1 + ll2;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_lsub:
      {
	int64 ll1, ll2, ll;
	stack -= 4;
	ll1 = vars2int64(stack+2);
	ll2 = vars2int64(stack);
	ll = ll2 - ll1;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_lmul:
      {
	int64 ll1, ll2, ll;
	stack -= 4;
	ll1 = vars2int64(stack+2);
	ll2 = vars2int64(stack);
	ll = ll1 * ll2;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_ldiv:
      {
	int64 ll1, ll2, ll;
	stack -= 4;
	ll1 = vars2int64(stack+2);
	ll2 = vars2int64(stack);
	if ( ll1 == 0L )
	  goto div_by_zero_error;
	ll = ll2 / ll1;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_lrem:
      {
	int64 ll1, ll2, ll;
	stack -= 4;
	ll1 = vars2int64(stack+2);
	ll2 = vars2int64(stack);
	if ( ll1 == 0L )
	  goto div_by_zero_error;
	ll = ll2 % ll1;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_lneg:
      {
	int64 ll1, /*ll2,*/ ll;
	stack -= 2;
	ll1 = vars2int64(stack);
	if ( ll1 != 0L )
	  ll = - ll1;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_lshl:
      {
	stack -= 3;
	int642vars (lshl (vars2int64 (stack), stack [2].intValue), stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_lshr:
      {
	stack -= 3;
	int642vars (lshr (vars2int64 (stack), stack [2].intValue), stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_lushr:
      {
	stack -= 3;
	int642vars (lushr (vars2int64 (stack), stack [2].intValue), stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_land:
      {
	int64 ll1, ll2, ll;
	stack -= 4;
	ll1 = vars2int64(stack);
	ll2 = vars2int64(stack+2);
	ll = ll1 & ll2;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_lor:
      {
	int64 ll1, ll2, ll;
	stack -= 4;
	ll1 = vars2int64(stack);
	ll2 = vars2int64(stack+2);
	ll = ll1 | ll2;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_lxor:
      {
	int64 ll1, ll2, ll;
	stack -= 4;
	ll1 = vars2int64(stack);
	ll2 = vars2int64(stack+2);
	ll = ll1 ^ ll2;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_i2l:
      {
	int64 /*ll1, ll2,*/ ll;
	stack -= 1;
	ll = stack[0].intValue;
	int642vars(ll,stack);
	stack += 2;
	pc++;
	break;
      }
      case OP_l2i:
	stack -= 2;
	stack[0].intValue = (int)vars2int64(stack); //To avoid compiler warning.
	stack++;
	pc++;
	break;
      case OP_l2f:
	stack -= 2;
	stack[0].floatValue = (float32)vars2int64(stack);
	stack++;
	pc++;
	break;
      case OP_f2l:
	stack -= 1;
	int642vars((int64)stack[0].floatValue,stack);
	stack += 2;
	pc++;
	break;
      case OP_l2d:
	stack -= 2;
	double2vars((float64)vars2int64(stack),stack);
	stack+=2;
	pc++;
	break;
      case OP_d2l:
	stack -= 2;
	int642vars((int64)vars2double(stack),stack);
	stack += 2;
	pc++;
	break;
      case OP_lcmp:
      {
	int64 ll1, ll2;
	stack-=4;
	ll1 = vars2int64(stack);
	ll2 = vars2int64(stack+2);
     	if( ll1 > ll2 ) {
	  stack[0].intValue =  1;
	} else if( ll1 < ll2 ) {
	  stack[0].intValue =  -1;
	} else {
	  stack[0].intValue =  0;
	}
	stack++;
	pc++;
	break;
      }

      /*
	END From SuperWaba 2.0beta1
	guich@200 - added support of double types natively
      */

#else   /* WITH_64BITS */

      /* without 64 bits support */
      /* all these OP code are un-supported */

    case OP_lconst_0:
    case OP_lconst_1:
    case OP_dconst_0:
    case OP_dconst_1:
    case OP_ldc2_w:
    case OP_lload:
    case OP_dload:
    case OP_lload_0:
    case OP_lload_1:
    case OP_lload_2:
    case OP_lload_3:
    case OP_dload_0:
    case OP_dload_1:
    case OP_dload_2:
    case OP_dload_3:
    case OP_laload:
    case OP_daload:
    case OP_lstore:
    case OP_dstore:
    case OP_lstore_0:
    case OP_lstore_1:
    case OP_lstore_2:
    case OP_lstore_3:
    case OP_dstore_0:
    case OP_dstore_1:
    case OP_dstore_2:
    case OP_dstore_3:
    case OP_lastore:
    case OP_dastore:
    case OP_ladd:
    case OP_dadd:
    case OP_lsub:
    case OP_dsub:
    case OP_lmul:
    case OP_dmul:
    case OP_ldiv:
    case OP_ddiv:
    case OP_lrem:
    case OP_drem:
    case OP_lneg:
    case OP_dneg:
    case OP_lshl:
    case OP_lshr:
    case OP_lushr:
    case OP_land:
    case OP_lor:
    case OP_lxor:
    case OP_i2l:
    case OP_i2d:
    case OP_l2i:
    case OP_l2f:
    case OP_l2d:
    case OP_f2l:
    case OP_f2d:
    case OP_d2i:
    case OP_d2l:
    case OP_d2f:
    case OP_lcmp:
    case OP_dcmpl:
    case OP_dcmpg:
    case OP_lreturn:
    case OP_dreturn:

#endif    /* WITH_64BITS */

      // NOTE: this is the full list of unsupported opcodes. Adding all
      // these cases here does not cause the VM executable code to be any
      // larger, it just makes sure that the compiler uses a jump table
      // with no spaces in it to make sure performance is as good as we
      // can get (tested under Codewarrior for PalmOS).

    case OP_athrow:
    default:
      VmQuickError(ERR_BadOpcode);
      goto error;
    }

  goto step;

 stack_overflow_error:
  VmQuickError(ERR_StackOverflow);
  goto error;
 null_obj_error:
  VmQuickError(ERR_NullObjectAccess);
  goto error;
 div_by_zero_error:
  VmQuickError(ERR_DivideByZero);
  goto error;
 index_range_error:
  VmQuickError(ERR_IndexOutOfRange);
  goto error;
 null_array_error:
  VmQuickError(ERR_NullArrayAccess);
  goto error;
 error:
  vmStackPtr = baseFramePtr;
#ifdef WITH_THREAD_NATIVE
  return 0;
#else
  return;
#endif

#ifdef DONT_UNWIND_ON_ERROR
  {
    if (method->returnsValue)
      {
	returnedValue.obj = 0;
	pushReturnedValue = 1;
      }
    else
      pushReturnedValue = 0;
    goto methodreturn;
  }
#endif

 methodreturn:
  // pop frame and restore state

#ifdef WITH_THREAD
  if((METH_accessFlags(method) & ACCESS_SYNCHRONIZED) > 0)
    {
      exitFromSyncronized(method);
    }
#endif

  if ((METH_accessFlags(method) & ACCESS_NATIVE) > 0)
    {
      vmStackPtr -= 2;
      vmStackPtr -= vmStack[--vmStackPtr].intValue;
    }
  else
    vmStackPtr -= METH_maxLocals(method) + METH_maxStack(method) + 2;

  if (vmStackPtr == baseFramePtr + 3)
    {
      // fully completed execution
      vmStackPtr = baseFramePtr;
#ifdef WITH_THREAD_NATIVE
      free(vmStack);
      free(nmStack);
      return 0;
#else
      return;
#endif
    }
  stack = (Var *)vmStack[--vmStackPtr].refValue;
  if (pushReturnedValue)
    {
      stack[0] = returnedValue;
      stack++;
    }
  var = (Var *)vmStack[--vmStackPtr].refValue;
  pc = vmStack[--vmStackPtr].pc;
  wclass = (WClass *)vmStack[vmStackPtr - 1].refValue;
  method = (WClassMethod *)vmStack[vmStackPtr - 2].refValue;
  goto step;
}



/*
   Local Variables:
   c-file-style: "smartdata"
   End:
*/
