//
//  Opcode.cpp
//  jswf
//
//  Created by Alexander Rath on 20.01.15.
//  Copyright (c) 2015 Alexander Rath. All rights reserved.
//

#include "Opcode.h"
#include "ops-macro.h"
#include "GenericReader.h"
#include "ABCFile.h"

using namespace jswf;
using namespace jswf::avm2;
Opcode jswf::avm2::opcodes[256];

void populateOpcode(const char *name, Opcode::Code code, int stackPop, int stackPush, int scopeBalance = 0, Opcode::Flags flags = Opcode::NoFlag, ConstantKind::Enum constant = ConstantKind::UndefinedKind) {
  opcodes[code].name = name;
  opcodes[code].code = code;
  opcodes[code].stackPop = -stackPop;
  opcodes[code].stackPush = stackPush;
  opcodes[code].scopeBalance = scopeBalance;
  opcodes[code].flags = flags;
  opcodes[code].constantKind = constant;
}

__attribute__((constructor))
void populateOpcodes() {
#define op(name, code, ...) populateOpcode(# name, (Opcode::Code)code, __VA_ARGS__);
  ops
#undef op
}