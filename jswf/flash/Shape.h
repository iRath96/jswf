//
//  Shape.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__Shape__
#define __jswf__Shape__

#include <stdio.h>
#include <stdint.h>
#include "Header.h"

#include "Styles.h"
#include "DictionaryElement.h"

#include <vector>
#include <sstream>

namespace jswf {
  namespace flash {
    struct Segment {
      sb_t x0, y0;
      sb_t x1, y1;
      
      bool isCurved;
      sb_t cx, cy;
      
      styles::LineStylePtr lineStyle;
      styles::FillStylePtr fillStyle0, fillStyle1;
    };
    
    class Shape : public DictionaryElement {
      sb_t x = 0, y = 0; // current drawing position
      styles::LineStylePtr lineStyle;
      styles::FillStylePtr fillStyle0, fillStyle1;
    public:
      std::vector<Segment> segments;
      
      Rect bounds, edgeBounds;
      bool usesFillWindingRule   = false,
           usesNonScalingStrokes = false,
           usesScalingStrokes    = true;
      
      void moveTo(sb_t x, sb_t y) {
        this->x = x;
        this->y = y;
      }
      
      void lineTo(sb_t dx, sb_t dy) {
        segments.push_back((Segment){
          .x0 = x,
          .y0 = y,
          
          .x1 = x+dx,
          .y1 = y+dy,
          
          .isCurved = false,
          
          .lineStyle = lineStyle,
          .fillStyle0 = fillStyle0,
          .fillStyle1 = fillStyle1
        });
        
        x += dx;
        y += dy;
      }
      
      void qlineTo(sb_t cx, sb_t cy, sb_t ax, sb_t ay) {
        segments.push_back((Segment){
          .x0 = x,
          .y0 = y,
          
          .cx = x+cx,
          .cy = y+cy,
          
          .x1 = x+ax+cx,
          .y1 = y+ay+cy,
          
          .isCurved = true,
          
          .lineStyle = lineStyle,
          .fillStyle0 = fillStyle0,
          .fillStyle1 = fillStyle1
        });
        
        x += ax + cx;
        y += ay + cy;
      }
      
      void setLineStyle(styles::LineStylePtr &style) { lineStyle = style; }
      void setFillStyle0(styles::FillStylePtr &style) { fillStyle0 = style; }
      void setFillStyle1(styles::FillStylePtr &style) { fillStyle1 = style; }
      
      void appendFillStyle(std::stringstream &buffer, styles::FillStylePtr &ptr) {
        if(ptr.get() == NULL)
          buffer << ",null";
        else {
          if(dynamic_cast<styles::SolidFillStyle *>(ptr.get())) {
            buffer << ",[\"solid\",[";
            RGBA *color = &((styles::SolidFillStyle *)(ptr.get()))->color;
            buffer << (int)color->r << "," << (int)color->g << "," << (int)color->b << "," << (int)color->a;
            buffer << "]]";
          } else {
            printf("This kind of FillStyle is not yet supported.\n");
            throw "This kind of FillStyle is not yet supported.";
          }
        }
      }
      
      std::string toJS() {
        std::stringstream buffer;
        buffer << "[";
        
        for(auto seg = segments.begin(); seg != segments.end(); ++seg) {
          if(seg != segments.begin()) buffer << ",";
          
          buffer << "[";
          
          buffer << "[" << seg->x0 << "," << seg->y0 << "],";
          buffer << "[" << seg->x1 << "," << seg->y1 << "],";
          
          if(seg->isCurved)
            buffer << "[" << seg->cx << "," << seg->cy << "]";
          else
            buffer << "null";
          
          if(seg->lineStyle.get() == NULL)
            buffer << ",null";
          else {
            printf("LineStyles not supported yet.\n");
            throw "LineStyles not supported yet.";
          }
          
          appendFillStyle(buffer, seg->fillStyle0);
          appendFillStyle(buffer, seg->fillStyle1);
          
          buffer << "]";
        }
        
        buffer << "]";
        return buffer.str();
      }
    };
  }
}

#endif /* defined(__jswf__Shape__) */