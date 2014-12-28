//
//  DefineShape4Tag.h
//  jswf
//
//  Created by Alexander Rath on 15.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineShape4Tag_h
#define jswf_DefineShape4Tag_h

#include "DefineShapeTag.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Extends \ref DefineShape3Tag with support for `LINESTYLE2`.
       */
      class DefineShape4Tag : public DefineShape3Tag {
        virtual styles::LineStyle *readLineStyle() {
          styles::LineStyle *style = new styles::LineStyle();
          style->width = reader->readU16();
          
          uint8_t startCapStyle = reader->readUB(2);
          uint8_t joinStyle = reader->readUB(2);
          bool hasFillFlag  = reader->readUB(1);
          bool noHScaleFlag = reader->readUB(1);
          bool noVScaleFlag = reader->readUB(1);
          bool pixelHinting = reader->readUB(1);
          
          uint8_t reserved = reader->readUB(5);
          assert(reserved == 0);
          
          bool noClose = reader->readUB(1);
          uint8_t endCapStyle = reader->readUB(2);
          
          if(joinStyle == 2) {
            uint16_t miterLimitFactor = reader->readU16();
          }
          
          if(hasFillFlag) {
            styles::FillStyle *fill = readFillStyle();
          } else {
            readColor(style->color);
          }
          
          return style;
        }
        
        virtual void readBetween() {
          reader->align(1); // TODO:2014-12-14:alex:For all RECTs?
          flashReader.readRect(shape->edgeBounds);
          
          reader->align(1);
          uint8_t reserved = reader->readUB(5);
          assert(reserved == 0);
          
          shape->usesFillWindingRule   = reader->readUB(1);
          shape->usesNonScalingStrokes = reader->readUB(1);
          shape->usesScalingStrokes    = reader->readUB(1);
        }
      public:
        DefineShape4Tag(tag_type_t t, std::string &p) : DefineShape3Tag(t, p, false) { read(); }
      };
    }
  }
}

#endif