//
//  Node.cpp
//  jswf
//
//  Created by Alexander Rath on 23.01.15.
//  Copyright (c) 2015 Alexander Rath. All rights reserved.
//

#include "Node.h"

using namespace jswf::avm2::ast;

std::shared_ptr<Node> Node::getProperty(Multiname &mn) {
  return std::make_shared<AttrNode>(shared_from_this(), mn);
}

std::shared_ptr<Node> Node::setProperty(Multiname &mn, std::shared_ptr<Node> &value) {
  return std::make_shared<DiadicNode>(value, getProperty(mn), "=", 0);
}