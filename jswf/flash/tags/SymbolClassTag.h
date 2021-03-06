//
//  SymbolClassTag.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_SymbolClassTag_h
#define jswf_SymbolClassTag_h

#include "ITagForDocument.h"
#include "TagWithReader.h"

#include "macros.h"

namespace jswf {
  namespace flash {
    /**
     * Describes a (<tt>Character</tt>, class-name) tuple.
     */
    struct SymbolClass {
      uint16_t characterId;
      string className;
    };
    
    namespace tags {
      /**
       * Assigns class-names of AVM2 classes to <tt>Character</tt>s.
       */
      class SymbolClassTag : public TagWithReader, public ITagForDocument {
      public:
        std::vector<SymbolClass> symbolClasses; //!< A list of (<tt>Character</tt>, class-name) tuples.
        SymbolClassTag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          __read_array(sc, symbolClasses, reader->readU16(), {
            sc.characterId = reader->readU16();
            sc.className = reader->readString();
          });
        }
        
        void applyToDocument(Document &document) {
          for(auto it = symbolClasses.begin(); it != symbolClasses.end(); ++it)
            document.dictionary[it->characterId]->avm2Class = document.avm2.getClassByName(it->className);
        }
      };
    }
  }
}

#endif