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
       * Describes kinds of TraitInfo .
       */
      enum Kind : u8_t {
        SlotKind     = 0, //!< Member definition, use SlotTraitInfo
        MethodKind   = 1, //!< Method definition, use MethodTraitInfo
        GetterKind   = 2, //!< Getter definition, use MethodTraitInfo
        SetterKind   = 3, //!< Setter definition, use MethodTraitInfo
        ClassKind    = 4, //!< Class definition, use ClassTraitInfo
        FunctionKind = 5, //!< Function definition, use FunctionTraitInfo
        ConstKind    = 6  //!< Constant member definition, use SlotTraitInfo
      };
      
      /**
       * Describes attributes of TraitInfo .
       */
      enum Attributes : u8_t {
        FinalAttribute    = 0x1,
        OverrideAttribute = 0x2,
        MetadataAttribute = 0x4
      };
      
      MultinamePtr name; //!< The name of this trait.
      
      Kind kind; //!< The kind of this trait.
      Attributes attributes; //!< Attributes (flags) of this trait.
      
      // u8_t data[]
      
      std::vector<u30_t> metadata; //!< Metadata for this trait
      
      virtual ~TraitInfo() {}
    };
    
    struct SlotTraitInfo : TraitInfo { // kind = 0, 6
      u30_t slotId;
      MultinamePtr typeName;
      
      u30_t vindex;
      ConstantKind::Enum vkind;
      
      SlotTraitInfo() { kind = TraitInfo::SlotKind; }
      SlotTraitInfo(u30_t slotId, const MultinamePtr &name, const MultinamePtr &type, u30_t vindex, ConstantKind::Enum vkind) :
      slotId(slotId), typeName(type), vindex(vindex), vkind(vkind) {
        this->name = name;
        kind = TraitInfo::SlotKind;
      }
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