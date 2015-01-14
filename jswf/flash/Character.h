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
#include <cstddef>

namespace jswf {
  namespace avm2 {
    class Class;
  }
  
  namespace flash {
    /**
     * Represents a character for the document's <tt>DICTIONARY</tt>.
     */
    class Character {
    public:
      avm2::Class *avm2Class = NULL;
      
      uint16_t id;
      virtual ~Character() {}
    };
  }
}

#endif