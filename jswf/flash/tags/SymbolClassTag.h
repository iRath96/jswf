//
//  SymbolClassTag.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_SymbolClassTag_h
#define jswf_SymbolClassTag_h

#include "TagWithReader.h"

namespace jswf {
  namespace flash {
    struct SymbolClass {
      uint16_t characterId;
      string className;
    };
    
    namespace tags {
      class SymbolClassTag : public TagWithReader {
      public:
        std::vector<SymbolClass> symbolClasses;
        SymbolClassTag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          __read_array(sc, symbolClasses, reader->readU16(), {
            sc.characterId = reader->readU16();
            sc.className = reader->readString();
          });
        }
      };
    }
  }
}

#endif