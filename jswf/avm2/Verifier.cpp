//
//  Verifier.cpp
//  jswf
//
//  Created by Alexander Rath on 20.01.15.
//  Copyright (c) 2015 Alexander Rath. All rights reserved.
//

#include "Verifier.h"
#include "ABCFile.h"

using namespace jswf::avm2;

void Verifier::analyzeBlock(Block &block) {
  printf("Analyzing from %ld for L%ld\n", block.start, block.id);
  
  bcReader.seek_abs(block.start);
  OpcodeData<void> op;
  
  while(!bcReader.eof()) {
    if(bcMap[bcReader.pos] != NULL && bcMap[bcReader.pos] != &block) { // this instruction has already been analyzed
      // we need to split the block here
      
      if(bcMap[bcReader.pos]->start == bcReader.pos) { // Another block begins right here
        printf("  collides with the beginning of L%ld at %ld\n", bcMap[bcReader.pos]->id, bcReader.pos);
        
        block.successors.push_back(bcMap[bcReader.pos]);
        block.end = bcReader.pos;
        
        printf("  ends at %ld with front-collision\n", block.end);
        
        return;
      }
      
      Block *previousBlock = bcMap[bcReader.pos];
      printf("  collides with L%ld at %ld\n", previousBlock->id, bcReader.pos);
      printf("  new Block for collision %ld\n", bcReader.pos);
      
      Block *tail = new Block(blocks.size(), bcReader.pos);
      tail->analyzed = previousBlock->analyzed;
      
      blocks.push_back(tail);
      
      tail->end = previousBlock->end;
      tail->successors = previousBlock->successors;
      tail->predecessors.push_back(&block);
      tail->predecessors.push_back(previousBlock);
      
      for(size_t i = tail->start, j = tail->end; i < j; ++i) // update bcMap
        if(bcMap[i] == previousBlock) bcMap[i] = tail;
      
      previousBlock->end = bcReader.pos;
      previousBlock->successors.clear();
      previousBlock->successors.push_back(tail);
      
      block.successors.push_back(tail);
      block.end = bcReader.pos;
      
      printf("  ends at %ld with collision\n", block.end);
      
      return;
    } else
      bcMap[bcReader.pos] = &block;
    
    op.readStatic(&bcReader);
    
    if(op.opcode.flags & Opcode::IsBranchFlag) {
      // This is a branch. Our analysis of this block ends here.
      
      Block *b0 = bcMap[bcReader.pos], *b1 = bcMap[bcReader.pos+op.branchOffset];
      
      block.end = bcReader.pos;
      
      if(b0 == NULL) {
        printf("  new Block for following %ld\n", bcReader.pos);
        blocks.push_back(bcMap[bcReader.pos] = b0 = new Block(blocks.size(), bcReader.pos));
      }
      
      if(b1 == NULL) {
        printf("  new Block for targetted %ld\n", bcReader.pos+op.branchOffset);
        blocks.push_back(bcMap[bcReader.pos+op.branchOffset] = b1 = new Block(blocks.size(), bcReader.pos+op.branchOffset));
      }
      
      if(op.code != Opcode::op_jump) block.successors.push_back(b0);
      block.successors.push_back(b1);
      
      printf("  ends at %ld with branch, can go to %ld\n", block.end, bcReader.pos+op.branchOffset);
      
      return;
    }
  }
  
  block.end = bcReader.pos;
  printf("  ends at %ld\n", block.end);
  
  // The block ends with the end of the code.
  return;
}

void Verifier::analyzeStackUsage(Block &block) {
  bcReader.seek_abs(block.start);
  OpcodeData<void> op;
  
  while(bcReader.pos < block.end) {
    op.readStatic(&bcReader);
    
    block.stackBalance -= op.totalStackPop(file);
    if(block.stackBalance < block.stackMin) block.stackMin = block.stackBalance;
    
    block.stackBalance += op.totalStackPush();
    if(block.stackBalance > block.stackMax) block.stackMax = block.stackBalance;
  }
}

std::string Verifier::disassembleBlock(Block &block) {
  bcReader.seek_abs(block.start);
  OpcodeData<void> op;
  
  while(bcReader.pos < block.end) {
    op.readStatic(&bcReader);
    
    printf("[%ld,-%d,+%d] %s\\l", op.pos, op.totalStackPop(file), op.totalStackPush(), op.disassemble(file).c_str());
  }
  
  return "";
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
  
  return str;
}

void Verifier::decompileBlock(Block &block) {
#define __push(n) stack.push(ast::NodePtr(n))
#define __out(n) statements.push_back(ast::NodePtr(n))
  
  bcReader.seek_abs(block.start);
  
  std::stack<ast::NodePtr> stack;
  std::vector<ast::NodePtr> statements;
  
  bool negateJumpCondition = false;
  ast::NodePtr jumpCondition;
  
  OpcodeData<ast::Node> op;
  
  while(bcReader.pos < block.end) {
    op.read(&bcReader, file, this, stack);
    
    switch(op.code) {
#pragma mark Load and store instructions
      case Opcode::op_getlocal_0:
      case Opcode::op_getlocal_1:
      case Opcode::op_getlocal_2:
      case Opcode::op_getlocal_3: op.registerIndex = op.code - Opcode::op_getlocal_0;
      case Opcode::op_getlocal  : __push(new ast::LocalNode(op.registerIndex, *minfo)); break;
        
      case Opcode::op_setlocal_0:
      case Opcode::op_setlocal_1:
      case Opcode::op_setlocal_2:
      case Opcode::op_setlocal_3: op.registerIndex = op.code - Opcode::op_setlocal_0;
      case Opcode::op_setlocal  : {
        ast::LocalNode *lhs = new ast::LocalNode(op.registerIndex, *minfo);
        __out(new ast::DiadicNode(op.value1, ast::NodePtr(lhs), "=", 0));
      }; break;
        
      case Opcode::op_kill: {
        ast::LocalNode *lhs = new ast::LocalNode(op.registerIndex, *minfo);
        __out(new ast::DiadicNode(std::make_shared<ast::ConstantNode>("undefined"), ast::NodePtr(lhs), "=", 0));
      }; break;
        
#pragma mark Airthmetic instructions
      case Opcode::op_increment  : __push(new ast::DiadicNode(std::make_shared<ast::IntNode>(1), op.value1, "+", 0)); break;
      case Opcode::op_increment_i: __push(new ast::DiadicNode(std::make_shared<ast::IntNode>(1), op.value1, "+", 0)); break; // TODO: int cast.
        
      case Opcode::op_add       :
      case Opcode::op_add_i     : __push(new ast::DiadicNode(op.value2, op.value1, "+", 0)); break;
      case Opcode::op_subtract  :
      case Opcode::op_subtract_i: __push(new ast::DiadicNode(op.value2, op.value1, "-", 0)); break;
      case Opcode::op_multiply  :
      case Opcode::op_multiply_i: __push(new ast::DiadicNode(op.value2, op.value1, "*", 0)); break;
      case Opcode::op_divide    : __push(new ast::DiadicNode(op.value2, op.value1, "/", 0)); break;
      case Opcode::op_negate    :
      case Opcode::op_negate_i  : __push(new ast::MonadicNode(op.value1, "-", 0)); break;
        
        // constants
      case Opcode::op_pushnull     :
      case Opcode::op_pushundefined:
      case Opcode::op_pushtrue     :
      case Opcode::op_pushfalse    :
      case Opcode::op_pushnan      :
        
        // pool constants
      case Opcode::op_pushstring   :
      case Opcode::op_pushint      :
      case Opcode::op_pushdouble   :
      case Opcode::op_pushnamespace:
        
        // immediate values
      case Opcode::op_pushbyte     :
      case Opcode::op_pushshort    : __push(op.constant); break;
        
        // scope management
      case Opcode::op_pushwith: __out(new ast::WithNode(op.value1)); break;
        
        // management
      case Opcode::op_pop : __out(op.value1); break;
      case Opcode::op_swap: stack.push(op.value2); stack.push(op.value1); break;
      case Opcode::op_dup: {
        ast::NodePtr dupTmp = std::make_shared<ast::ConstantNode>("duptmp");
        __out(new ast::DiadicNode(op.value1, dupTmp, "=", 0));
        
        stack.push(dupTmp);
        stack.push(dupTmp);
      }; break;
        
#pragma mark Function invocation and return instructions
        // invocation
      case Opcode::op_call: {
        __push(new ast::CallNode(op.value1, op.arguments)); // TODO: op.value2
      }; break;
        
      case Opcode::op_callproperty:
      case Opcode::op_callpropvoid: {
        // TODO: op.value1
        
        ast::NodePtr method = op.value1->getProperty(op.multiname);
        ast::NodePtr ret = std::make_shared<ast::CallNode>(method, op.arguments);
        
        if(op.code == Opcode::op_callpropvoid)
          __out(ret);
        else
          __push(ret);
      }; break;
        
        // return
      case Opcode::op_returnvoid : __out(new ast::ReturnNode(NULL)); break;
      case Opcode::op_returnvalue: __out(new ast::ReturnNode(op.value1)); break;
        
      case Opcode::op_getlex        : __push(std::make_shared<ast::VoidNode>()->getProperty(op.multiname)); break;
      case Opcode::op_findproperty  :
      case Opcode::op_findpropstrict: __push(new ast::VoidNode()); break;
        
      case Opcode::op_setproperty   :
      case Opcode::op_initproperty  : __out(op.value1->setProperty(op.multiname, op.value2)); break;
      case Opcode::op_getproperty   : __push(op.value1->getProperty(op.multiname)); break;
      
      case Opcode::op_not          : __push(new ast::MonadicNode(op.value1, "!", 0)); break;
      case Opcode::op_equals       : __push(new ast::DiadicNode(op.value2, op.value1, "==", 0)); break;
      case Opcode::op_lessthan     : __push(new ast::DiadicNode(op.value2, op.value1, "<" , 0)); break;
      case Opcode::op_greaterthan  : __push(new ast::DiadicNode(op.value2, op.value1, ">" , 0)); break;
      case Opcode::op_lessequals   : __push(new ast::DiadicNode(op.value2, op.value1, "<=", 0)); break;
      case Opcode::op_greaterequals: __push(new ast::DiadicNode(op.value2, op.value1, ">=", 0)); break;
        
      case Opcode::op_constructsuper: __out(new ast::CommentNode("super()")); break;
        
      case Opcode::op_pushscope: break;
      case Opcode::op_popscope : break;
      case Opcode::op_coerce_a : __push(op.value1); break;
        
      case Opcode::op_label: break;
        
      case Opcode::op_ifnlt: negateJumpCondition = true; jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, "<" , 0); break;
      case Opcode::op_ifnle: negateJumpCondition = true; jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, "<=", 0); break;
      case Opcode::op_ifngt: negateJumpCondition = true; jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, ">" , 0); break;
      case Opcode::op_ifnge: negateJumpCondition = true; jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, ">=", 0); break;
        
      case Opcode::op_jump   : /*jumpCondition = std::make_shared<ast::ConstantNode>("true")*/; break;
      case Opcode::op_iftrue : jumpCondition = op.value1; break;
      case Opcode::op_iffalse: jumpCondition = std::make_shared<ast::MonadicNode>(op.value1, "!", 0); break;
        
      case Opcode::op_ifeq: jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, "==", 0); break;
      case Opcode::op_ifne: jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, "!=", 0); break;
      case Opcode::op_iflt: jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, "<" , 0); break;
      case Opcode::op_ifle: jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, "<=", 0); break;
      case Opcode::op_ifgt: jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, ">" , 0); break;
      case Opcode::op_ifge: jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, ">=", 0); break;
        
      case Opcode::op_ifstricteq: jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, "===", 0); break;
      case Opcode::op_ifstrictne: jumpCondition = std::make_shared<ast::DiadicNode>(op.value2, op.value1, "!==", 0); break;
        
#pragma mark Default
      default:
        if(!(op.opcode.flags & Opcode::IsBranchFlag))
          printf("Not implemented\n");
    } // end switch
    
    // clean up
    op.reset();
  }
  
  for(auto it = statements.begin(); it != statements.end(); ++it) {
    std::string stm = (*it)->toString();
    stm = replaceAll(stm, "\"", "\\\"");
    printf("%s;\\l", stm.c_str());
  }
  
  if(jumpCondition != NULL) {
    if(statements.size() > 0) printf("\\l");
    printf("jump if %s(%s)", negateJumpCondition ? "not " : "", jumpCondition->toString().c_str());
  }
}