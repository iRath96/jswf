//
//  ABCFile.cpp
//  jswf
//
//  Created by Alexander Rath on 27.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "ABCFile.h"
#include "StringReader.h"

#include <unordered_map>

using namespace jswf;
using namespace jswf::avm2;

#include "Node.h"

using namespace jswf::avm2::ast;

void ABCFile::decompile(MethodInfo &info, std::string name) {
  CompoundNode compound(0);
  decompile(info, name, &compound);
  printf("%s", compound.toString().c_str());
}

void ABCFile::decompile(MethodInfo &info, std::string name, CompoundNode *compound) {
  CompoundNode *currentCompound = new FunctionNode(&info, name);
  compound->body.push_back(NodePtr(currentCompound));
}