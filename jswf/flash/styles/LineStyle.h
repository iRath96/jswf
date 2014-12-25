//
//  LineStyle.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_LineStyle_h
#define jswf_LineStyle_h

#include "Header.h"

namespace jswf {
  namespace flash {
    namespace styles {
      class LineStyle;
      typedef std::shared_ptr<LineStyle> LineStylePtr;
      
      class LineStyle {
      public:
        uint32_t id;
        
        uint16_t width;
        RGBA color;
      };
    }
  }
}

#endif