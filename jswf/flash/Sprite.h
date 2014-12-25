//
//  Sprite.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__Sprite__
#define __jswf__Sprite__

#include <stdio.h>
#include "DictionaryElement.h"
#include "Frame.h"
#include "Tag.h"

#include <vector>

namespace jswf {
  namespace flash {
    class Sprite : public DictionaryElement {
    public:
      std::vector<std::shared_ptr<tags::Tag>> tags;
      std::vector<Frame> frames;
      uint16_t frameCount;
      
      uint16_t currentFrame = 0;
      bool isPlaying = true;
    };
  }
}

#endif /* defined(__jswf__Sprite__) */