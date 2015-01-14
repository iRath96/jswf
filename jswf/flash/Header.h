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
    /**
     * Provides Compression::Enum
     */
    struct Compression {
      /**
       * Describes the kinds of compressions used by SWF documents.
       */
      enum Enum {
        Uncompressed,
        ZLib
      };
    };
    
    /**
     * Represents <tt>RECT</tt> records.
     */
    struct Rect {
      sb_t x0, //!< The low x-coordinate.
           y0; //!< The low y-coordinate.
      sb_t x1, //!< The high x-coordinate.
           y1; //!< The high y-coordinate.
    };
    
    /**
     * Represents <tt>RGB</tt>, <tt>RGBA</tt> and <tt>ARGB</tt> color records.
     */
    struct RGBA {
      uint8_t r, //!< Red channel.
              g, //!< Green channel.
              b, //!< Blue channel.
              a; //!< Alpha channel.
    };
    
    /**
     * Represents <tt>CXFORM</tt> and <tt>CXFORMWITHALPHA</tt> records.
     * For each channel \f$v\f$ from \f${r,g,b,a}\f$, the transformed value \f$v'\f$ can be computed as follows:\n
     * \f$v' = max(0, min((v \cdot vM \cdot \frac{1}{256}) + vA, 255))\f$
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
     * Represents <tt>MATRIX</tt> records.
     * \f$x' = x \cdot sx + y \cdot r1 + tx\f$\n
     * \f$y' = y \cdot sy + x \cdot r0 + ty\f$
     * @todo These structures are in the wrong file.
     */
    struct Matrix {
      fb_t sx = 1, sy = 1; // scale
      fb_t r0 = 0, r1 = 0; // skew
      sb_t tx = 0, ty = 0; // translate
    };
    
    /**
     * Represents the <tt>HEADER</tt> record.
     */
    struct Header {
    public:
      Compression::Enum compression; //!< The compression used for the file.
      version_t version; //!< The Flash version that this file targets.
      uint32_t fileSize; //!< The total file size (including header) after decompression.
      uint16_t frameRate, //!< The frame rate of this file in 1/256 FPS
               frameCount; //!< The count of frames for the main timeline
      Rect rect; //!< The dimensions of the file in <tt>twips</tt>
    };
  }
}

#endif /* defined(__jswf__Header__) */