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
    
    /**
     * Represents `RECT` records.
     */
    struct Rect {
      sb_t x0, //!< The low x-coordinate.
           y0; //!< The low y-coordinate.
      sb_t x1, //!< The high x-coordinate.
           y1; //!< The high y-coordinate.
    };
    
    /**
     * Represents `RGB`, `RGBA` and `ARGB` color records.
     */
    struct RGBA {
      uint8_t r, //!< Red channel.
              g, //!< Green channel.
              b, //!< Blue channel.
              a; //!< Alpha channel.
    };
    
    /**
     * Represents `CXFORM` and `CXFORMWITHALPHA` records.
     * For each channel `v` from `{r,g,b,a}`, the transformed value `v'` can be computed as follows:
     * `v' = max(0, min((v * vM / 256) + vA, 255))`
     */
    struct ColorTransform {
      // r,g,b,a = color channels
      // M = multiplication, A = addition
      
      sb_t rM = 256, rA = 0;
      sb_t gM = 256, gA = 0;
      sb_t bM = 256, bA = 0;
      sb_t aM = 256, aA = 0;
    };
    
    /**
     * Represents `MATRIX` records.
     * `x' = x * sx + y * r1 + tx`
     * `y' = y * sy + x * r0 + ty`
     * @todo The documentation for this and ColorTransform could look better.
     */
    struct Matrix {
      fb_t sx = 1, sy = 1; // scale
      fb_t r0 = 0, r1 = 0; // skew
      sb_t tx = 0, ty = 0; // translate
    };
    
    /**
     * Represents the `HEADER` record.
     */
    struct Header {
    public:
      Compression::Enum compression; //!< The compression used for the file.
      version_t version; //!< The Flash version that this file targets.
      uint32_t fileSize; //!< The total file size (including header) after decompression.
      uint16_t frameRate, //!< The frame rate of this file in 1/256 FPS
               frameCount; //!< The count of frames for the main timeline
      Rect rect; //!< The dimensions of the file in `twips`
    };
  }
}

#endif /* defined(__jswf__Header__) */