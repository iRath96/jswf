//
//  EndTag.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_EndTag_h
#define jswf_EndTag_h

#include "Tag.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Marks the end of a tag array and thereby the end of a \ref Document or \ref Sprite.
       * @see flash::Document
       * @see flash::tags::DefineSpriteTag
       */
      class EndTag : public Tag {
      public:
        EndTag(tag_type_t t, std::string &p) : Tag(t, p) {}
      };
    }
  }
}

#endif