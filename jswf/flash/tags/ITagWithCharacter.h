//
//  ITagWithCharacter.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_ITagWithCharacter_h
#define jswf_ITagWithCharacter_h

#include "ITagForDocument.h"
#include "Character.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Serves as interface for `TAG`s that
       * define a character for the document's `DICTIONARY`
       * (eg \ref DefineShapeTag, \ref DefineButtonTag).
       * @todo It's not nice to implement a function in an interface!
       */
      class ITagWithCharacter : public ITagForDocument {
      public:
        std::shared_ptr<Character> character; //!< A `shared_ptr` to a polymorphistic character described by this `TAG`
        void applyToDocument(Document &document) {
          document.dictionary[character->id] = character;
        }
        
        virtual ~ITagWithCharacter() {}
      };
    }
  }
}

#endif