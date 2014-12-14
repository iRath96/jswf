//
//  SetBackgroundColorTag.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_SetBackgroundColorTag_h
#define jswf_SetBackgroundColorTag_h

#include "TagWithReader.h"
#include "Reader.h"
#include "Header.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class SetBackgroundColorTag : public TagWithReader {
      public:
        RGBA color;
        SetBackgroundColorTag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          flashReader.readRGB(color);
        }
      };
    }
  }
}

#endif