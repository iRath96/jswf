//
//  Frame.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__Frame__
#define __jswf__Frame__

#include <stdio.h>
#include <map>

#include "Header.h"

namespace jswf {
  namespace flash {
    struct DisplayListEntry {
      uint16_t characterId;
      
      bool doesClip = false;
      uint16_t clipDepth;
      
      bool setsColorTransform = false;
      
      Matrix matrix;
      ColorTransform colorTransform;
    };
    
    class Frame {
    public:
      std::map<uint16_t, DisplayListEntry> displayList;
    };
  }
}

#endif /* defined(__jswf__Frame__) */