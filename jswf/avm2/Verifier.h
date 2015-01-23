//
//  Verifier.h
//  jswf
//
//  Created by Alexander Rath on 20.01.15.
//  Copyright (c) 2015 Alexander Rath. All rights reserved.
//

/**
 * @file
 * Defines jswf::avm2::Verifier.
 */

#ifndef __jswf__Verifier__
#define __jswf__Verifier__

#include "StringReader.h"
#include "Opcode.h"

#include "Node.h"

#include <string>
#include <map>

namespace jswf {
  namespace avm2 {
    class ABCFile;
    
    struct Block {
      size_t id;
      
      bool analyzed = false;
      size_t start, end;
      std::vector<Block *> predecessors, successors;
      
      Block(size_t id, size_t start) : id(id), start(start) {}
      
      // second phase
      
      int stackBalance = 0;
      int stackMin = 0, stackMax = 0;
    };
    
    /**
     * Verifies bytecode.
     */
    class Verifier : public IConstantSource<ast::Node> {
    public:
      ABCFile *origin;
      io::StringReader bcReader;
      Block **bcMap;
      std::vector<Block *> blocks;
      ABCFile *file;
      
      MethodInfo *minfo;
      
      ast::NodePtr getPoolConstant(ConstantKind::Enum kind, ABCFile *file, u30_t index) {
        switch(kind) {
          case ConstantKind::UTF8Kind: return std::make_shared<ast::StringNode>(file->constantPool.strings[index]);
          default: return std::make_shared<ast::ConstantNode>("wtf:constant:" + std::to_string(index));
        }
      }
      
      ast::NodePtr getConstant(ConstantKind::Enum kind) {
        switch(kind) {
          case ConstantKind::TrueKind     : return std::make_shared<ast::ConstantNode>("true");
          case ConstantKind::FalseKind    : return std::make_shared<ast::ConstantNode>("false");
          case ConstantKind::NANKind      : return std::make_shared<ast::ConstantNode>("nan");
          case ConstantKind::UndefinedKind: return std::make_shared<ast::ConstantNode>("undefined");
          case ConstantKind::NullKind     : return std::make_shared<ast::ConstantNode>("null");
          default: return std::make_shared<ast::ConstantNode>("wtf:constant");
        }
      }
      
      ast::NodePtr getImmediateConstant(ConstantKind::Enum kind, int32_t imm) {
        return std::make_shared<ast::IntNode>(imm);
      }
      
      void analyzeBlock(Block &block);
      void analyzeStackUsage(Block &block);
      
      std::string disassembleBlock(Block &block);
      void decompileBlock(Block &block);
      
      void verifyBytecode(const std::string bytecode) {
        bcReader = io::StringReader(bytecode);
        bcMap = new Block *[bytecode.size()]();
        
        Block *startBlock = new Block(0, 0);
        blocks.push_back(startBlock);
        
        size_t lastSize = 0;
        while(blocks.size() > lastSize) {
          size_t i = lastSize;
          lastSize = blocks.size();
          for(size_t j = blocks.size(); i < j; ++i)
            if(!blocks[i]->analyzed) {
              analyzeBlock(*blocks[i]); // analyze all new blocks
              blocks[i]->analyzed = true;
            }
        }
        
        // output all blocks
        
        // do some funky GraphViz!
        printf("digraph {\n");
        
        for(size_t i = 0, j = blocks.size(); i < j; ++i) {
          Block &b = *blocks[i];
          analyzeStackUsage(b);
          
          printf("  L%ld[shape=\"box\",label=\"L%ld (%d, %d to %d)\\n\\l", i, b.id, b.stackBalance, b.stackMin, b.stackMax);
          disassembleBlock(b);
          printf("\n");
          decompileBlock(b);
          printf("\"];\n");
          
          for(size_t ii = 0, jj = b.successors.size(); ii < jj; ++ii) {
            printf("  L%ld -> L%ld", i, b.successors[ii]->id);
            if(ii) printf(" [style=\"dashed\"]");
            printf(";\n");
          }
        }
        
        printf("}");
        
        if(blocks.size() > 2)
          printf("");
        
        delete[] bcMap;
      }
    };
  }
}

#endif /* defined(__jswf__Verifier__) */