//
//  TagWithCharacter.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_TagWithCharacter_h
#define jswf_TagWithCharacter_h

#include "TagWithReader.h"
#include "Character.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Serves as super-class for `TAG`s that
       * define a character for the document's `DICTIONARY`
       * (eg \ref DefineShapeTag, \ref DefineButtonTag).
       */
      class TagWithCharacter : public TagWithReader {
      public:
        std::shared_ptr<Character> element;
        TagWithCharacter(tag_type_t type, std::string &payload) : TagWithReader(type, payload) {}
      };
    }
  }
}

#endif