//
//  TagWithDictionaryElement.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_TagWithDictionaryElement_h
#define jswf_TagWithDictionaryElement_h

#include "TagWithReader.h"
#include "DictionaryElement.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class TagWithDictionaryElement : public TagWithReader {
      public:
        std::shared_ptr<DictionaryElement> element;
        TagWithDictionaryElement(tag_type_t type, std::string &payload) : TagWithReader(type, payload) {}
      };
    }
  }
}

#endif