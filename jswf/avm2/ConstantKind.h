//
//  ConstantKind.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_ConstantKind_h
#define jswf_ConstantKind_h

#include "types.h"
#include "Namespace.h"

namespace jswf {
  namespace avm2 {
    struct ConstantKind : NamespaceKind {
      enum Enum : u8_t {
        IntKind = 0x03,
        UIntKind = 0x04,
        DoubleKind = 0x06,
        UTF8Kind = 0x01,
        
        // constants
        
        FalseKind = 0x0a,
        TrueKind = 0x0b,
        NullKind = 0x0c,
        UndefinedKind = 0x00,
      };
    };
  }
}

#endif