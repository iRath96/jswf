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
      std::shared_ptr<jswf::io::GenericReader> reader; //!< A <tt>shared_ptr</tt> to the io::GenericReader we read from.
      
      /**
       * Create a Reader using a <tt>shared_ptr</tt> to a io::GenericReader.
       */
      Reader(std::shared_ptr<jswf::io::GenericReader> r) { reader = r; }
      
      /**
       * Reads a <tt>HEADER</tt> record.
       * @param [out] header A reference to the structure to read into.
       */
      void readHeader(Header &header);
      
      /**
       * Reads a <tt>TAG</tt> record.
       * @return A pointer to a newly allocated tag object (polymorphistic to Tag)
       */
      tags::Tag *readTag();
      
      /**
       * Reads a <tt>RECT</tt> record.
       * @param [out] rect A reference to the structure to read into.
       */
      void readRect(Rect &rect);
      
      /**
       * Reads a <tt>MATRIX</tt> record.
       * @param [out] matrix A reference to the structure to read into.
       */
      void readMatrix(Matrix &matrix);
      
      /**
       * Reads a <tt>CXFORM</tt> / <tt>CXFORMWITHALPHA</tt> record.
       * @param [out] colorTransform A reference to the structure to read into.
       * @param [in] withAlpha <tt>true</tt> if a <tt>CXFORMWITHALPHA</tt> should be read, <tt>false</tt> if a <tt>CXFORM</tt> should be read.
       */
      void readColorTransform(ColorTransform &colorTransform, bool withAlpha);
      
      // Colors
      
      /**
       * Reads a <tt>RGB</tt> record.
       * @param [out] rgba A reference to the structure to read into.
       */
      void readRGB(RGBA &rgba);
      
      /**
       * Reads a <tt>RGBA</tt> record.
       * @param [out] rgba A reference to the structure to read into.
       */
      void readRGBA(RGBA &rgba);
      
      /**
       * Reads a <tt>ARGB</tt> record.
       * @param [out] rgba A reference to the structure to read into.
       */
      void readARGB(RGBA &rgba);
    };
  }
}

#endif