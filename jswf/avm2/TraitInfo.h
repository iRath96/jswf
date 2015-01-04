//
//  TraitInfo.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_TraitInfo_h
#define jswf_TraitInfo_h

#include "types.h"
#include <vector>

#include "ConstantKind.h"
#include "Multiname.h"

namespace jswf {
  namespace avm2 {
    struct TraitInfo {
      /**
       * Describes kinds of \ref TraitInfo .
       */
      enum Kind : u8_t {
        SlotKind     = 0, //!< Member definition, use \ref SlotTraitInfo
        MethodKind   = 1, //!< Method definition, use \ref MethodTraitInfo
        GetterKind   = 2, //!< Getter definition, use \ref MethodTraitInfo
        SetterKind   = 3, //!< Setter definition, use \ref MethodTraitInfo
        ClassKind    = 4, //!< Class definition, use \ref ClassTraitInfo
        FunctionKind = 5, //!< Function definition, use \ref FunctionTraitInfo
        ConstKind    = 6  //!< Constant member definition, use \ref SlotTraitInfo
      };
      
      /**
       * Describes attributes of \ref TraitInfo .
       */
      enum Attributes : u8_t {
        FinalAttribute    = 0x1,
        OverrideAttribute = 0x2,
        MetadataAttribute = 0x4
      };
      
      MultinamePtr name;
      
      Kind kind;
      Attributes attributes;
      
      // u8_t data[]
      
      std::vector<u30_t> metadata;
      
      virtual ~TraitInfo() {}
    };
    
    struct SlotTraitInfo : TraitInfo { // kind = 0, 6
      u30_t slotId;
      MultinamePtr typeName;
      
      u30_t vindex;
      ConstantKind::Enum vkind;
    };
    
    struct ClassInfo;
    struct ClassTraitInfo : TraitInfo { // kind = 4
      u30_t slotId;
      ClassInfo *classInfo;
    };
    
    struct MethodInfo;
    struct FunctionTraitInfo : TraitInfo { // kind = 5
      u30_t slotId;
      MethodInfo *methodInfo;
    };
    
    struct MethodTraitInfo : TraitInfo { // kind = 1, 2, 3
      u30_t dispId;
      MethodInfo *methodInfo;
    };
  }
}

#endif