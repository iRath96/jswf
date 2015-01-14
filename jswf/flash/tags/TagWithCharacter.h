//
//  TagWithCharacter.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_TagWithCharacter_h
#define jswf_TagWithCharacter_h

#include "ITagForDocument.h"
#include "Character.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Super-class for <tt>TAG</tt>s that define a character for the document's <tt>DICTIONARY</tt>
       * (eg DefineShapeTag, DefineButtonTag).
       * @todo It's not nice to implement a function in an interface!
       */
      class TagWithCharacter : public ITagForDocument {
      public:
        std::shared_ptr<Character> character; //!< A <tt>shared_ptr</tt> to a polymorphistic character described by this <tt>TAG</tt>
        void applyToDocument(Document &document) {
          document.dictionary[character->id] = character;
        }
        
        virtual ~TagWithCharacter() {}
      };
    }
  }
}

#endif