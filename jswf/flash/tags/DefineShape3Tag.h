//
//  DefineShape3Tag.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineShape3Tag_h
#define jswf_DefineShape3Tag_h

#include "DefineShapeTag.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class DefineShape3Tag : public DefineShapeTag {
        virtual void readColor(RGBA &rgba) {
          printf("readRGBA\n");
          flashReader.readRGBA(rgba);
        }
      public:
        DefineShape3Tag(tag_type_t t, std::string &p) : DefineShapeTag(t, p, false) { read(); }
      };
    }
  }
}

#endif