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
#include <map>
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
    
    struct Point {
      sb_t x, y;
      bool operator ==(const Point &rhs) { return x == rhs.x && y == rhs.y; };
      bool operator !=(const Point &rhs) { return x != rhs.x || y != rhs.y; };
    };
    
    struct Edge {
      Point a, b;
    };
    
    struct Polygon {
      std::vector<Edge> edges;
    };
    
    class Shape : public DictionaryElement {
      sb_t x = 0, y = 0; // current drawing position
      styles::LineStylePtr lineStyle;
      styles::FillStylePtr fillStyle0, fillStyle1;
    public:
      std::vector<Segment> segments;
      
      std::map<styles::FillStylePtr, std::vector<Polygon>> polygons;
      
      void polygonize() {
        std::vector<Segment> flatSegments;
        
#define SUBBEZIER 16
        for(auto seg = segments.begin(); seg != segments.end(); ++seg) {
          if(seg->isCurved) {
            sb_t x = seg->x0, y = seg->y0;
            for(int i = 1; i <= SUBBEZIER; ++i) {
              double t = i / (double)SUBBEZIER;
              
#define MORPH(a, b, t) (a + (b - a) * t)
              sb_t xa = MORPH(seg->x0, seg->cx, t), ya = MORPH(seg->y0, seg->cy, t);
              sb_t xb = MORPH(seg->cx, seg->x1, t), yb = MORPH(seg->cy, seg->y1, t);
              sb_t xx = MORPH(xa, xb, t), yy = MORPH(ya, yb, t);
#undef MORPH
              
              Segment s = *seg;
              s.isCurved = false;
              s.x0 = x ; s.y0 = y ;
              s.x1 = xx; s.y1 = yy;
              
              flatSegments.push_back(s);
              
              x = xx; y = yy;
            }
          } else
            flatSegments.push_back(*seg);
        }
        
        // Magic
        
        std::map<styles::FillStylePtr, std::vector<Edge>> rawEdges;
        for(auto it = flatSegments.begin(); it != flatSegments.end(); ++it) {
          if(it->fillStyle0 != NULL) {
            Edge e = { .a = { .x = it->x0, .y = it->y0 }, .b = { .x = it->x1, .y = it->y1 } };
            if(e.a != e.b) rawEdges[it->fillStyle0].push_back(e);
          }
          
          if(it->fillStyle1 != NULL) { // but reverse.
            Edge e = { .b = { .x = it->x0, .y = it->y0 }, .a = { .x = it->x1, .y = it->y1 } };
            if(e.a != e.b) rawEdges[it->fillStyle1].push_back(e);
          }
        }
        
        // Weeeee
        
        polygons.clear();
        for(auto it = rawEdges.begin(); it != rawEdges.end(); ++it) {
          std::vector<Edge> &pEdges = it->second;
          
          Point firstPoint = pEdges[0].a;
          Point searchPoint = pEdges[0].a;
          
          Polygon p;
          
          while(!pEdges.empty()) {
            bool foundNextEdge = false;
            Edge nextEdge;
            
            for(auto edge = pEdges.begin(); edge != pEdges.end(); ++edge)
              if(edge->a == searchPoint) {
                foundNextEdge = true;
                nextEdge = *edge;
                pEdges.erase(edge);
                break;
              }
            
            if(!foundNextEdge) {
              // Polygon incomplete, let's go to the next one
              goto nextPolygon;
            } else {
              p.edges.push_back(nextEdge);
              
              if(nextEdge.b == firstPoint) {
                // Polygon complete
                polygons[it->first].push_back(p);
                goto nextPolygon;
              } else {
                // Polygon in progress
                searchPoint = nextEdge.b;
                continue;
              }
            }
            
          nextPolygon:
            p.edges.clear();
            firstPoint = searchPoint = pEdges[0].a;
          }
        }
      }
      
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