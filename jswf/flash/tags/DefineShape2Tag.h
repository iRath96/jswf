//
//  DefineShape2Tag.h
//  jswf
//
//  Created by Alexander Rath on 15.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineShape2Tag_h
#define jswf_DefineShape2Tag_h

#include "DefineShapeTag.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Extends \ref DefineShapeTag with support for
       * up to 65534 (0xfffe) elements for `FillStyleArray` and `LineStyleArray`.
       */
      class DefineShape2Tag : public DefineShapeTag {
      protected:
        virtual void readBetween() {}
        virtual uint16_t readArrayCount() { //!< \todo Document this. Especially throw.
          uint16_t count = reader->readU8();
          if(count == 0xff) count = reader->readU16();
          if(count == 0xffff) throw "Array overflow (count=0xff)";
          printf("count: %x\n", count);
          return count;
        }
      public:
        DefineShape2Tag(tag_type_t t, std::string &p) : DefineShapeTag(t, p, false) { read(); }
        DefineShape2Tag(tag_type_t t, std::string &p, bool) : DefineShapeTag(t, p, false) {}
      };
    }
  }
}

#endif