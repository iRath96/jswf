//
//  ITagForSprite.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_ITagForSprite_h
#define jswf_ITagForSprite_h

#include "Sprite.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class ITagForSprite {
      public:
        virtual void applyToSprite(Sprite &sprite) = 0;
        virtual ~ITagForSprite() {}
      };
    }
  }
}

#endif