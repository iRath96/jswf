//
//  DictionaryElement.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DictionaryElement_h
#define jswf_DictionaryElement_h

#include <stdint.h>

namespace jswf {
  namespace flash {
    class DictionaryElement {
    public:
      uint16_t id;
      virtual ~DictionaryElement() {}
    };
  }
}

#endif