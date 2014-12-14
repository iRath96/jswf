//
//  FillStyle.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_FillStyle_h
#define jswf_FillStyle_h

#include "Header.h"

namespace jswf {
  namespace flash {
    namespace styles {
      class FillStyle;
      typedef std::shared_ptr<FillStyle> FillStylePtr;
      
      class FillStyle {
      public:
        virtual ~FillStyle() {};
      };
      
      class SolidFillStyle : public FillStyle {
      public:
        RGBA color;
      };
      
      class GradientFillStyle : public FillStyle {
      public:
        Matrix matrix;
        Gradient gradient;
      };
      
      class FocalGradientFillStyle : public GradientFillStyle {
      public:
        fixed8_t focalPoint;
      };
      
      class BitmapFillStyle : public FillStyle {
      public:
        uint16_t bitmapId;
        Matrix matrix;
        bool repeat, smooth;
      };
    }
  }
}

#endif