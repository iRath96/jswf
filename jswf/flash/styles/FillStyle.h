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

#include <vector>

namespace jswf {
  namespace flash {
    namespace styles {
      class FillStyle;
      typedef std::shared_ptr<FillStyle> FillStylePtr;
      
      class FillStyle {
      public:
        uint32_t id;
        
        virtual ~FillStyle() {};
      };
      
      class SolidFillStyle : public FillStyle {
      public:
        RGBA color;
      };
      
      struct GradientStop {
        uint8_t ratio;
        RGBA color;
      };
      
      class GradientFillStyle : public FillStyle {
      public:
        uint8_t spreadMode, interpolationMode;
        Matrix matrix;
        
        bool isRadial = false;
        std::vector<GradientStop> stops;
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