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
  while(bcReader.pos < block.end) {
    Opcode::Code code = (Opcode::Code)bcReader.readU8();
    Opcode opcode = opcodes[code];
    
    if(opcode.flags & Opcode::HasImmediateValueFlag)
      switch(opcode.constantKind) {
        case ConstantKind::ByteKind : bcReader.readS8(); break;
        case ConstantKind::ShortKind: bcReader.readVU30(); break;
        default: break;
      }
    
    block.stackBalance -= opcode.stackPop;
    
    if(opcode.flags & Opcode::HasMultinameFlag) {
      Multiname &mn = *file->constantPool.multinames[bcReader.readVU30()];
      if(!mn.hasName) block.stackBalance -= 1;
      if(!(mn.hasNS || mn.hasNSSet)) block.stackBalance -= 1;
    }
    
    if(opcode.flags & Opcode::HasArgumentCountFlag) block.stackBalance -= bcReader.readVU30();
    if(opcode.flags & Opcode::HasRegisterFlag) bcReader.readVU30();
    if(opcode.flags & Opcode::IsBranchFlag) bcReader.readS24();
    if((opcode.flags & Opcode::HasPoolConstantFlag) == Opcode::HasPoolConstantFlag) bcReader.readVU30();
    
    if(block.stackBalance < block.stackMin) block.stackMin = block.stackBalance;
    
    block.stackBalance += opcode.stackPush;
    if(block.stackBalance > block.stackMax) block.stackMax = block.stackBalance;
  }
}

std::string Verifier::disassembleBlock(Block &block) {
  bcReader.seek_abs(block.start);
  OpcodeData<void> op;
  
  while(bcReader.pos < block.end) {
    op.readStatic(&bcReader);
    printf("[%ld] %s\\l", op.pos, op.disassemble(file).c_str());
  }
  
  return "";
}