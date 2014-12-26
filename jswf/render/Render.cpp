//
//  Renderer.cpp
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "Render.h"
#include "Document.h"

#define FSAA 4
#define FSAA2 (FSAA * FSAA)

#include <math.h>

using namespace jswf;

void render::renderFrame(const flash::Frame &frame, const Context &context) {
  bool needsClipLayer = false;
  for(auto it = frame.displayList.begin(); it != frame.displayList.end(); ++it)
    if(it->second.doesClip) {
      needsClipLayer = true;
      break;
    }
  
  // TODO:2014-12-25:alex:Clipping is calculated incorrectly.
  uint32_t *clip = needsClipLayer ? new uint32_t[context.w * context.h] : NULL;
  uint16_t clipDepth = 0;
  
  for(auto it = frame.displayList.begin(); it != frame.displayList.end(); ++it) {
    const flash::DisplayObject &obj = it->second;
    if(context.document->dictionary.find(obj.characterId) == context.document->dictionary.end()) {
      printf("Cannot find character %d\n", obj.characterId);
      throw "Cannot find character.";
    }
    
    bool isClipped = clipDepth > it->first;
    
    Context c = context;
    if(isClipped) c.clip = clip;
    if(obj.setsColorTransform) c.colorTransform = obj.colorTransform;
    
    if(obj.doesClip) {
      memset(clip, 0, context.w * context.h);
      
      c.buffer = clip;
      clipDepth = obj.clipDepth;
    }
    
    // x' = x * sx + y * r1 + tx
    // y' = y * sy + x * r0 + ty
    
    // x'' = x' * sx' + y' * r1' + tx'
    // x'' = (x * sx + y * r1 + tx) * sx' + (y * sy + x * r0 + ty) * r1' + tx'
    
    // sx^ = sx * sx' + r0 * r1'
    // r1^ = r1 * sx' + sy * r1'
    // tx^ = tx * sx' + ty * r1' + tx'
    
    // TODO:2014-12-25:alex:Put this in a function?
    c.matrix.sx = obj.matrix.sx * context.matrix.sx + obj.matrix.r0 * context.matrix.r1;
    c.matrix.sy = obj.matrix.sy * context.matrix.sy + obj.matrix.r1 * context.matrix.r0;
    
    c.matrix.r0 = obj.matrix.r0 * context.matrix.sy + obj.matrix.sx * context.matrix.r0;
    c.matrix.r1 = obj.matrix.r1 * context.matrix.sx + obj.matrix.sy * context.matrix.r1;
    
    c.matrix.tx = obj.matrix.tx * context.matrix.sx + obj.matrix.ty * context.matrix.r1 + context.matrix.tx;
    c.matrix.ty = obj.matrix.ty * context.matrix.sy + obj.matrix.tx * context.matrix.r0 + context.matrix.ty;
    
    flash::DictionaryElement *character = context.document->dictionary[obj.characterId].get();
    if(dynamic_cast<flash::Shape *>(character)) { // Shape
      flash::Shape *shape = (flash::Shape *)character;
      //printf("Drawing shape: %d\n", shape->id);
      renderShape(*shape, c);
    } else if(dynamic_cast<flash::Sprite *>(character)) { // Sprite
      flash::Sprite *sprite = (flash::Sprite *)character;
      //printf("Drawing sprite: %d\n", sprite->id);
      renderFrame(sprite->frames[sprite->currentFrame], c);
      if(sprite->isPlaying) sprite->currentFrame = (sprite->currentFrame + 1) % sprite->frameCount;
    } else if(dynamic_cast<flash::Button *>(character)) { // Button
      flash::Button *button = (flash::Button *)character;
      //printf("Drawing button: %d\n", button->id);
      renderFrame(button->frames[flash::Button::UpState], c);
    } else {
      printf("Not sprite, not shape, not button\n");
      exit(1);
    }
  }
  
  delete[] clip;
}

inline void reverseTransformPoint(const flash::Matrix &m, sb_t &x, sb_t &y) {
  // mainly used for gradients.
  
  sb_t xOld = x, yOld = y;
  if(m.sx == 0 && m.sy == 0) {
    // x' = y * r1 + tx => y = (x' - tx) / r1
    // y' = x * r0 + ty => x = (y' - ty) / r0
    
    x = (yOld - m.ty) / m.r0;
    y = (xOld - m.tx) / m.r1;
    
    return;
  }
  
  if(m.sx == 0) {
    // x' = y * r1 + tx => y = (x' - tx) / r1
    // y' = y * sy + x * r0 + ty => x = (y' - y * sy - ty) / r0
    
    y = (xOld - m.tx) / m.r1;
    x = (yOld - y * m.sy - m.ty) / m.r0;
    
    return;
  }
  
  if(m.sy == 0) {
    // y' = x * r0 + ty => x = (y' - ty) / r0
    // x' = x * sx + y * r1 + tx => y = (x' - x * sx - tx) / r1
    
    x = (yOld - m.ty) / m.r0;
    y = (xOld - x * m.sx - m.tx) / m.r1;
    
    return;
  }
  
  // x' = x * sx + y * r1 + tx
  
  // x = (x' - (y' - ty) / sy * r1 - tx) / sx / (1 - r0 / sy * r1 / sx)
  // y = (y' - (x' - tx) / sx * r0 - ty) / sy / (1 - r1 / sx * r0 / sy)
  
  x = (xOld - (yOld - m.ty) / m.sy * m.r1 - m.tx) / m.sx / (1 - m.r0 / m.sy * m.r1 / m.sx);
  y = (yOld - (xOld - m.tx) / m.sx * m.r0 - m.ty) / m.sy / (1 - m.r1 / m.sx * m.r0 / m.sy);
  
  return;
}

inline void transformPoint(const flash::Matrix &matrix, sb_t &x, sb_t &y) {
  sb_t xOld = x, yOld = y;
  x = xOld * matrix.sx + yOld * matrix.r1 + matrix.tx;
  y = yOld * matrix.sy + xOld * matrix.r0 + matrix.ty;
}

inline uint8_t clamp(sb_t min, sb_t v, sb_t max) {
  return v < min ? min : (v > max ? max : v);
}

inline void transformColor(const flash::ColorTransform &ct, flash::RGBA &color) {
  // TODO:2014-12-24:alex:Use a macro.
  
  color.r = clamp(0, (color.r * ct.rM / 256) + ct.rA, 255);
  color.g = clamp(0, (color.g * ct.gM / 256) + ct.gA, 255);
  color.b = clamp(0, (color.b * ct.bM / 256) + ct.bA, 255);
  color.a = clamp(0, (color.a * ct.aM / 256) + ct.aA, 255);
}

inline void reverseSegment(flash::Segment &segment) {
  std::swap(segment.x0, segment.x1);
  std::swap(segment.y0, segment.y1);
  std::swap(segment.fillStyle0, segment.fillStyle1);
}

void calculateIntersections2(sb_t twipsY, std::vector<sb_t> &intersections, const std::vector<flash::Edge> &edges) {
  for(auto edge = edges.begin(); edge != edges.end(); ++edge) {
    if(edge->a.y == edge->b.y) continue; // No horizontal lines needed.
    
    if((twipsY < edge->a.y || twipsY >= edge->b.y) &&
       (twipsY < edge->b.y || twipsY >= edge->a.y))
      continue;
    
    sb_t twipsX = (twipsY - edge->a.y) * (edge->b.x - edge->a.x) / (edge->b.y - edge->a.y) + edge->a.x;
    intersections.push_back(twipsX);
  }
  
  std::sort(intersections.begin(), intersections.end());
}

void render::renderShape(const flash::Shape &shape, const Context &context) {
  //printf("shape => %d\n", shape.id);
  
  std::vector<flash::styles::FillStylePtr> fills;
  for(auto it = shape.polygons.begin(); it != shape.polygons.end(); ++it) fills.push_back(it->first);
  
  std::sort(fills.begin(), fills.end(), [](flash::styles::FillStylePtr a, flash::styles::FillStylePtr b) {
    return a->id < b->id;
  });
  
  for(auto it = fills.begin(); it != fills.end(); ++it) {
    flash::styles::FillStylePtr fill = *it;
    const std::vector<flash::Polygon> &polygons = shape.polygons.at(*it);
    
    std::vector<flash::Edge> edges;
    for(auto polygon = polygons.begin(); polygon != polygons.end(); ++polygon) {
      for(auto edge = polygon->edges.begin(); edge != polygon->edges.end(); ++edge) {
        flash::Edge e = *edge;
        
        transformPoint(context.matrix, e.a.x, e.a.y);
        transformPoint(context.matrix, e.b.x, e.b.y);
        
        edges.push_back(e);
      }
    }
    
    // TODO:2014-12-25:alex:Write some nice comments on how this moving x-boundary and subEdges work!
    
    int subHeight = context.h / 256;
    int lastSubY = -subHeight;
    
    std::vector<flash::Edge> subEdges;
    sb_t minX = 0, maxX = 0;
    
    uint16_t row[context.w];
    
    for(int y = 0; y < context.h; ++y) {
      if(y >= lastSubY + subHeight) {
        minX = context.w * 20;
        maxX = 0;
        
        sb_t twipsYLow = y * 20;
        sb_t twipsYHigh = (y + subHeight) * 20;
        
        subEdges.clear();
        for(auto edge = edges.begin(); edge != edges.end(); ++edge) {
          if(edge->a.y == edge->b.y) continue; // No horizontal lines needed.
          
          if((twipsYHigh < edge->a.y || twipsYLow >= edge->b.y) &&
             (twipsYHigh < edge->b.y || twipsYLow >= edge->a.y))
            continue;
          
          if(edge->a.x > maxX) maxX = edge->a.x;
          if(edge->a.x < minX) minX = edge->a.x;
          
          if(edge->b.x > maxX) maxX = edge->b.x;
          if(edge->b.x < minX) minX = edge->b.x;
          
          subEdges.push_back(*edge);
        }
        
        maxX /= 20; maxX += 1; if(maxX > context.w) maxX = context.w;
        minX /= 20; if(minX < 0) minX = 0;
      }
      
      if(minX >= maxX) continue; // Nothing to draw in this whole sub segment.
      
      //memset(row, 0, sizeof(row));
      memset(row+minX, 0, 2*(maxX-minX));
      
      for(int sy = 0; sy < FSAA; ++sy) {
        std::vector<sb_t> intersections;
        calculateIntersections2(y * 20 + sy * 20 / FSAA, intersections, subEdges);
        
        if(intersections.size() == 0) continue;
        
        // TODO:2014-12-25:alex:Rounding errors because I could multiply with FSAA when calculateIntersections already.
        
        uint32_t i = 0;
        while(intersections[i+1] < 0 && i+1 < intersections.size()) ++i;
        
        sb_t endX = context.w * FSAA;
        sb_t endTwipsX = endX * 20;
        
        sb_t fromX = intersections[i] * FSAA / 20;
        while(intersections[i] < endTwipsX && i+1 < intersections.size()) {
          sb_t toX = intersections[i+1] * FSAA / 20;
          
          if((i & 1) == 0) { // Even-odd winding rule
            if(toX >= endX) toX = endX;
            for(sb_t x = fromX; x < toX; ++x) row[x/FSAA] += 1;
          }
          
          fromX = toX;
          ++i;
        }
      }
      
      for(uint16_t x = minX; x < maxX; ++x) {
        uint8_t r = 0, g = 0, b = 0, a = 0;
        
        if(row[x] == 0) continue; // Transparent.
        
        flash::RGBA color;
        if(dynamic_cast<flash::styles::SolidFillStyle *>(fill.get())) {
          color = ((flash::styles::SolidFillStyle *)fill.get())->color;
        } else if(dynamic_cast<flash::styles::GradientFillStyle *>(fill.get())) {
          flash::styles::GradientFillStyle *gradient = (flash::styles::GradientFillStyle *)fill.get();
          
          sb_t gx = x * 20, gy = y * 20;
          reverseTransformPoint(context.matrix, gx, gy);
          reverseTransformPoint(gradient->matrix, gx, gy);
          uint8_t ratio;
          
          // Normalize gx to [ 0; 16384 )
          
          if(gradient->isRadial) gx = sqrt(gx * gx + gy * gy);
          else gx = (gx + 16384) / 2;
          
          ratio = clamp(0, gx / (16384 / 256), 255);
          
          std::vector<flash::styles::GradientStop>::iterator before, after = gradient->stops.begin();
          for(; after != gradient->stops.end(); ++after) if(after->ratio > ratio) break;
          
          before = after == gradient->stops.begin() ? after : after-1;
          uint8_t rdiv = after->ratio - before->ratio;
          if(rdiv == 0) color = before->color;
          else {
            uint8_t bfac = after->ratio - ratio, afac = ratio - before->ratio;
            color.r = before->color.r * bfac / rdiv + after->color.r * afac / rdiv;
            color.g = before->color.g * bfac / rdiv + after->color.g * afac / rdiv;
            color.b = before->color.b * bfac / rdiv + after->color.b * afac / rdiv;
            color.a = before->color.a * bfac / rdiv + after->color.a * afac / rdiv;
          }
          
          //printf("woot: %u\n", ratio);
        } else {
          // Don't know how to fill this, use solid red.
          color.r = 255;
          color.a = 255;
        }
        
        transformColor(context.colorTransform, color);
        
        r = color.r;
        g = color.g;
        b = color.b;
        a = color.a * row[x] / FSAA2;
        
        uint32_t bufferIndex = context.w * (context.h - y - 1) + x;
        
#define p8(a, i) *((uint8_t *)(&a)+i)
        if(context.clip)
          a = a * p8(context.clip[bufferIndex], 3) / 255.0;
        
        if(a == 0) continue; // transparent.
        
        uint32_t colorBefore = context.buffer[bufferIndex];
        uint8_t alphaBefore = p8(colorBefore, 3);
        uint8_t alphaNew = alphaBefore + (255 - alphaBefore) * a / 255;
        
        double blend = a / (double)alphaNew;
        
#define m8(i, a) p8(pixel, i) = p8(pixel, i) * (1 - blend) + a * blend;
        uint32_t pixel = colorBefore;
        
        p8(pixel, 3) = alphaNew;
        m8(0, r);
        m8(1, g);
        m8(2, b);
        
        context.buffer[bufferIndex] = pixel;
      }
    }
  }
}

/*
struct Intersection {
  flash::Segment *debug;
  double twipsX;
  sb_t xSum;
  flash::styles::FillStylePtr fill, fillLeft;
  bool operator<(const Intersection &rhs) const { return twipsX == rhs.twipsX ? xSum < rhs.xSum : (twipsX < rhs.twipsX); }
};

void calculateIntersections(sb_t twipsY, std::vector<Intersection> &intersections, std::vector<flash::Segment> &segments) {
  for(auto seg = segments.begin(); seg != segments.end(); ++seg) {
    if(seg->y0 == seg->y1) continue; // No horizontal lines needed.
    
    if(seg->y0 > seg->y1) { // normalize this
      sb_t tmp = seg->y0;
      seg->y0 = seg->y1;
      seg->y1 = tmp;
      
      tmp = seg->x0;
      seg->x0 = seg->x1;
      seg->x1 = tmp;
      
      flash::styles::FillStylePtr tmp2 = seg->fillStyle0;
      seg->fillStyle0 = seg->fillStyle1;
      seg->fillStyle1 = tmp2;
    }
    
    if(twipsY < seg->y0 || twipsY >= seg->y1) continue;
    
    Intersection i;
    i.debug = &*seg;
    i.twipsX = (twipsY - seg->y0) * (seg->x1 - seg->x0) / (double)(seg->y1 - seg->y0) + seg->x0;
    i.xSum = seg->x0 + seg->x1;
    i.fill = seg->fillStyle0;
    i.fillLeft = seg->fillStyle1;
    
    intersections.push_back(i);
  }
  
  std::sort(intersections.begin(), intersections.end());
  
  if(intersections.size() > 0 && intersections[0].twipsX > 0) {
    Intersection transparent;
    transparent.twipsX = 0;
    transparent.xSum = 0;
    transparent.fill = NULL;
    intersections.insert(intersections.begin(), transparent);
  }
}

#define SUBBEZIER 8
void render::renderShape(flash::Shape &shape, Context &context) {
  std::map<uint32_t, std::vector<flash::Segment>> segments;
  for(auto seg = shape.segments.begin(); seg != shape.segments.end(); ++seg) {
    if(seg->isCurved) {
      sb_t x = seg->x0, y = seg->y0;
      for(int i = 1; i <= SUBBEZIER; ++i) {
        double t = i / (double)SUBBEZIER;
        
#define MORPH(a, b, t) (a + (b - a) * t)
        sb_t xa = MORPH(seg->x0, seg->cx, t), ya = MORPH(seg->y0, seg->cy, t);
        sb_t xb = MORPH(seg->cx, seg->x1, t), yb = MORPH(seg->cy, seg->y1, t);
        sb_t xx = MORPH(xa, xb, t), yy = MORPH(ya, yb, t);
#undef MORPH
        
        flash::Segment s = *seg;
        s.isCurved = false;
        s.x0 = x ; s.y0 = y ;
        s.x1 = xx; s.y1 = yy;
        
        transformPoint(context.matrix, s.x0, s.y0);
        transformPoint(context.matrix, s.x1, s.y1);
        
        if(s.fillStyle0 != NULL) segments[s.fillStyle0->id].push_back(s);
        reverseSegment(s);
        if(s.fillStyle0 != NULL) segments[s.fillStyle0->id].push_back(s);
        
        x = xx; y = yy;
      }
    } else {
      flash::Segment s = *seg;
      transformPoint(context.matrix, s.x0, s.y0);
      transformPoint(context.matrix, s.x1, s.y1);
      
      if(s.fillStyle0 != NULL) segments[s.fillStyle0->id].push_back(s);
      reverseSegment(s);
      if(s.fillStyle0 != NULL) segments[s.fillStyle0->id].push_back(s);
    }
  }
  
  for(auto it = segments.begin(); it != segments.end(); ++it) {
    // For every FillStyle
    
    for(int y = 0; y < context.h; ++y) {
      uint64_t row[context.w];
      memset(row, 0, sizeof(row));
      
      for(int sy = 0; sy < FSAA; ++sy) {
        std::vector<Intersection> intersections;
        calculateIntersections(y * 20 + sy * 20 / FSAA, intersections, it->second);
        
        if(intersections.size() == 0) continue;
        
        uint32_t i = 0;
        sb_t maxTwipsX = context.w * 20;
        while(intersections[i].twipsX < 0 && i+1 < intersections.size()) ++i;
        
        int fromX = intersections[i].twipsX * FSAA / 20;
        while(intersections[i].twipsX < maxTwipsX && i+1 < intersections.size()) {
          int toX = intersections[i+1].twipsX * FSAA / 20;
          uint64_t fill = 0;
          if(dynamic_cast<flash::styles::SolidFillStyle *>(intersections[i].fill.get())) {
            flash::RGBA *c = &((flash::styles::SolidFillStyle *)intersections[i].fill.get())->color;
            fill = ((uint64_t)c->r << 48) | ((uint64_t)c->g << 32) | ((uint64_t)c->b << 16) | ((uint64_t)c->a << 0);
          }
          
          for(int x = fromX; x < toX; ++x) row[x/FSAA] += fill;
          fromX = toX;
          
          ++i;
        }
      }
      
      for(int x = 0; x < context.w; ++x) {
        uint16_t *colorSum = (uint16_t *)&row[x];
        uint8_t r, g, b, a;
        
        r = colorSum[3];
        g = colorSum[2];
        b = colorSum[1];
        a = colorSum[0];
        
        uint32_t bufferIndex = context.w * (context.h - y - 1) + x;
        
        if(a == 0) continue; // transparent.
        
#define p8(a, i) *((uint8_t *)(&a)+i)
        if(context.clip) a = a * p8(context.clip[bufferIndex], 3) / 255.0;
        
        uint32_t colorBefore = context.buffer[bufferIndex];
        uint8_t alphaBefore = p8(colorBefore, 3);
        uint8_t alphaNew = alphaBefore + (255 - alphaBefore) * a / FSAA2 / 255;
        
        double blend = a / FSAA2 / (double)alphaNew;
        
#define m8(i, a) p8(pixel, i) = p8(pixel, i) * (1 - blend) + a / FSAA2 * blend;
        uint32_t pixel = colorBefore;
        
        p8(pixel, 3) = alphaNew;
        m8(0, r);
        m8(1, g);
        m8(2, b);
        
        context.buffer[bufferIndex] = pixel;
      }
    }
  }
}*/

/*
void render::renderShape(flash::Shape &shape, Context &context) {
  // TODO:2014-12-15:alex:Check context.clip *before* calculating a pixel.
  
  printf("tx: %lld\n", context.matrix.tx);
  printf("r1: %lf\n", context.matrix.r1);
  
#define MORPH(a, b, t) (a + (b - a) * t)
  std::vector<flash::Segment> segments;
  for(auto seg = shape.segments.begin(); seg != shape.segments.end(); ++seg) {
    if(seg->isCurved) {
      sb_t x = seg->x0, y = seg->y0;
      for(int i = 1; i <= SUBBEZIER; ++i) {
        double t = i / (double)SUBBEZIER;
        
        sb_t xa = MORPH(seg->x0, seg->cx, t), ya = MORPH(seg->y0, seg->cy, t);
        sb_t xb = MORPH(seg->cx, seg->x1, t), yb = MORPH(seg->cy, seg->y1, t);
        sb_t xx = MORPH(xa, xb, t), yy = MORPH(ya, yb, t);
        
        flash::Segment s = *seg;
        s.isCurved = false;
        s.x0 = x ; s.y0 = y ;
        s.x1 = xx; s.y1 = yy;
        
        transformPoint(context.matrix, s.x0, s.y0);
        transformPoint(context.matrix, s.x1, s.y1);
        
        segments.push_back(s);
        
        x = xx; y = yy;
      }
    } else {
      flash::Segment s = *seg;
      transformPoint(context.matrix, s.x0, s.y0);
      transformPoint(context.matrix, s.x1, s.y1);
      segments.push_back(s);
    }
  }
  
  struct Intersection {
    flash::Segment *debug;
    double twipsX;
    sb_t xSum;
    flash::styles::FillStylePtr fill, fillLeft;
    bool operator<(const Intersection &rhs) const { return twipsX == rhs.twipsX ? xSum < rhs.xSum : (twipsX < rhs.twipsX); }
  };
  
  for(int y = 0; y < context.h; ++y) {
    std::vector<Intersection> sublines[FSAA];
    uint32_t indices[FSAA];
    
    for(int sy = 0; sy < FSAA; ++sy) {
      indices[sy] = 0;
      
      sb_t twipsY = y * 20 + sy * (20 / FSAA);
      for(auto seg = segments.begin(); seg != segments.end(); ++seg) {
        if(seg->y0 == seg->y1) continue; // No horizontal lines needed.
        
        if(seg->y0 > seg->y1) { // normalize this
          sb_t tmp = seg->y0;
          seg->y0 = seg->y1;
          seg->y1 = tmp;
          
          tmp = seg->x0;
          seg->x0 = seg->x1;
          seg->x1 = tmp;
          
          flash::styles::FillStylePtr tmp2 = seg->fillStyle0;
          seg->fillStyle0 = seg->fillStyle1;
          seg->fillStyle1 = tmp2;
        }
        
        if(twipsY < seg->y0 || twipsY >= seg->y1) continue;
        
        Intersection i;
        i.debug = &*seg;
        i.twipsX = (twipsY - seg->y0) * (seg->x1 - seg->x0) / (double)(seg->y1 - seg->y0) + seg->x0;
        i.xSum = seg->x0 + seg->x1;
        i.fill = seg->fillStyle0;
        i.fillLeft = seg->fillStyle1;
        
        sublines[sy].push_back(i);
      }
      
      std::sort(sublines[sy].begin(), sublines[sy].end());
      
      if(sublines[sy].size() > 0 && sublines[sy][0].twipsX > 0) {
        Intersection transparent;
        transparent.twipsX = 0;
        transparent.xSum = 0;
        transparent.fill = NULL;
        sublines[sy].insert(sublines[sy].begin(), transparent);
      }
      
      // Note: Gradients seem to be calculated on a per-pixel basis.
      
      flash::styles::FillStylePtr left(NULL);
      for(auto it = sublines[sy].begin(); it != sublines[sy].end(); ++it) {
        if(it->fillLeft != left) {
          long i = it - sublines[sy].begin();
          printf("error at %ld\n", i);
          
          long j = i+1;
          for(; j < sublines[sy].size(); ++j)
            if(sublines[sy][j].fillLeft == left)
              break;
          
          if(j < sublines[sy].size())
            std::swap(sublines[sy][i], sublines[sy][j]);
          else
            printf("COULD NOT BE FIXED!\n");
        }
        left = it->fill;
      }
    }
    
    for(int x = 0; x < context.w; ++x) {
      // Calculate each pixel
      
      uint32_t bufferIndex = context.w * (context.h - y - 1) + x;
      
#define p8(a, i) *((uint8_t *)(&a)+i)
      if(context.clip && p8(context.clip[bufferIndex], 3) == 0) continue; // mask is transparent, skip the pixel.
      
      uint16_t r = 0, g = 0, b = 0, a = 0;
      
      for(int sy = 0; sy < FSAA; ++sy) {
        if(sublines[sy].size() == 0) continue;
        for(int sx = 0; sx < FSAA; ++sx) {
          sb_t twipsX = x * 20 + sx * (20 / FSAA);
          while(sublines[sy].size() > indices[sy]+1 && sublines[sy][indices[sy]+1].twipsX < twipsX)
            ++indices[sy];
          
          flash::styles::FillStyle *fill = sublines[sy][indices[sy]].fill.get();
          
          if(fill == NULL) continue;
          if(dynamic_cast<flash::styles::SolidFillStyle *>(fill)) { // TODO:2014-12-16:alex:typeid?
            flash::styles::SolidFillStyle *sfill = (flash::styles::SolidFillStyle *)fill;
            r += sfill->color.r;
            g += sfill->color.g;
            b += sfill->color.b;
            a += sfill->color.a;
          }
        }
      }
      
      if(a == 0) continue; // transparent.
      
      if(context.clip)
        a = a * p8(context.clip[bufferIndex], 3) / 255.0;
      
      uint32_t colorBefore = context.buffer[bufferIndex];
      uint8_t alphaBefore = p8(colorBefore, 3);
      uint8_t alphaNew = alphaBefore + (255 - alphaBefore) * a / FSAA2 / 255;
      
      double blend = a / FSAA2 / (double)alphaNew;
      
#define m8(i, a) p8(pixel, i) = p8(pixel, i) * (1 - blend) + a / FSAA2 * blend;
      uint32_t pixel = colorBefore;
      
      p8(pixel, 3) = alphaNew;
      m8(0, r);
      m8(1, g);
      m8(2, b);
      
      context.buffer[bufferIndex] = pixel;
    }
  }
}*/