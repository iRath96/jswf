//
//  Character.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_Character_h
#define jswf_Character_h

#include <stdint.h>

namespace jswf {
  namespace flash {
    /**
     * Represents a character for the document's `DICTIONARY`.
     */
    class Character {
    public:
      uint16_t id;
      virtual ~Character() {}
    };
  }
}

#endif