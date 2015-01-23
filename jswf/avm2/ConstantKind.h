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
    struct ConstantKind {
      enum Enum : u8_t {
        IntKind = 0x03,
        UIntKind = 0x04,
        DoubleKind = 0x06,
        UTF8Kind = 0x01,
        
        // constant values
        
        FalseKind = 0x0a,
        TrueKind = 0x0b,
        NullKind = 0x0c,
        UndefinedKind = 0x00,
        
        // inherit namespace kinds
        namespace_kinds
        
        // further kinds that jswf uses
        
        NANKind = 0x80, // for pushnan
        ByteKind = 0x81, // for pushbyte
        ShortKind = 0x82, // for pushshort
      };
      
      static bool needsPool(ConstantKind::Enum kind) {
        return !(kind == UndefinedKind || kind == FalseKind || kind == TrueKind || kind == NullKind || kind == NANKind);
      }
    };
  }
}

#endif