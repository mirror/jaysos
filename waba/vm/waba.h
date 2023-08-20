/* $Id: waba.h,v 1.17 2002/01/22 03:43:22 monaka Exp $ */

#ifndef __WABA_H__
#define __WABA_H__

#include "config.h"

#if defined(JAYSOS)
#include "jaysos/nmjaysos_a.h"
#elif defined(PALMOS)
#include "palm/nmpalm_a.h"
#elif defined(WIN32)
#include "win32/nmwin32_a.h"
#elif defined(WINCE)
#include "win32/nmwin32_a.h"
#elif defined(LINUX)
#include "linux/nm_linux_a.h"
#elif defined(TOPPERS)
#include "toppers/nmtoppers_a.h"
#endif

/* define FASTANDBIG if you want to have a fastest program,
   but a more big footprint                                 */

/* in our case, we want a small footprint first */
#undef FASTANDBIG

//#define SMALLMEM 1

/*  #if defined(PALMOS) */
/*  #include "palm/nmpalm_a.c" */
/*  #elif defined(WIN32) */
/*  #include "win32/nmwin32_a.c" */
/*  #elif defined(LINUX) */
/*  #include "linux/nm_linux_a.c" */
/*  #endif */

/*
"True words are not beautiful, beautiful words are not true.
 The good are not argumentative, the argumentative are not good.
 Knowers do not generalize, generalists do not know.
 Sages do not accumulate anything but give everything to others,
 having more the more they give.
 The Way of heaven helps and does not harm.
 The Way for humans is to act without contention."

 - Lao-tzu, Tao Te Ching circa 500 B.C.
*/

/* define the constants used in waba programs, like keys, events... */
#define WABA_KEY_PAGE_UP    75000
#define WABA_KEY_PAGE_DOWN  75001
#define WABA_KEY_HOME       75002
#define WABA_KEY_END        75003
#define WABA_KEY_UP         75004
#define WABA_KEY_DOWN       75005
#define WABA_KEY_LEFT       75006
#define WABA_KEY_RIGHT      75007
#define WABA_KEY_INSERT     75008
#define WABA_KEY_ENTER      75009
#define WABA_KEY_TAB        75010
#define WABA_KEY_BACKSPACE  75011
#define WABA_KEY_ESCAPE     75012
#define WABA_KEY_DELETE     75013
#define WABA_KEY_MENU       75014
#define WABA_KEY_COMMAND    75015

#define WABA_EVENT_KEY_PRESS 100
#define WABA_EVENT_PEN_DOWN  200
#define WABA_EVENT_PEN_MOVE  201
#define WABA_EVENT_PEN_UP    202
#define WABA_EVENT_PEN_DRAG  203

//
// TYPES AND METHODS
//

// Access flags
#define ACCESS_PUBLIC       0x0001
#define ACCESS_PRIVATE      0x0002
#define ACCESS_PROTECTED    0x0004
#define ACCESS_STATIC       0x0008
#define ACCESS_FINAL        0x0010
#define ACCESS_SYNCHRONIZED 0x0020
#define ACCESS_VOLATILE     0x0040
#define ACCESS_TRANSIENT    0x0080
#define ACCESS_NATIVE       0x0100
#define ACCESS_INTERFACE    0x0200
#define ACCESS_ABSTRACT     0x0400

// Constant Pool tags
#define CONSTANT_Utf8               1
#define CONSTANT_Integer            3
#define CONSTANT_Float              4
#define CONSTANT_Long               5
#define CONSTANT_Double             6
#define CONSTANT_Class              7
#define CONSTANT_String             8
#define CONSTANT_Fieldref           9
#define CONSTANT_Methodref          10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_NameAndType        12

// Standard Opcodes
#define OP_nop             0
#define OP_aconst_null     1
#define OP_iconst_m1       2
#define OP_iconst_0        3
#define OP_iconst_1        4
#define OP_iconst_2        5
#define OP_iconst_3        6
#define OP_iconst_4        7
#define OP_iconst_5        8
#define OP_lconst_0        9
#define OP_lconst_1        10
#define OP_fconst_0        11
#define OP_fconst_1        12
#define OP_fconst_2        13
#define OP_dconst_0        14
#define OP_dconst_1        15
#define OP_bipush          16
#define OP_sipush          17
#define OP_ldc             18
#define OP_ldc_w           19
#define OP_ldc2_w          20
#define OP_iload           21
#define OP_lload           22
#define OP_fload           23
#define OP_dload           24
#define OP_aload           25
#define OP_iload_0         26
#define OP_iload_1         27
#define OP_iload_2         28
#define OP_iload_3         29
#define OP_lload_0         30
#define OP_lload_1         31
#define OP_lload_2         32
#define OP_lload_3         33
#define OP_fload_0         34
#define OP_fload_1         35
#define OP_fload_2         36
#define OP_fload_3         37
#define OP_dload_0         38
#define OP_dload_1         39
#define OP_dload_2         40
#define OP_dload_3         41
#define OP_aload_0         42
#define OP_aload_1         43
#define OP_aload_2         44
#define OP_aload_3         45
#define OP_iaload          46
#define OP_laload          47
#define OP_faload          48
#define OP_daload          49
#define OP_aaload          50
#define OP_baload          51
#define OP_caload          52
#define OP_saload          53
#define OP_istore          54
#define OP_lstore          55
#define OP_fstore          56
#define OP_dstore          57
#define OP_astore          58
#define OP_istore_0        59
#define OP_istore_1        60
#define OP_istore_2        61
#define OP_istore_3        62
#define OP_lstore_0        63
#define OP_lstore_1        64
#define OP_lstore_2        65
#define OP_lstore_3        66
#define OP_fstore_0        67
#define OP_fstore_1        68
#define OP_fstore_2        69
#define OP_fstore_3        70
#define OP_dstore_0        71
#define OP_dstore_1        72
#define OP_dstore_2        73
#define OP_dstore_3        74
#define OP_astore_0        75
#define OP_astore_1        76
#define OP_astore_2        77
#define OP_astore_3        78
#define OP_iastore         79
#define OP_lastore         80
#define OP_fastore         81
#define OP_dastore         82
#define OP_aastore         83
#define OP_bastore         84
#define OP_castore         85
#define OP_sastore         86
#define OP_pop             87
#define OP_pop2            88
#define OP_dup             89
#define OP_dup_x1          90
#define OP_dup_x2          91
#define OP_dup2            92
#define OP_dup2_x1         93
#define OP_dup2_x2         94
#define OP_swap            95
#define OP_iadd            96
#define OP_ladd            97
#define OP_fadd            98
#define OP_dadd            99
#define OP_isub            100
#define OP_lsub            101
#define OP_fsub            102
#define OP_dsub            103
#define OP_imul            104
#define OP_lmul            105
#define OP_fmul            106
#define OP_dmul            107
#define OP_idiv            108
#define OP_ldiv            109
#define OP_fdiv            110
#define OP_ddiv            111
#define OP_irem            112
#define OP_lrem            113
#define OP_frem            114
#define OP_drem            115
#define OP_ineg            116
#define OP_lneg            117
#define OP_fneg            118
#define OP_dneg            119
#define OP_ishl            120
#define OP_lshl            121
#define OP_ishr            122
#define OP_lshr            123
#define OP_iushr           124
#define OP_lushr           125
#define OP_iand            126
#define OP_land            127
#define OP_ior             128
#define OP_lor             129
#define OP_ixor            130
#define OP_lxor            131
#define OP_iinc            132
#define OP_i2l             133
#define OP_i2f             134
#define OP_i2d             135
#define OP_l2i             136
#define OP_l2f             137
#define OP_l2d             138
#define OP_f2i             139
#define OP_f2l             140
#define OP_f2d             141
#define OP_d2i             142
#define OP_d2l             143
#define OP_d2f             144
#define OP_i2b             145
#define OP_i2c             146
#define OP_i2s             147
#define OP_lcmp            148
#define OP_fcmpl           149
#define OP_fcmpg           150
#define OP_dcmpl           151
#define OP_dcmpg           152
#define OP_ifeq            153
#define OP_ifne            154
#define OP_iflt            155
#define OP_ifge            156
#define OP_ifgt            157
#define OP_ifle            158
#define OP_if_icmpeq       159
#define OP_if_icmpne       160
#define OP_if_icmplt       161
#define OP_if_icmpge       162
#define OP_if_icmpgt       163
#define OP_if_icmple       164
#define OP_if_acmpeq       165
#define OP_if_acmpne       166
#define OP_goto            167
#define OP_jsr             168
#define OP_ret             169
#define OP_tableswitch     170
#define OP_lookupswitch    171
#define OP_ireturn         172
#define OP_lreturn         173
#define OP_freturn         174
#define OP_dreturn         175
#define OP_areturn         176
#define OP_return          177
#define OP_getstatic       178
#define OP_putstatic       179
#define OP_getfield        180
#define OP_putfield        181
#define OP_invokevirtual   182
#define OP_invokespecial   183
#define OP_invokestatic    184
#define OP_invokeinterface 185
#define OP_new             187
#define OP_newarray        188
#define OP_anewarray       189
#define OP_arraylength     190
#define OP_athrow          191
#define OP_checkcast       192
#define OP_instanceof      193
#define OP_monitorenter    194
#define OP_monitorexit     195
#define OP_wide            196
#define OP_multianewarray  197
#define OP_ifnull          198
#define OP_ifnonnull       199
#define OP_goto_w          200
#define OP_jsr_w           201
#define OP_breakpoint      202

#ifdef DEBUG_OPCODE

/* protection */
#define OP_MAX_OP          202

static char * _OP_name [] =
{
  "OP_nop",
  "OP_aconst_null",
  "OP_iconst_m1",
  "OP_iconst_0",
  "OP_iconst_1",
  "OP_iconst_2",
  "OP_iconst_3",
  "OP_iconst_4",
  "OP_iconst_5",
  "OP_lconst_0",
  "OP_lconst_1",
  "OP_fconst_0",
  "OP_fconst_1",
  "OP_fconst_2",
  "OP_dconst_0",
  "OP_dconst_1",
  "OP_bipush",
  "OP_sipush",
  "OP_ldc",
  "OP_ldc_w",
  "OP_ldc2_w",
  "OP_iload",
  "OP_lload",
  "OP_fload",
  "OP_dload",
  "OP_aload",
  "OP_iload_0",
  "OP_iload_1",
  "OP_iload_2",
  "OP_iload_3",
  "OP_lload_0",
  "OP_lload_1",
  "OP_lload_2",
  "OP_lload_3",
  "OP_fload_0",
  "OP_fload_1",
  "OP_fload_2",
  "OP_fload_3",
  "OP_dload_0",
  "OP_dload_1",
  "OP_dload_2",
  "OP_dload_3",
  "OP_aload_0",
  "OP_aload_1",
  "OP_aload_2",
  "OP_aload_3",
  "OP_iaload",
  "OP_laload",
  "OP_faload",
  "OP_daload",
  "OP_aaload",
  "OP_baload",
  "OP_caload",
  "OP_saload",
  "OP_istore",
  "OP_lstore",
  "OP_fstore",
  "OP_dstore",
  "OP_astore",
  "OP_istore_0",
  "OP_istore_1",
  "OP_istore_2",
  "OP_istore_3",
  "OP_lstore_0",
  "OP_lstore_1",
  "OP_lstore_2",
  "OP_lstore_3",
  "OP_fstore_0",
  "OP_fstore_1",
  "OP_fstore_2",
  "OP_fstore_3",
  "OP_dstore_0",
  "OP_dstore_1",
  "OP_dstore_2",
  "OP_dstore_3",
  "OP_astore_0",
  "OP_astore_1",
  "OP_astore_2",
  "OP_astore_3",
  "OP_iastore",
  "OP_lastore",
  "OP_fastore",
  "OP_dastore",
  "OP_aastore",
  "OP_bastore",
  "OP_castore",
  "OP_sastore",
  "OP_pop",
  "OP_pop2",
  "OP_dup",
  "OP_dup_x1",
  "OP_dup_x2",
  "OP_dup2",
  "OP_dup2_x1",
  "OP_dup2_x2",
  "OP_swap",
  "OP_iadd",
  "OP_ladd",
  "OP_fadd",
  "OP_dadd",
  "OP_isub",
  "OP_lsub",
  "OP_fsub",
  "OP_dsub",
  "OP_imul",
  "OP_lmul",
  "OP_fmul",
  "OP_dmul",
  "OP_idiv",
  "OP_ldiv",
  "OP_fdiv",
  "OP_ddiv",
  "OP_irem",
  "OP_lrem",
  "OP_frem",
  "OP_drem",
  "OP_ineg",
  "OP_lneg",
  "OP_fneg",
  "OP_dneg",
  "OP_ishl",
  "OP_lshl",
  "OP_ishr",
  "OP_lshr",
  "OP_iushr",
  "OP_lushr",
  "OP_iand",
  "OP_land",
  "OP_ior",
  "OP_lor",
  "OP_ixor",
  "OP_lxor",
  "OP_iinc",
  "OP_i2l",
  "OP_i2f",
  "OP_i2d",
  "OP_l2i",
  "OP_l2f",
  "OP_l2d",
  "OP_f2i",
  "OP_f2l",
  "OP_f2d",
  "OP_d2i",
  "OP_d2l",
  "OP_d2f",
  "OP_i2b",
  "OP_i2c",
  "OP_i2s",
  "OP_lcmp",
  "OP_fcmpl",
  "OP_fcmpg",
  "OP_dcmpl",
  "OP_dcmpg",
  "OP_ifeq",
  "OP_ifne",
  "OP_iflt",
  "OP_ifge",
  "OP_ifgt",
  "OP_ifle",
  "OP_if_icmpeq",
  "OP_if_icmpne",
  "OP_if_icmplt",
  "OP_if_icmpge",
  "OP_if_icmpgt",
  "OP_if_icmple",
  "OP_if_acmpeq",
  "OP_if_acmpne",
  "OP_goto",
  "OP_jsr",
  "OP_ret",
  "OP_tableswitch",
  "OP_lookupswitch",
  "OP_ireturn",
  "OP_lreturn",
  "OP_freturn",
  "OP_dreturn",
  "OP_areturn",
  "OP_return",
  "OP_getstatic",
  "OP_putstatic",
  "OP_getfield",
  "OP_putfield",
  "OP_invokevirtual",
  "OP_invokespecial",
  "OP_invokestatic",
  "OP_invokeinterface",
  "OP_new",
  "OP_newarray",
  "OP_anewarray",
  "OP_arraylength",
  "OP_athrow",
  "OP_checkcast",
  "OP_instanceof",
  "OP_monitorenter",
  "OP_monitorexit",
  "OP_wide",
  "OP_multianewarray",
  "OP_ifnull",
  "OP_ifnonnull",
  "OP_goto_w",
  "OP_jsr_w",
  "OP_breakpoint"
};

#endif    /* DEBUG_OPCODE */

//
// Error Handling
//

static char *errorMessages[] =
{
  "sanity",
  "incompatible device",
  "can't access waba classes",
  "can't access app classes",
  "can't allocate memory",
  "out of class memory",
  "out of object memory",
  "native stack overflow",
  "native stack underflow",
  "stack overflow",

  "bad class",
  "bad opcode",
  "can't find class",
  "can't find method",
  "can't find field",
  "null object access",
  "null array access",
  "index out of range",
  "divide by zero",
  "bad class cast",
  "class too large"
};

// fatal errors
#define ERR_SanityCheckFailed        1
#define ERR_IncompatibleDevice       2
#define ERR_CantAccessCoreClasses    3
#define ERR_CantAccessAppClasses     4
#define ERR_CantAllocateMemory       5
#define ERR_OutOfClassMem            6
#define ERR_OutOfObjectMem           7
#define ERR_NativeStackOverflow      8
#define ERR_NativeStackUnderflow     9
#define ERR_StackOverflow            10

// program errors
#define ERR_BadClass                 11
#define ERR_BadOpcode                12
#define ERR_CantFindClass            13
#define ERR_CantFindMethod           14
#define ERR_CantFindField            15
#define ERR_NullObjectAccess         16
#define ERR_NullArrayAccess          17
#define ERR_IndexOutOfRange          18
#define ERR_DivideByZero             19
#define ERR_ClassCastException       20
#define ERR_ClassTooLarge            21

// flags for stringToUtf()
#define STU_NULL_TERMINATE 1
#define STU_USE_STATIC     2

//
// types and accessors
//

typedef uint32 WObject;

typedef union
{
  int32 intValue;
  float32 floatValue;
  void *classRef;
  uchar *pc;
  void *refValue;
  WObject obj;

#ifdef WITH_64BITS

  // long/double support : comming from SuperWaba 2.0beta1 guich@200
  // stored in two consecutive Var Lo first, Hi then
  int32 int64ValueHalf;
  int32 float64ValueHalf;

#endif    /* WITH_64BITS */

} Var;

typedef Var (*NativeFunc)(Var stack[]);
typedef void (*ObjDestroyFunc)(WObject obj);

//
// more types and accessors
//

#define WOBJ_class(o) (objectPtr(o))[0].classRef
#define WOBJ_var(o, idx) (objectPtr(o))[idx + 1]

// NOTE: These get various values in objects at defined offsets.
// If the variables in the base classes change, these offsets will
// need to be recomputed. For example, the first (StringCharArray)
// get the character array var offset in a String object.
#define WOBJ_StringCharArrayObj(o) (objectPtr(o))[1].obj
#define WOBJ_StringBufferStrings(o) (objectPtr(o))[1].obj
#define WOBJ_StringBufferCount(o) (objectPtr(o))[2].intValue

#define WOBJ_arrayType(o) (objectPtr(o))[1].intValue
#define WOBJ_arrayLen(o) (objectPtr(o))[2].intValue
#define WOBJ_arrayStart(o) (&(objectPtr(o)[3]))

// for faster access
#define WOBJ_arrayTypeP(objPtr) (objPtr)[1].intValue
#define WOBJ_arrayLenP(objPtr) (objPtr)[2].intValue
#define WOBJ_arrayStartP(objPtr) (&(objPtr[3]))

typedef struct UtfStringStruct
{
  char *str;
  uint32 len;
} UtfString;

typedef union
{
  // FieldVar is either a reference to a static class variable (staticVar)
  // or an offset of a local variable within an object (varOffset)
  Var staticVar;
  uint32 varOffset; // computed var offset in object
} FieldVar;

typedef struct WClassFieldStruct
{
  uchar *header;
  FieldVar var;

#ifdef WITH_64BITS

  FieldVar var2;  // guich@200 - one more int32 to hold 64 bit values

#endif    /* WITH_64BITS */

} WClassField;

#define FIELD_accessFlags(f) getUInt16(f->header)
#define FIELD_nameIndex(f) getUInt16(&f->header[2])
#define FIELD_descIndex(f) getUInt16(&f->header[4])
#define FIELD_isStatic(f) ((FIELD_accessFlags(f) & ACCESS_STATIC) > 0)

typedef union
{
  // Code is either pointer to bytecode or pointer to native function
  // NOTE: If accessFlags(method) & ACCESS_NATIVE then nativeFunc
  // is set, otherwise codeAttr is set. Native methods don't have
  // maxStack, maxLocals so it is OK to merge the codeAttr w/nativeFunc.
  uchar *codeAttr;
  NativeFunc nativeFunc;
} Code;

typedef struct WClassMethodStruct
{
  uchar *header;
  Code code;
  uint16 numParams:14;
  uint16 returnsValue:1;
  uint16 isInit:1;
} WClassMethod;

#define METH_accessFlags(m) getUInt16(m->header)
#define METH_nameIndex(m) getUInt16(&m->header[2])
#define METH_descIndex(m) getUInt16(&m->header[4])
#define METH_maxStack(m) getUInt16(&m->code.codeAttr[6])
#define METH_maxLocals(m) getUInt16(&m->code.codeAttr[8])
#define METH_codeCount(m) getUInt32(&m->code.codeAttr[10])
#define METH_code(m) &m->code.codeAttr[14]

#define CONS_offset(wc, idx) wc->constantOffsets[idx - 1]
#define CONS_ptr(wc, idx) (wc->byteRep + CONS_offset(wc, idx))
#define CONS_tag(wc, idx) CONS_ptr(wc, idx)[0]
#define CONS_utfLen(wc, idx) getUInt16(&CONS_ptr(wc, idx)[1])
#define CONS_utfStr(wc, idx) &CONS_ptr(wc, idx)[3]
#define CONS_integer(wc, idx) getInt32(&CONS_ptr(wc, idx)[1])
#define CONS_float(wc, idx) getFloat32(&CONS_ptr(wc, idx)[1])
#define CONS_stringIndex(wc, idx) getUInt16(&CONS_ptr(wc, idx)[1])
#define CONS_classIndex(wc, idx) getUInt16(&CONS_ptr(wc, idx)[1])
#define CONS_nameAndTypeIndex(wc, idx) getUInt16(&CONS_ptr(wc, idx)[3])
#define CONS_nameIndex(wc, idx) getUInt16(&CONS_ptr(wc, idx)[1])
#define CONS_typeIndex(wc, idx) getUInt16(&CONS_ptr(wc, idx)[3])

#ifdef WITH_64BITS

/* guich@200: added to handle 64bit numbers */

#define CONS_double(wc, idx) getFloat64bits(&CONS_ptr(wc, idx)[1])
#define CONS_long(wc, idx) getInt64bits(&CONS_ptr(wc, idx)[1])

/* end guich@200 modification */

#endif    /* WITH_64BITS */

// The VM keeps an array of constant offsets for each constant in a class
// in the runtime class structure (see WClassStruct). For each constant,
// the offset is an offset from the start of the bytes defining the class.
// Depdending on whether SMALLMEM is defined, the offset is either a 16 or
// 32 bit quantity. So, if SMALLMEM is defined, the maximum offset is 2^16.
// However, we also keep a bit in the constant to determine whether the
// constant is an offset that is "bound" or not. So, the maximum value of
// an offset if SMALLMEM is defined (the small memory model) is 32767.
//
// This means under the small memory model, the biggest class constant
// pool we can have is 32K. Under the large memory model (SMALLMEM not
// defined) the maximum class constant pool size that we could have is
// 2^31 bytes. Using SMALLMEM can save quite a bit of memory since
// constant pools tend to be large.
//
// When a constant offset is "bound", instead of the offset being
// an offset into the constant pool, it is (with the exception of methods)
// a pointer offset from the start of the class heap to the actual data
// the constant refers to.
//
// For example, when a field constant is bound, it contains an offset
// from the start of the class heap to the actual WClassField * structure
// for the field. For class offsets, it is an offset to the WClass *
// structure. For method offsets, the offset is a virtual method number
// and class index. Only class, field and methods can be bound.
//
// A bound offset will only be bound if the offset of the actual structure
// in the class heap is within the range that can fit in the offset. For
// example, in a small memory model, if a WClassField * structure exists
// beyond 32K from the start of the class heap, its offset can't be bound.
// If that happens, the offset simply won't be bound and will retain
// an offset into the constant pool (known now as an "adaptive bind").
//
// Binding of constants (adaptive quickbind) will only be performed if
// QUICKBIND is defined. When an offset is bound, it's CONS_boundBit
// will be set to 1.

#ifdef SMALLMEM

typedef uint16 ConsOffset;
#define MAX_consOffset 0x7FFF
#define CONS_boundBit 0x8000
#define CONS_boundOffsetMask 0x7FFF

// 1 bit for bound bit, 7 bits for method, 8 bits for class index
#define MAX_boundMethodNum 127
#define MAX_boundClassIndex 255
#define CONS_boundMethodShift 8
#define CONS_boundClassMask 0xFF;

#else

typedef uint32 ConsOffset;
#define MAX_consOffset 0x7FFFFFFF
#define CONS_boundBit 0x80000000
#define CONS_boundOffsetMask 0x7FFFFFFF

// 1 bit for bound bit, 15 bits for method, 16 bits for class index
#define MAX_boundMethodNum 32767
#define MAX_boundClassIndex 65535
#define CONS_boundMethodShift 16
#define CONS_boundClassMask 0xFFFF;

#endif

#ifdef QUICKBIND

typedef struct
{
  uint16 classNum:6;
  uint16 methodNum:10;
} VMapValue;

#define MAX_superClassNum 63
#define MAX_methodNum 1023

typedef struct
{
  VMapValue *mapValues; // maps virtual method number to class, virtual method index
  uint16 mapSize; // size of map = number of inherited methods
  uint16 numVirtualMethods; // number of new virtual methods in the class
  uint16 numOverriddenMethods; // number of overridden methods in the class
} VirtualMethodMap;

// search types for getMethodMapNum()
#define SEARCH_ALL 0
#define SEARCH_INHERITED 1
#define SEARCH_THISCLASS 2

// keep this a prime number for best distribution
#define OVERRIDE_HASH_SIZE 127

#endif    /* QUICKBIND */

// NOTE: In the following structure, a constant offset can either be
// bound (by having boundBit set) in which case it is an offset into
// the classHeap directly or unbound in which case it is an offset into
// the byteRep of the class
typedef struct WClassStruct
{
  struct WClassStruct **superClasses; // array of this classes superclasses
  uint16 numSuperClasses;
  uint16 classNameIndex;
  uchar *byteRep; // pointer to class representation in memory (bytes)
  uchar *attrib2; // pointer to area after constant pool (accessFlags)
  uint16 numConstants;
  ConsOffset *constantOffsets;
  uint16 numFields;
  WClassField *fields;
  uint16 numMethods;
  WClassMethod *methods;
#ifdef QUICKBIND
  VirtualMethodMap vMethodMap;
#endif
  uint16 numVars; // computed number of object variables
  ObjDestroyFunc objDestroyFunc;
  struct WClassStruct *nextClass; // next class in hash table linked list
} WClass;

#define WCLASS_accessFlags(wc) getUInt16(wc->attrib2)
#define WCLASS_thisClass(wc) getUInt16(&wc->attrib2[2])
#define WCLASS_superClass(wc) getUInt16(&wc->attrib2[4])
#define WCLASS_numInterfaces(wc) getUInt16(&wc->attrib2[6])
#define WCLASS_interfaceIndex(wc, idx) getUInt16(&wc->attrib2[8 + (idx * 2)])
#define WCLASS_objectSize(wc) ((wc->numVars + 1) * sizeof(Var))
#define WCLASS_isInterface(wc) ((WCLASS_accessFlags(wc) & ACCESS_INTERFACE) > 0)

typedef struct
{
  uint16 errNum;
  char className[40];
  char methodName[40];
  char arg1[40];
  char arg2[40];
} ErrorStatus;

//
// private function prototypes
//

#ifdef WITH_THREAD_NATIVE
CRITICAL_SECTION vmThreadLock;
CRITICAL_SECTION nmThreadLock;

Var*        getVmStack(void);
WObject*    getNmStack(void);
uint32      getVmStackSizeInBytes(void);
uint32      getNmStackSizeInBytes(void);
#endif

void VmInit(uint32 vmStackSizeInBytes, uint32 nmStackSizeInBytes,
		   uint32 classHeapSize, uint32 objectHeapSize);
void VmError(uint16 errNum, WClass *iclass, UtfString *desc1, UtfString *desc2);
void VmQuickError(uint16 errNum);
WObject VmStartApp(char *className);
int VmStartApplication(char *className, int argc, char** argv); //SD
void VmStopApp(WObject mainWinObj);
void VmFree();
WClass *findLoadedClass(UtfString className);
WClass *getClass(UtfString className);
uchar *nativeLoadClass(UtfString className, uint32 *size);
void freeClass(WClass *wclass);
uchar *loadClassConstant(WClass *wclass, uint16 idx, uchar *p);
uchar *loadClassField(WClass *wclass, WClassField *field, uchar *p);
Var constantToVar(WClass *wclass, uint16 idx);
uchar *loadClassMethod(WClass *wclass, WClassMethod *method, uchar *p);
#ifdef QUICKBIND
int createVirtualMethodMap(WClass *wclass);
#endif
UtfString createUtfString(char *buf);
UtfString getUtfString(WClass *wclass, uint16 idx);
WObject createObject(WClass *wclass);
int32 arrayTypeSize(int32 type);
int32 arraySize(int32 type, int32 len);
WObject createArrayObject(int32 type, int32 len);
uint16 arrayType(char c);
WObject createMultiArray(int32 ndim, char *desc, Var *sizes);
WObject createStringFromUtf(UtfString s);
WObject createString(char *buf);
UtfString stringToUtf(WObject str, int flags);
int arrayRangeCheck(WObject array, int32 start, int32 count);
Var copyArray(Var stack[]);
WClassField *getField(WClass *wclass, UtfString name, UtfString desc);
WClass *getClassByIndex(WClass *wclass, uint16 classIndex);
#ifdef QUICKBIND
int compareMethodNameDesc(WClass *wclass, uint16 mapNum, UtfString name, UtfString desc);
int32 getMethodMapNum(WClass *wclass, UtfString name, UtfString desc, int searchType);
WClassMethod *getMethodByMapNum(WClass *wclass, WClass **vclass, uint16 mapNum);
#endif
WClassMethod *getMethod(WClass *wclass, UtfString name, UtfString desc, WClass **vclass);
int32 countMethodParams(UtfString desc);
int compatible(WClass *wclass, WClass *target);
int compatibleArray(WObject obj, UtfString arrayName);
uint32 getUnusedMem();
int initObjectHeap(uint32 heapSize);
void freeObjectHeap();
void markObject(WObject obj);
void sweep();
void gc();
WObject allocObject(int32 size);
Var *objectPtr(WObject obj);
int pushObject(WObject obj);
WObject popObject();
NativeFunc getNativeMethod(WClass *wclass, UtfString methodName, UtfString methodDesc);
void setClassHooks(WClass *wclass);
#ifdef WITH_THREAD_NATIVE
DWORD WINAPI executeMethod(void* passedWParams);
#else
void executeMethod( WClass *wclass,
                    WClassMethod *method,
                    Var params[],
                    uint32 numParams);
#endif
unsigned char * loadClassFrmJar(char * jar, char * classname, int * size);
uint32 ucsToUc(uint16* ucsStr, uint16* ucStr, uint32 ucsStrSize);

#ifdef WITH_64BITS

// guich@200 - transforms 2 consecutive vars in one long
//             the first var stores the lower 32 bits and
//             the 2nd var stores the upper 32 bits of the double
int64 vars2int64(Var *v);

// guich@200 - transforms 2 consecutive vars in one double;
//             the first var stores the lower 32 bits and
//             the 2nd var stores the upper 32 bits of the double
float64 vars2double(Var *v);

#endif    /* WITH_64BITS */

//
// global vars
//

/* Main qualified class name */
char * mainClassName;

#ifdef USE_VIRTUAL_KBD

/* Flag indicating whether to open the virtual keyboard upon startup */
extern int withVirtualKeyboard;

#endif /* USE_VIRTUAL_KBD */

extern int vmInitialized;
extern int isApplication;

// virtual machine stack
extern Var *vmStack;
extern uint32 vmStackSize; // in Var units
extern uint32 vmStackPtr;

// native method stack
extern WObject *nmStack;
extern uint32 nmStackSize; // in WObject units
extern uint32 nmStackPtr;

// keep these prime numbers for best distribution
#ifdef SMALLMEM
#define CLASS_HASH_SIZE 63
#else
#define CLASS_HASH_SIZE 255
#endif

// class heap
extern uchar *classHeap;
extern uint32 classHeapSize;
extern uint32 classHeapUsed;
extern WClass *classHashList[CLASS_HASH_SIZE];

// error status
extern ErrorStatus vmStatus;

// pointer to String class (for performance)
extern WClass *stringClass;

//
// Native Methods and Hooks
//

typedef struct
{
  char *className;
  ObjDestroyFunc destroyFunc;
  uint16 varsNeeded;
} ClassHook;

typedef struct
{
  uint32 hash;
  NativeFunc func;
} NativeMethod;


#if defined(JAYSOS)
#include "jaysos/nmjaysos_b.h"
#include "jaysos/nmjaysos_c.h"
#elif defined(PALMOS)
#include "palm/nmpalm_b.h"
#include "palm/nmpalm_c.h"
#elif defined(WIN32)
#include "win32/nmwin32_b.h"
#include "win32/nmwin32_c.h"
#elif defined(WINCE)
#include "win32/nmwin32_b.h"
#include "win32/nmwin32_c.h"
#elif defined(LINUX)
#include "linux/nm_linux_b.h"
#include "linux/nm_linux_c.h"
#include "linux/debug.h"
#elif defined(TOPPERS)
#include "toppers/nmtoppers_b.h"
#include "toppers/nmtoppers_c.h"
#endif

#endif       /* __WABA_H__ */

/*
   Local Variables:
   c-file-style: "smartdata"
   End:
*/
