//
//  Header.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__Header__
#define __jswf__Header__

#include <stdio.h>
#include <stdint.h>
#include "GenericReader.h"

namespace jswf {
  namespace flash {
    typedef uint8_t version_t;
    struct Compression {
      enum Enum {
        Uncompressed,
        ZLib
      };
    };
    
    struct Rect {
      sb_t x0, y0;
      sb_t x1, y1;
    };
    
    struct RGBA {
      uint8_t r, g, b, a;
    };
    
    struct Gradient {
      
    };
    
    struct FocalGradient : public Gradient {
      
    };
    
    struct Matrix {
      fb_t sx = 1, sy = 1; // scale
      fb_t r0 = 0, r1 = 0; // skew
      sb_t tx = 0, ty = 0; // translate
    };
    
    class Header {
    public:
      Compression::Enum compression;
      version_t version;
      uint32_t fileSize;
      uint16_t frameRate, // in 1/256 FPS
               frameCount;
      Rect rect;
    };
  }
}

#endif /* defined(__jswf__Header__) */