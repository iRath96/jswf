//
//  ShowFrameTag.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_ShowFrameTag_h
#define jswf_ShowFrameTag_h

#include "ITagForSprite.h"
#include "Tag.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Finalizes the current temporary frame and adds it to the `frames` of a \ref Document or \ref Sprite .
       * @todo Also some kind of Frame-modification super-class, but requires more parameters.
       */
      class ShowFrameTag : public Tag, public ITagForSprite {
      public:
        ShowFrameTag(tag_type_t t, std::string &p) : Tag(t, p) {}
        void applyToSprite(Sprite &sprite) {
          sprite.frames.push_back(sprite.temporaryFrame);
        }
      };
    }
  }
}

#endif