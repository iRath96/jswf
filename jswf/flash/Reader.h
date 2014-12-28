//
//  Reader.h
//  jswf/flash
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf_flash__Reader__
#define __jswf_flash__Reader__

#include <stdio.h>
#include "GenericReader.h"

namespace jswf {
  namespace flash {
    namespace tags { class Tag; }
    
    class Header;
    class Shape;
    
    struct Rect;
    struct RGBA;
    struct Matrix;
    struct ColorTransform;
    
    /**
     * Servers as reader for complex structures like tags, colors, matrices, etc.
     */
    class Reader {
    public:
      std::shared_ptr<jswf::io::GenericReader> reader; //!< A `shared_ptr` to the \ref io::GenericReader we read from.
      
      /**
       * Create a \ref Reader using a `shared_ptr` to a \ref io::GenericReader.
       */
      Reader(std::shared_ptr<jswf::io::GenericReader> r) { reader = r; }
      
      /**
       * Reads a `HEADER` record.
       * @param [out] header A reference to the structure to read into.
       */
      void readHeader(Header &header);
      
      /**
       * Reads a `TAG` record.
       * @return A pointer to a newly allocated tag object (polymorphistic to \ref Tag)
       */
      tags::Tag *readTag();
      
      /**
       * Reads a `RECT` record.
       * @param [out] rect A reference to the structure to read into.
       */
      void readRect(Rect &rect);
      
      /**
       * Reads a `MATRIX` record.
       * @param [out] matrix A reference to the structure to read into.
       */
      void readMatrix(Matrix &matrix);
      
      /**
       * Reads a `CXFORM` / `CXFORMWITHALPHA` record.
       * @param [out] colorTransform A reference to the structure to read into.
       * @param [in] withAlpha `true` if a `CXFORMWITHALPHA` should be read, `false` if a `CXFORM` should be read.
       */
      void readColorTransform(ColorTransform &colorTransform, bool withAlpha);
      
      // Colors
      
      /**
       * Reads a `RGB` record.
       * @param [out] rgba A reference to the structure to read into.
       */
      void readRGB(RGBA &rgba);
      
      /**
       * Reads a `RGBA` record.
       * @param [out] rgba A reference to the structure to read into.
       */
      void readRGBA(RGBA &rgba);
      
      /**
       * Reads a `ARGB` record.
       * @param [out] rgba A reference to the structure to read into.
       */
      void readARGB(RGBA &rgba);
    };
  }
}

#endif