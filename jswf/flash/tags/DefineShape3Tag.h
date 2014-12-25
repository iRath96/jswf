//
//  DefineShape3Tag.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineShape3Tag_h
#define jswf_DefineShape3Tag_h

#include "DefineShape2Tag.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class DefineShape3Tag : public DefineShape2Tag {
      protected:
        virtual void readBetween() {}
        virtual void readColor(RGBA &rgba) {
          printf("readRGBA\n");
          flashReader.readRGBA(rgba);
        }
      public:
        DefineShape3Tag(tag_type_t t, std::string &p) : DefineShape2Tag(t, p, false) { read(); }
        DefineShape3Tag(tag_type_t t, std::string &p, bool) : DefineShape2Tag(t, p, false) {}
      };
    }
  }
}

#endif