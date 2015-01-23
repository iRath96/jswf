//
//  ops-macro.h
//  jswf
//
//  Created by Alexander Rath on 20.01.15.
//  Copyright (c) 2015 Alexander Rath. All rights reserved.
//

/**
 * @file
 * Defines the <tt>ops</tt>-macro that lists information for all instructions.
 * Expects an <tt>op</tt>-macro to be defined that takes the arguments:
 * @todo Better description of the op macro. Describe the macro itself.
 * <tt>op(name, uint8_t code, int stackPop, int stackPush, int scopeBalance = 0, Opcode::Flags flags = 0, ConstantKind::Enum = 0)</tt>
 */

#define ops \
op(nop          , 0x02, -0, +0) \
op(throw        , 0x03, -1, +0) \
op(getsuper     , 0x04, -1, +1,  0, Opcode::HasMultinameFlag) \
op(setsuper     , 0x05, -2, +0,  0, Opcode::HasMultinameFlag) \
op(dxns         , 0x06, -0, +0,  0, Opcode::HasPoolConstantFlag , ConstantKind::UTF8Kind) \
op(dxnslate     , 0x07, -1, +0) \
op(kill         , 0x08, -0, +0,  0, Opcode::HasRegisterFlag) \
op(label        , 0x09, -0, +0) \
op(ifnlt        , 0x0c, -2, +0,  0, Opcode::IsBranchFlag) \
op(ifnle        , 0x0d, -2, +0,  0, Opcode::IsBranchFlag) \
op(ifngt        , 0x0e, -2, +0,  0, Opcode::IsBranchFlag) \
op(ifnge        , 0x0f, -2, +0,  0, Opcode::IsBranchFlag) \
op(jump         , 0x10, -0, +0,  0, Opcode::IsBranchFlag) \
op(iftrue       , 0x11, -1, +0,  0, Opcode::IsBranchFlag) \
op(iffalse      , 0x12, -1, +0,  0, Opcode::IsBranchFlag) \
op(ifeq         , 0x13, -2, +0,  0, Opcode::IsBranchFlag) \
op(ifne         , 0x14, -2, +0,  0, Opcode::IsBranchFlag) \
op(iflt         , 0x15, -2, +0,  0, Opcode::IsBranchFlag) \
op(ifle         , 0x16, -2, +0,  0, Opcode::IsBranchFlag) \
op(ifgt         , 0x17, -2, +0,  0, Opcode::IsBranchFlag) \
op(ifge         , 0x18, -2, +0,  0, Opcode::IsBranchFlag) \
op(ifstricteq   , 0x19, -2, +0,  0, Opcode::IsBranchFlag) \
op(ifstrictne   , 0x1a, -2, +0,  0, Opcode::IsBranchFlag) \
op(lookupswitch , 0x1b, -2, +0,  0, Opcode::IsCustomLengthFlag) \
op(pushwith     , 0x1c, -1, +0, +1) \
op(popscope     , 0x1d, -0, +0, -1) \
op(nextname     , 0x1e, -2, +1) \
op(hasnext      , 0x1f, -2, +1) \
op(pushnull     , 0x20, -0, +1,  0, Opcode::HasConstantFlag, ConstantKind::NullKind) \
op(pushundefined, 0x21, -0, +1,  0, Opcode::HasConstantFlag, ConstantKind::UndefinedKind) \
op(nextvalue    , 0x23, -2, +1) \
op(pushbyte     , 0x24, -0, +1,  0, Opcode::HasImmediateValueFlag, ConstantKind::ByteKind) \
op(pushshort    , 0x25, -0, +1,  0, Opcode::HasImmediateValueFlag, ConstantKind::ShortKind) \
op(pushtrue     , 0x26, -0, +1,  0, Opcode::HasConstantFlag, ConstantKind::TrueKind) \
op(pushfalse    , 0x27, -0, +1,  0, Opcode::HasConstantFlag, ConstantKind::FalseKind) \
op(pushnan      , 0x28, -0, +1,  0, Opcode::HasConstantFlag, ConstantKind::NANKind) \
op(pop          , 0x29, -1, +0) \
op(dup          , 0x2a, -1, +2) \
op(swap         , 0x2b, -2, +2) \
op(pushstring   , 0x2c, -0, +1,  0, Opcode::HasPoolConstantFlag , ConstantKind::UTF8Kind) \
op(pushint      , 0x2d, -0, +1,  0, Opcode::HasPoolConstantFlag , ConstantKind::IntKind) \
op(pushuint     , 0x2e, -0, +1,  0, Opcode::HasPoolConstantFlag , ConstantKind::UIntKind) \
op(pushdouble   , 0x2f, -0, +1,  0, Opcode::HasPoolConstantFlag , ConstantKind::DoubleKind) \
op(pushscope    , 0x30, -1, +0, +1) \
op(pushnamespace, 0x31, -0, +1,  0, Opcode::HasPoolConstantFlag , ConstantKind::NormalNamespaceKind) \
op(hasnext2     , 0x32, -0, +1,  0, Opcode::IsCustomLengthFlag) \
op(newfunction  , 0x40, -0, +1,  0, Opcode::IsCustomLengthFlag) \
op(call         , 0x41, -2, +1,  0, Opcode::HasArgumentCountFlag) \
op(construct    , 0x42, -1, +1,  0, Opcode::HasArgumentCountFlag) \
op(callmethod   , 0x43, -1, +1,  0, Opcode::HasArgumentCountFlag) \
op(callstatic   , 0x44, -1, +1,  0, Opcode::HasArgumentCountFlag) \
op(callsuper    , 0x45, -1, +1,  0, Opcode::HasArgumentCountFlag | Opcode::HasMultinameFlag) \
op(callproperty , 0x46, -1, +1,  0, Opcode::HasArgumentCountFlag | Opcode::HasMultinameFlag) \
op(returnvoid   , 0x47, -0, +0) \
op(returnvalue  , 0x48, -1, +0) \
op(constructsuper,0x49, -1, +0,  0, Opcode::HasArgumentCountFlag) \
op(constructprop, 0x4a, -1, +1,  0, Opcode::HasArgumentCountFlag | Opcode::HasMultinameFlag) \
op(callproplex  , 0x4c, -1, +1,  0, Opcode::HasArgumentCountFlag | Opcode::HasMultinameFlag) \
op(callsupervoid, 0x4e, -1, +0,  0, Opcode::HasArgumentCountFlag | Opcode::HasMultinameFlag) \
op(callpropvoid , 0x4f, -1, +0,  0, Opcode::HasArgumentCountFlag | Opcode::HasMultinameFlag) \
op(newobject    , 0x55, -0, +1,  0, Opcode::IsCustomStackFlag) \
op(newarray     , 0x56, -0, +1,  0, Opcode::HasArgumentCountFlag) \
op(newactivation, 0x57, -0, +1) \
op(newclass     , 0x58, -0, +1,  0, Opcode::IsCustomLengthFlag) \
op(getdescendants,0x59, -1, +1,  0, Opcode::HasMultinameFlag) \
op(newcatch     , 0x5a, -0, +1,  0, Opcode::IsCustomLengthFlag) \
op(findpropstrict,0x5d, -0, +1,  0, Opcode::HasMultinameFlag) \
op(findproperty , 0x5e, -0, +1,  0, Opcode::HasMultinameFlag) \
op(getlex       , 0x60, -0, +1,  0, Opcode::HasQNameFlag) \
op(setproperty  , 0x61, -2, +0,  0, Opcode::HasMultinameFlag) \
op(getlocal     , 0x62, -0, +1,  0, Opcode::HasRegisterFlag) \
op(setlocal     , 0x63, -1, +0,  0, Opcode::HasRegisterFlag) \
op(getglobalscope,0x64, -0, +1) \
op(getscopeobject,0x65, -0, +1,  0, Opcode::IsCustomLengthFlag) \
op(getproperty  , 0x66, -1, +1,  0, Opcode::HasMultinameFlag) \
op(initproperty , 0x68, -2, +0,  0, Opcode::HasMultinameFlag) \
op(deleteproperty,0x6a, -1, +1,  0, Opcode::HasMultinameFlag) \
op(getslot      , 0x6c, -1, +1) \
op(setslot      , 0x6d, -2, +0) \
op(getglobalslot, 0x6e, -0, +1) \
op(setglobalslot, 0x6f, -1, +0) \
op(convert_s    , 0x70, -1, +1) \
op(esc_xelem    , 0x71, -1, +1) \
op(esc_xattr    , 0x72, -1, +1) \
op(convert_i    , 0x73, -1, +1) \
op(convert_u    , 0x74, -1, +1) \
op(convert_d    , 0x75, -1, +1) \
op(convert_b    , 0x76, -1, +1) \
op(convert_o    , 0x77, -1, +1) \
op(checkfilter  , 0x78, -1, +1) \
op(coerce       , 0x80, -1, +1,  0, Opcode::HasQNameFlag) \
op(coerce_a     , 0x82, -1, +1) \
op(coerce_s     , 0x85, -1, +1) \
op(astype       , 0x86, -1, +1,  0, Opcode::HasQNameFlag) \
op(astypelate   , 0x87, -2, +1) \
op(negate       , 0x90, -1, +1) \
op(increment    , 0x91, -1, +1) \
op(inclocal     , 0x92, -0, +0,  0, Opcode::HasRegisterFlag) \
op(decrement    , 0x93, -1, +1) \
op(declocal     , 0x94, -0, +0,  0, Opcode::HasRegisterFlag) \
op(typeof       , 0x95, -1, +1) \
op(not          , 0x96, -1, +1) \
op(bitnot       , 0x97, -1, +1) \
op(add          , 0xa0, -2, +1) \
op(subtract     , 0xa1, -2, +1) \
op(multiply     , 0xa2, -2, +1) \
op(divide       , 0xa3, -2, +1) \
op(modulo       , 0xa4, -2, +1) \
op(lshift       , 0xa5, -2, +1) \
op(rshift       , 0xa6, -2, +1) \
op(urshift      , 0xa7, -2, +1) \
op(bitand       , 0xa8, -2, +1) \
op(bitor        , 0xa9, -2, +1) \
op(bitxor       , 0xaa, -2, +1) \
op(equals       , 0xab, -2, +1) \
op(strictequals , 0xac, -2, +1) \
op(lessthan     , 0xad, -2, +1) \
op(lessequals   , 0xae, -2, +1) \
op(greaterthan  , 0xaf, -2, +1) \
op(greaterequals, 0xb0, -2, +1) \
op(instanceof   , 0xb1, -2, +1) \
op(istype       , 0xb2, -1, +1,  0, Opcode::HasQNameFlag) \
op(istypelate   , 0xb3, -2, +1) \
op(in           , 0xb4, -2, +1) \
op(increment_i  , 0xc0, -1, +1) \
op(decrement_i  , 0xc1, -1, +1) \
op(inclocal_i   , 0xc2, -0, +0,  0, Opcode::HasRegisterFlag) \
op(declocal_i   , 0xc3, -0, +0,  0, Opcode::HasRegisterFlag) \
op(negate_i     , 0xc4, -1, +1) \
op(add_i        , 0xc5, -2, +1) \
op(subtract_i   , 0xc6, -2, +1) \
op(multiply_i   , 0xc7, -2, +1) \
op(getlocal_0   , 0xd0, -0, +1) \
op(getlocal_1   , 0xd1, -0, +1) \
op(getlocal_2   , 0xd2, -0, +1) \
op(getlocal_3   , 0xd3, -0, +1) \
op(setlocal_0   , 0xd4, -1, +0) \
op(setlocal_1   , 0xd5, -1, +0) \
op(setlocal_2   , 0xd6, -1, +0) \
op(setlocal_3   , 0xd7, -1, +0) \
op(debug        , 0xef, -0, +0) \
op(debugline    , 0xf0, -0, +0) \
op(debugfile    , 0xf1, -0, +0) \
