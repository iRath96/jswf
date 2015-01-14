//
//  DefineButton2.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineButton2Tag_h
#define jswf_DefineButton2Tag_h

#include "DefineButtonTag.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Extends DefineButtonTag with support for <tt>TrackAsMenu</tt>.
       * @todo "by allowing any state transition to trigger actions."... what?
       */
      class DefineButton2Tag : public DefineButtonTag {
      protected:
        virtual void readBetween() {
          uint8_t reserved = reader->readUB(7);
          assert(reserved == 0);
          
          button->trackAsMenu = reader->readUB(1);
          
          reader->readU16(); // action offset, not needed.
        }
      public:
        DefineButton2Tag(tag_type_t t, std::string &p) : DefineButtonTag(t, p, false) { read(); }
        DefineButton2Tag(tag_type_t t, std::string &p, bool) : DefineButtonTag(t, p, false) {}
      };
    }
  }
}

#endif