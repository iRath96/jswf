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
#include "Structs.h"

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