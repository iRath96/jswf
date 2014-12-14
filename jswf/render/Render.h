//
//  Render.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__Render__
#define __jswf__Render__

#include <stdio.h>
#include <stdint.h>

#include "Shape.h"
#include "Frame.h"

namespace jswf {
  namespace flash {
    class Document;
  }
  
  namespace render {
    struct Context {
      uint16_t w, h;
      uint32_t *buffer;
      uint32_t *clip = NULL;
      
      flash::Matrix matrix;
      flash::Document *document;
    };
    
    void renderShape(flash::Shape &shape, Context &ctx);
    void renderFrame(flash::Frame &frame, Context &ctx);
  }
}

#endif