//
//  Opcode.h
//  jswf
//
//  Created by Alexander Rath on 20.01.15.
//  Copyright (c) 2015 Alexander Rath. All rights reserved.
//

#ifndef jswf_Opcode_h
#define jswf_Opcode_h

#include "ConstantKind.h"
#include "Multiname.h"
#include "GenericReader.h"
#include "ABCFile.h"

#include <stack>

// TODO:2015-01-20:alex:Implement setsuper

#include "ops-macro.h"

namespace jswf {
  namespace avm2 {
    struct Opcode;
    extern Opcode opcodes[256];
    
    struct Opcode {
      const char *name;
      
      ConstantKind::Enum constantKind;
      
      int stackPop, stackPush;
      int scopeBalance;
      
      enum Flags {
        NoFlag               = 0,
        
        HasMultinameFlag     = 1,
        MultinameIsQNameFlag = 2,
        HasQNameFlag         = 1|2,
        
        IsBranchFlag         = 4,
        HasArgumentCountFlag = 8, //!< Number of additional stack elements to pop, typical of literal construction or method invocations
        
        HasConstantFlag         = 16, // a ConstantKind that does not need an index
        ConstantIsPoolConstFlag = 32, // a ConstantKind that needs an index
        HasPoolConstantFlag     = 16|32,
        
        HasImmediateValueFlag   = 64, // a ConstantKind but with immediate value
        
        HasRegisterFlag         = 128,
        HasImplicitRegisterFlag = 256, //
        
        IsCustomLengthFlag = 512, // e.g. lookupswitch
        IsCustomStackFlag = 1024, // e.g. newobject
      };
      
      enum Code {
        op_invalid = 0x00,
#define op(name, code, ...) op_ ## name = code,
        ops
#undef op
      };
      
      Opcode::Code code;
      Opcode::Flags flags;
    };
    
    class ABCFile;
    
    template<typename T>
    class IConstantSource {
    public:
      virtual std::shared_ptr<T> getPoolConstant(ConstantKind::Enum kind, ABCFile *file, u30_t index) = 0;
      virtual std::shared_ptr<T> getConstant(ConstantKind::Enum kind) = 0;
      virtual std::shared_ptr<T> getImmediateConstant(ConstantKind::Enum kind, int32_t imm) = 0;
    };
    
    template<typename T>
    struct OpcodeData {
      size_t pos;
      
      Opcode::Code code;
      Opcode opcode;
      
      u30_t registerIndex = 0;
      s24_t branchOffset = 0;
      
      u30_t poolIndex = 0;
      std::shared_ptr<T> constant;
      
      u30_t argumentCount = 0;
      std::vector<std::shared_ptr<T>> arguments;
      
      std::shared_ptr<T> value1, value2;
      
      u30_t multinameIndex = 0;
      Multiname multiname;
      
      int32_t immediateInt;
      
      inline int totalStackPop() { return opcode.stackPop + argumentCount + (multiname.hasName ? 0 : 1) + (multiname.hasNS || multiname.hasNSSet ? 0 : 1); }
      
      std::string disassemble(ABCFile *file) {
        std::string dis = opcode.code == Opcode::op_invalid ? "op_" + std::to_string(code) : std::string(opcode.name);
        
        if(opcode.flags & Opcode::HasImmediateValueFlag)
          dis += " " + std::to_string(immediateInt);
        
        if(opcode.flags & Opcode::HasMultinameFlag    ) dis += " " + file->constantPool.multinames[multinameIndex]->nameString();
        if(opcode.flags & Opcode::HasArgumentCountFlag) dis += "[" + std::to_string(argumentCount) + "]";
        if(opcode.flags & Opcode::HasRegisterFlag     ) dis += " loc" + std::to_string(registerIndex);
        if((opcode.flags & Opcode::HasPoolConstantFlag) == Opcode::HasPoolConstantFlag) {
          switch(opcode.constantKind) {
            case ConstantKind::UTF8Kind: dis += " \\\"" + file->constantPool.strings[poolIndex] + "\\\""; break;
            default: dis += " const:" + std::to_string(poolIndex);
          }
        }
        
        if(opcode.flags & Opcode::IsBranchFlag) dis += (branchOffset > 0 ? " +" : " ") + std::to_string(branchOffset);
        
        return dis;
      }
      
      void readStatic(io::GenericReader *reader) {
        pos = reader->pos;
        
        code = (Opcode::Code)reader->readU8();
        opcode = opcodes[code];
        
        if(opcode.flags & Opcode::HasMultinameFlag) multinameIndex = reader->readVU30();
        if(opcode.flags & Opcode::HasArgumentCountFlag) argumentCount = reader->readVU30();
        if(opcode.flags & Opcode::HasRegisterFlag) registerIndex = reader->readVU30();
        if(opcode.flags & Opcode::IsBranchFlag) branchOffset = reader->readS24();
        
        if((opcode.flags & Opcode::HasPoolConstantFlag) == Opcode::HasPoolConstantFlag) poolIndex = reader->readVU30();
        
        if(opcode.flags & Opcode::HasImmediateValueFlag)
          switch(opcode.constantKind) {
            case ConstantKind::ByteKind : immediateInt = reader->readS8  (); break;
            case ConstantKind::ShortKind: immediateInt = reader->readVU30(); break;
            default: immediateInt = 0;
          }
      }
      
      void read(io::GenericReader *reader, ABCFile *file, IConstantSource<T> *source, std::stack<std::shared_ptr<T>> &stack) {
#define __pop (top = stack.top(), stack.pop(), top)
        
        pos = reader->pos;
        
        code = (Opcode::Code)reader->readU8();
        opcode = opcodes[code];
        
        std::shared_ptr<T> top; // for __pop
        
        // opcode argument order: (each part is optional)
        //   multiname_index, arg_count, register_index, branch_offset, pool_index, immediate
        // opcode stack order: (each part is optional)
        //   value1, multiname_ns, multiname_name, value2, arg0...argN
        
        if(opcode.flags & Opcode::HasMultinameFlag) multinameIndex = reader->readVU30();
        
        if(opcode.flags & Opcode::HasArgumentCountFlag) {
          argumentCount = reader->readVU30();
          for(uint32_t i = 0; i < argumentCount; ++i) arguments.insert(arguments.begin(), __pop);
        }
        
        if(opcode.stackPop > 1) value2 = __pop;
        
        if(opcode.flags & Opcode::HasMultinameFlag) {
          multiname = *file->constantPool.multinames[multinameIndex];
          if(opcode.flags & Opcode::MultinameIsQNameFlag) {
            if(!multiname.isQualified())
              throw "Error error error.";
          } else {
            if(!multiname.hasName) multiname.setName(__pop->coerce_s());
            if(!multiname.hasNS) multiname.setNS(std::make_shared<Namespace>(__pop->coerce_ns()));
          }
        }
        
        if(opcode.flags & Opcode::HasRegisterFlag) registerIndex = reader->readVU30();
        if(opcode.flags & Opcode::IsBranchFlag) branchOffset = reader->readS24();
        
        if((opcode.flags & Opcode::HasPoolConstantFlag) == Opcode::HasPoolConstantFlag)
          constant = source->getPoolConstant(opcode.constantKind, file, poolIndex = reader->readVU30());
        else if(opcode.flags & Opcode::HasConstantFlag)
          constant = source->getConstant(opcode.constantKind);
        
        if(opcode.stackPop > 0) value1 = __pop;
        
        if(opcode.flags & Opcode::HasImmediateValueFlag) {
          switch(opcode.constantKind) {
            case ConstantKind::ByteKind : immediateInt = reader->readS8(); break;
            case ConstantKind::ShortKind: immediateInt = reader->readVU30(); break;
            default: immediateInt = 0;
          }
          
          constant = source->getImmediateConstant(opcode.constantKind, immediateInt);
        }
        
#undef pop
      }
      
      void reset() {
        if(opcode.flags & Opcode::HasArgumentCountFlag) arguments.clear();
        if(opcode.flags & Opcode::HasConstantFlag) constant.reset();
        
        if(opcode.stackPop > 0) value1.reset();
        if(opcode.stackPop > 1) value2.reset();
      }
    };
    
    inline Opcode::Flags operator|(Opcode::Flags a, Opcode::Flags b) { return static_cast<Opcode::Flags>(static_cast<int>(a) | static_cast<int>(b)); }
  }
}

#undef ops

#endif