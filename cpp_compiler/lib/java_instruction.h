// https://en.wikipedia.org/wiki/List_of_Java_bytecode_instructions

//                              Opcode // Stack [before]→[after]         ## Description
#define INS_aaload              0x32   // arrayref, index → value        ## load onto the stack a reference from an array
#define INS_aastore             0x53   // arrayref, index, value →       ## store a reference in an array
#define INS_aconst_null         0x01   // → null                         ## push a null reference onto the stack
//1: index
#define INS_aload               0x19   // → objectref                    ## load a reference onto the stack from a local variable #index
#define INS_aload_0             0x2a   // → objectref                    ## load a reference onto the stack from local variable 0
#define INS_aload_1             0x2b   // → objectref                    ## load a reference onto the stack from local variable 1
#define INS_aload_2             0x2c   // → objectref                    ## load a reference onto the stack from local variable 2
#define INS_aload_3             0x2d   // → objectref                    ## load a reference onto the stack from local variable 3
//2: indexbyte1, indexbyte2
#define INS_anewarray           0xbd   // count → arrayref               ## create a new array of references of length count and component type identified by the class reference index (indexbyte1 << 8 | indexbyte2) in the constant pool
#define INS_areturn             0xb0   // objectref → [empty]            ## return a reference from a method
#define INS_arraylength         0xbe   // arrayref → length              ## get the length of an array
//1: index
#define INS_astore              0x3a   // objectref →                    ## store a reference into a local variable #index
#define INS_astore_0            0x4b   // objectref →                    ## store a reference into local variable 0
#define INS_astore_1            0x4c   // objectref →                    ## store a reference into local variable 1
#define INS_astore_2            0x4d   // objectref →                    ## store a reference into local variable 2
#define INS_astore_3            0x4e   // objectref →                    ## store a reference into local variable 3
#define INS_athrow              0xbf   // objectref → [empty], objectref ## throws an error or exception (notice that the rest of the stack is cleared, leaving only a reference to the Throwable)
#define INS_baload              0x33   // arrayref, index → value        ## load a byte or Boolean value from an array
#define INS_bastore             0x54   // arrayref, index, value →       ## store a byte or Boolean value into an array
//1: byte
#define INS_bipush              0x10   // → value                        ## push a byte onto the stack as an integer value
#define INS_breakpoint          0xca   //                                ## reserved for breakpoints in Java debuggers; should not appear in any class file
#define INS_caload              0x34   // arrayref, index → value        ## load a char from an array
#define INS_castore             0x55   // arrayref, index, value →       ## store a char into an array
//2: indexbyte1, indexbyte2
#define INS_checkcast           0xc0   // objectref → objectref          ## checks whether an objectref is of a certain type, the class reference of which is in the constant pool at index (indexbyte1 << 8 | indexbyte2)
#define INS_d2f                 0x90   // value → result                 ## convert a double to a float
#define INS_d2i                 0x8e   // value → result                 ## convert a double to an int
#define INS_d2l                 0x8f   // value → result                 ## convert a double to a long
#define INS_dadd                0x63   // value1, value2 → result        ## add two doubles
#define INS_daload              0x31   // arrayref, index → value        ## load a double from an array
#define INS_dastore             0x52   // arrayref, index, value →       ## store a double into an array
#define INS_dcmpg               0x98   // value1, value2 → result        ## compare two doubles, 1 on NaN
#define INS_dcmpl               0x97   // value1, value2 → result        ## compare two doubles, -1 on NaN
#define INS_dconst_0            0x0e   // → 0.0                          ## push the constant 0.0 (a double) onto the stack
#define INS_dconst_1            0x0f   // → 1.0                          ## push the constant 1.0 (a double) onto the stack
#define INS_ddiv                0x6f   // value1, value2 → result        ## divide two doubles
//1: index
#define INS_dload               0x18   // → value                        ## load a double value from a local variable #index
#define INS_dload_0             0x26   // → value                        ## load a double from local variable 0
#define INS_dload_1             0x27   // → value                        ## load a double from local variable 1
#define INS_dload_2             0x28   // → value                        ## load a double from local variable 2
#define INS_dload_3             0x29   // → value                        ## load a double from local variable 3
#define INS_dmul                0x6b   // value1, value2 → result        ## multiply two doubles
#define INS_dneg                0x77   // value → result                 ## negate a double
#define INS_drem                0x73   // value1, value2 → result        ## get the remainder from a division between two doubles
#define INS_dreturn             0xaf   // value → [empty]                ## return a double from a method
//1: index
#define INS_dstore              0x39   // value →                        ## store a double value into a local variable #index
#define INS_dstore_0            0x47   // value →                        ## store a double into local variable 0
#define INS_dstore_1            0x48   // value →                        ## store a double into local variable 1
#define INS_dstore_2            0x49   // value →                        ## store a double into local variable 2
#define INS_dstore_3            0x4a   // value →                        ## store a double into local variable 3
#define INS_dsub                0x67   // value1, value2 → result        ## subtract a double from another
#define INS_dup                 0x59   // value → value, value           ## duplicate the value on top of the stack
#define INS_dup_x1              0x5a   // value2, value1 → value1, value2, value1 ## insert a copy of the top value into the stack two values from the top. value1 and value2 must not be of the type double or long.
#define INS_dup_x2              0x5b   // value3, value2, value1 → value1, value3, value2, value1 ## insert a copy of the top value into the stack two (if value2 is double or long it takes up the entry of value3, too) or three values (if value2 is neither double nor long) from the top
#define INS_dup2                0x5c   // {value2, value1} → {value2, value1}, {value2, value1} ## duplicate top two stack words (two values, if value1 is not double nor long; a single value, if value1 is double or long)
#define INS_dup2_x1             0x5d   // value3, {value2, value1} → {value2, value1}, value3, {value2, value1} ## duplicate two words and insert beneath third word (see explanation above)
#define INS_dup2_x2             0x5e   // {value4, value3}, {value2, value1} → {value2, value1}, {value4, value3}, {value2, value1} ## duplicate two words and insert beneath fourth word
#define INS_f2d                 0x8d   // value → result                 ## convert a float to a double
#define INS_f2i                 0x8b   // value → result                 ## convert a float to an int
#define INS_f2l                 0x8c   // value → result                 ## convert a float to a long
#define INS_fadd                0x62   // value1, value2 → result        ## add two floats
#define INS_faload              0x30   // arrayref, index → value        ## load a float from an array
#define INS_fastore             0x51   // arrayref, index, value →       ## store a float in an array
#define INS_fcmpg               0x96   // value1, value2 → result        ## compare two floats, 1 on NaN
#define INS_fcmpl               0x95   // value1, value2 → result        ## compare two floats, -1 on NaN
#define INS_fconst_0            0x0b   // → 0.0f                         ## push 0.0f on the stack
#define INS_fconst_1            0x0c   // → 1.0f                         ## push 1.0f on the stack
#define INS_fconst_2            0x0d   // → 2.0f                         ## push 2.0f on the stack
#define INS_fdiv                0x6e   // value1, value2 → result        ## divide two floats
//1: index
#define INS_fload               0x17   // → value                        ## load a float value from a local variable #index
#define INS_fload_0             0x22   // → value                        ## load a float value from local variable 0
#define INS_fload_1             0x23   // → value                        ## load a float value from local variable 1
#define INS_fload_2             0x24   // → value                        ## load a float value from local variable 2
#define INS_fload_3             0x25   // → value                        ## load a float value from local variable 3
#define INS_fmul                0x6a   // value1, value2 → result        ## multiply two floats
#define INS_fneg                0x76   // value → result                 ## negate a float
#define INS_frem                0x72   // value1, value2 → result        ## get the remainder from a division between two floats
#define INS_freturn             0xae   // value → [empty]                ## return a float
//1: index
#define INS_fstore              0x38   // value →                        ## store a float value into a local variable #index
#define INS_fstore_0            0x43   // value →                        ## store a float value into local variable 0
#define INS_fstore_1            0x44   // value →                        ## store a float value into local variable 1
#define INS_fstore_2            0x45   // value →                        ## store a float value into local variable 2
#define INS_fstore_3            0x46   // value →                        ## store a float value into local variable 3
#define INS_fsub                0x66   // value1, value2 → result        ## subtract two floats
//2: indexbyte1, indexbyte2
#define INS_getfield            0xb4   // objectref → value              ## get a field value of an object objectref, where the field is identified by field reference in the constant pool index (indexbyte1 << 8 | indexbyte2)
//2: indexbyte1, indexbyte2
#define INS_getstatic           0xb2   // → value                        ## get a static field value of a class, where the field is identified by field reference in the constant pool index (indexbyte1 << 8 | indexbyte2)
//2: branchbyte1, branchbyte2
#define INS_goto                0xa7   // [no change]                    ## goes to another instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//4: branchbyte1, branchbyte2, branchbyte3, branchbyte4
#define INS_goto_w              0xc8   // [no change]                    ## goes to another instruction at branchoffset (signed int constructed from unsigned bytes branchbyte1 << 24 | branchbyte2 << 16 | branchbyte3 << 8 | branchbyte4)
#define INS_i2b                 0x91   // value → result                 ## convert an int into a byte
#define INS_i2c                 0x92   // value → result                 ## convert an int into a character
#define INS_i2d                 0x87   // value → result                 ## convert an int into a double
#define INS_i2f                 0x86   // value → result                 ## convert an int into a float
#define INS_i2l                 0x85   // value → result                 ## convert an int into a long
#define INS_i2s                 0x93   // value → result                 ## convert an int into a short
#define INS_iadd                0x60   // value1, value2 → result        ## add two ints
#define INS_iaload              0x2e   // arrayref, index → value        ## load an int from an array
#define INS_iand                0x7e   // value1, value2 → result        ## perform a bitwise AND on two integers
#define INS_iastore             0x4f   // arrayref, index, value →       ## store an int into an array
#define INS_iconst_m1           0x02   // → -1                           ## load the int value −1 onto the stack
#define INS_iconst_0            0x03   // → 0                            ## load the int value 0 onto the stack
#define INS_iconst_1            0x04   // → 1                            ## load the int value 1 onto the stack
#define INS_iconst_2            0x05   // → 2                            ## load the int value 2 onto the stack
#define INS_iconst_3            0x06   // → 3                            ## load the int value 3 onto the stack
#define INS_iconst_4            0x07   // → 4                            ## load the int value 4 onto the stack
#define INS_iconst_5            0x08   // → 5                            ## load the int value 5 onto the stack
#define INS_idiv                0x6c   // value1, value2 → result        ## divide two integers
//2: branchbyte1, branchbyte2
#define INS_if_acmpeq           0xa5   // value1, value2 →               ## if references are equal, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_if_acmpne           0xa6   // value1, value2 →               ## if references are not equal, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_if_icmpeq           0x9f   // value1, value2 →               ## if ints are equal, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_if_icmpge           0xa2   // value1, value2 →               ## if value1 is greater than or equal to value2, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_if_icmpgt           0xa3   // value1, value2 →               ## if value1 is greater than value2, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_if_icmple           0xa4   // value1, value2 →               ## if value1 is less than or equal to value2, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_if_icmplt           0xa1   // value1, value2 →               ## if value1 is less than value2, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_if_icmpne           0xa0   // value1, value2 →               ## if ints are not equal, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_ifeq                0x99   // value →                        ## if value is 0, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_ifge                0x9c   // value →                        ## if value is greater than or equal to 0, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_ifgt                0x9d   // value →                        ## if value is greater than 0, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_ifle                0x9e   // value →                        ## if value is less than or equal to 0, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_iflt                0x9b   // value →                        ## if value is less than 0, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_ifne                0x9a   // value →                        ## if value is not 0, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_ifnonnull           0xc7   // value →                        ## if value is not null, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: branchbyte1, branchbyte2
#define INS_ifnull              0xc6   // value →                        ## if value is null, branch to instruction at branchoffset (signed short constructed from unsigned bytes branchbyte1 << 8 | branchbyte2)
//2: index, const
#define INS_iinc                0x84   // [No change]                    ## increment local variable #index by signed byte const
//1: index
#define INS_iload               0x15   // → value                        ## load an int value from a local variable #index
#define INS_iload_0             0x1a   // → value                        ## load an int value from local variable 0
#define INS_iload_1             0x1b   // → value                        ## load an int value from local variable 1
#define INS_iload_2             0x1c   // → value                        ## load an int value from local variable 2
#define INS_iload_3             0x1d   // → value                        ## load an int value from local variable 3
#define INS_impdep1             0xfe   //                                ## reserved for implementation-dependent operations within debuggers; should not appear in any class file
#define INS_impdep2             0xff   //                                ## reserved for implementation-dependent operations within debuggers; should not appear in any class file
#define INS_imul                0x68   // value1, value2 → result        ## multiply two integers
#define INS_ineg                0x74   // value → result                 ## negate int
//2: indexbyte1, indexbyte2
#define INS_instanceof          0xc1   // objectref → result             ## determines if an object objectref is of a given type, identified by class reference index in constant pool (indexbyte1 << 8 | indexbyte2)
//4: indexbyte1, indexbyte2, 0, 0
#define INS_invokedynamic       0xba   // [arg1, arg2, ...] → result     ## invokes a dynamic method and puts the result on the stack (might be void); the method is identified by method reference index in constant pool (indexbyte1 << 8 | indexbyte2)
//4: indexbyte1, indexbyte2, count, 0
#define INS_invokeinterface     0xb9   // objectref, [arg1, arg2, ...] → result ## invokes an interface method on object objectref and puts the result on the stack (might be void); the interface method is identified by method reference index in constant pool (indexbyte1 << 8 | indexbyte2)
//2: indexbyte1, indexbyte2
#define INS_invokespecial       0xb7   // objectref, [arg1, arg2, ...] → result ## invoke instance method on object objectref and puts the result on the stack (might be void); the method is identified by method reference index in constant pool (indexbyte1 << 8 | indexbyte2)
//2: indexbyte1, indexbyte2
#define INS_invokestatic        0xb8   // [arg1, arg2, ...] → result     ## invoke a static method and puts the result on the stack (might be void); the method is identified by method reference index in constant pool (indexbyte1 << 8 | indexbyte2)
//2: indexbyte1, indexbyte2
#define INS_invokevirtual       0xb6   // objectref, [arg1, arg2, ...] → result ## invoke virtual method on object objectref and puts the result on the stack (might be void); the method is identified by method reference index in constant pool (indexbyte1 << 8 | indexbyte2)
#define INS_ior                 0x80   // value1, value2 → result        ## bitwise int OR
#define INS_irem                0x70   // value1, value2 → result        ## logical int remainder
#define INS_ireturn             0xac   // value → [empty]                ## return an integer from a method
#define INS_ishl                0x78   // value1, value2 → result        ## int shift left
#define INS_ishr                0x7a   // value1, value2 → result        ## int arithmetic shift right
//1: index
#define INS_istore              0x36   // value →                        ## store int value into variable #index
#define INS_istore_0            0x3b   // value →                        ## store int value into variable 0
#define INS_istore_1            0x3c   // value →                        ## store int value into variable 1
#define INS_istore_2            0x3d   // value →                        ## store int value into variable 2
#define INS_istore_3            0x3e   // value →                        ## store int value into variable 3
#define INS_isub                0x64   // value1, value2 → result        ## int subtract
#define INS_iushr               0x7c   // value1, value2 → result        ## int logical shift right
#define INS_ixor                0x82   // value1, value2 → result        ## int xor
#define INS_l2d                 0x8a   // value → result                 ## convert a long to a double
#define INS_l2f                 0x89   // value → result                 ## convert a long to a float
#define INS_l2i                 0x88   // value → result                 ## convert a long to a int
#define INS_ladd                0x61   // value1, value2 → result        ## add two longs
#define INS_laload              0x2f   // arrayref, index → value        ## load a long from an array
#define INS_land                0x7f   // value1, value2 → result        ## bitwise AND of two longs
#define INS_lastore             0x50   // arrayref, index, value →       ## store a long to an array
#define INS_lcmp                0x94   // value1, value2 → result        ## push 0 if the two longs are the same, 1 if value1 is greater than value2, -1 otherwise
#define INS_lconst_0            0x09   // → 0L                           ## push 0L (the number zero with type long) onto the stack
#define INS_lconst_1            0x0a   // → 1L                           ## push 1L (the number one with type long) onto the stack
//1: index
#define INS_ldc                 0x12   // → value                        ## push a constant #index from a constant pool (String, int, float, Class, java.lang.invoke.MethodType, java.lang.invoke.MethodHandle, or a dynamically-computed constant) onto the stack
//2: indexbyte1, indexbyte2
#define INS_ldc_w               0x13   // → value                        ## push a constant #index from a constant pool (String, int, float, Class, java.lang.invoke.MethodType, java.lang.invoke.MethodHandle, or a dynamically-computed constant) onto the stack (wide index is constructed as indexbyte1 << 8 | indexbyte2)
//2: indexbyte1, indexbyte2
#define INS_ldc2_w              0x14   // → value                        ## push a constant #index from a constant pool (double, long, or a dynamically-computed constant) onto the stack (wide index is constructed as indexbyte1 << 8 | indexbyte2)
#define INS_ldiv                0x6d   // value1, value2 → result        ## divide two longs
//1: index
#define INS_lload               0x16   // → value                        ## load a long value from a local variable #index
#define INS_lload_0             0x1e   // → value                        ## load a long value from a local variable 0
#define INS_lload_1             0x1f   // → value                        ## load a long value from a local variable 1
#define INS_lload_2             0x20   // → value                        ## load a long value from a local variable 2
#define INS_lload_3             0x21   // → value                        ## load a long value from a local variable 3
#define INS_lmul                0x69   // value1, value2 → result        ## multiply two longs
#define INS_lneg                0x75   // value → result                 ## negate a long
//8+: <0–3 bytes padding>, defaultbyte1, defaultbyte2, defaultbyte3, defaultbyte4, npairs1, npairs2, npairs3, npairs4, match-offset pairs...
#define INS_lookupswitch        0xab   // key →                          ## a target address is looked up from a table using a key and execution continues from the instruction at that address
#define INS_lor                 0x81   // value1, value2 → result        ## bitwise OR of two longs
#define INS_lrem                0x71   // value1, value2 → result        ## remainder of division of two longs
#define INS_lreturn             0xad   // value → [empty]                ## return a long value
#define INS_lshl                0x79   // value1, value2 → result        ## bitwise shift left of a long value1 by int value2 positions
#define INS_lshr                0x7b   // value1, value2 → result        ## bitwise shift right of a long value1 by int value2 positions
//1: index
#define INS_lstore              0x37   // value →                        ## store a long value in a local variable #index
#define INS_lstore_0            0x3f   // value →                        ## store a long value in a local variable 0
#define INS_lstore_1            0x40   // value →                        ## store a long value in a local variable 1
#define INS_lstore_2            0x41   // value →                        ## store a long value in a local variable 2
#define INS_lstore_3            0x42   // value →                        ## store a long value in a local variable 3
#define INS_lsub                0x65   // value1, value2 → result        ## subtract two longs
#define INS_lushr               0x7d   // value1, value2 → result        ## bitwise shift right of a long value1 by int value2 positions, unsigned
#define INS_lxor                0x83   // value1, value2 → result        ## bitwise XOR of two longs
#define INS_monitorenter        0xc2   // objectref →                    ## enter monitor for object ("grab the lock" – start of synchronized() section)
#define INS_monitorexit         0xc3   // objectref →                    ## exit monitor for object ("release the lock" – end of synchronized() section)
//3: indexbyte1, indexbyte2, dimensions
#define INS_multianewarray      0xc5   // count1, [count2,...] → arrayref ## create a new array of dimensions dimensions of type identified by class reference in constant pool index (indexbyte1 << 8 | indexbyte2); the sizes of each dimension is identified by count1, [count2, etc.]
//2: indexbyte1, indexbyte2
#define INS_new                 0xbb   // → objectref                    ## create new object of type identified by class reference in constant pool index (indexbyte1 << 8 | indexbyte2)
//1: atype
#define INS_newarray            0xbc   // count → arrayref               ## create new array with count elements of primitive type identified by atype
#define INS_nop                 0x00   // [No change]                    ## perform no operation
#define INS_pop                 0x57   // value →                        ## discard the top value on the stack
#define INS_pop2                0x58   // {value2, value1} →             ## discard the top two values on the stack (or one value, if it is a double or long)
//2: indexbyte1, indexbyte2
#define INS_putfield            0xb5   // objectref, value →             ## set field to value in an object objectref, where the field is identified by a field reference index in constant pool (indexbyte1 << 8 | indexbyte2)
//2: indexbyte1, indexbyte2
#define INS_putstatic           0xb3   // value →                        ## set static field to value in a class, where the field is identified by a field reference index in constant pool (indexbyte1 << 8 | indexbyte2)
#define INS_return              0xb1   // → [empty]                      ## return void from method
#define INS_saload              0x35   // arrayref, index → value        ## load short from array
#define INS_sastore             0x56   // arrayref, index, value →       ## store short to array
//2: byte1, byte2
#define INS_sipush              0x11   // → value                        ## push a short onto the stack as an integer value
#define INS_swap                0x5f   // value2, value1 → value1, value2 ## swaps two top words on the stack (note that value1 and value2 must not be double or long)
//16+: [0–3 bytes padding], defaultbyte1, defaultbyte2, defaultbyte3, defaultbyte4, lowbyte1, lowbyte2, lowbyte3, lowbyte4, highbyte1, highbyte2, highbyte3, highbyte4, jump offsets...
#define INS_tableswitch         0xaa   // index →                        ## continue execution from an address in the table at offset index
//3/5: opcode, indexbyte1, indexbyte2 or iinc, indexbyte1, indexbyte2, countbyte1, countbyte2
#define INS_wide                0xc4   // [same as for corresponding instructions] ## execute opcode, where opcode is either iload, fload, aload, lload, dload, istore, fstore, astore, lstore, dstore, or ret, but assume the index is 16 bit; or execute iinc, where the index is 16 bits and the constant to increment by is a signed 16 bit short
