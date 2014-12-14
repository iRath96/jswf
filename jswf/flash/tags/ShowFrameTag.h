//
//  ShowFrameTag.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_ShowFrameTag_h
#define jswf_ShowFrameTag_h

#include "Tag.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class ShowFrameTag : public Tag {
      public:
        ShowFrameTag(tag_type_t t, std::string &p) : Tag(t, p) {}
      };
    }
  }
}

#endif