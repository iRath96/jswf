//
//  MethodInfo.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_MethodInfo_h
#define jswf_MethodInfo_h

#include "types.h"
#include "Multiname.h"
#include "TraitInfo.h"

namespace jswf {
  namespace avm2 {
    struct OptionDetail {
      u30_t value;
      ConstantKind::Enum kind;
    };
    
    struct ExceptionInfo {
      u30_t from, to;
      u30_t target;
      u30_t excType;
      u30_t varName;
    };
    
    class ABCFile;
    struct MethodBody;
    struct MethodInfo {
      enum Flags : u8_t {
        NeedsArgumentsFlag = 0x01,
        NeedsActivationFlag = 0x02,
        NeedsRestFlag = 0x04,
        HasOptionalFlag = 0x08,
        SetDxnsFlag = 0x40,
        HasParamNamesFlag = 0x80
      };
      
      u30_t paramCount; // Hey, you can pass up to 1,073,741,823 parameters to a function!
      MultinamePtr returnType;
      std::vector<MultinamePtr> paramTypes;
      
      string *name;
      Flags flags;
      
      std::vector<OptionDetail> options;
      std::vector<string *> paramNames;
      
      MethodBody *body = NULL;
      
      ABCFile *file;
    };
    
    struct MethodBody {
      MethodInfo *method;
      
      u30_t maxStack, localCount;
      u30_t initScopeDepth, maxScopeDepth;
      string code;
      std::vector<ExceptionInfo> exceptions;
      std::vector<std::shared_ptr<TraitInfo>> traits;
    };
  }
}

#endif