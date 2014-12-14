//
//  Renderer.cpp
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "Render.h"
#include "Document.h"

using namespace jswf;

void render::renderFrame(flash::Frame &frame, Context &context) {
  uint32_t clip[context.w * context.h];
  uint16_t clipDepth = 0;
  
  //for(uint64_t i = 0, j = context.w * context.h; i < j; ++i)
  //  ((uint32_t *)screen->pixels)[i] = 0xffffffff; // Background color
  
  for(auto it = frame.displayList.begin(); it != frame.displayList.end(); ++it) {
    flash::DisplayObject &obj = it->second;
    if(context.document->dictionary.find(obj.characterId) == context.document->dictionary.end()) {
      printf("Cannot find character %d\n", obj.characterId);
      throw "Cannot find character.";
    }
    
    bool isClipped = clipDepth > it->first;
    
    Context c = context;
    c.clip = isClipped ? clip : NULL;
    
    if(obj.doesClip) {
      memset(clip, 0, sizeof(clip));
      
      c.buffer = clip;
      clipDepth = obj.clipDepth;
    }
    
    c.matrix.sx *= obj.matrix.sx;
    c.matrix.sy *= obj.matrix.sy;
    c.matrix.tx = c.matrix.tx * obj.matrix.sx + obj.matrix.tx;
    c.matrix.ty = c.matrix.ty * obj.matrix.sy + obj.matrix.ty;
    
    flash::DictionaryElement *character = context.document->dictionary[obj.characterId].get();
    if(dynamic_cast<flash::Shape *>(character)) { // Shape
      flash::Shape *shape = (flash::Shape *)character;
      printf("Drawing shape: %d\n", shape->id);
      renderShape(*shape, c);
    } else if(dynamic_cast<flash::Sprite *>(character)) { // Sprite
      flash::Sprite *sprite = (flash::Sprite *)character;
      printf("Drawing sprite: %d\n", sprite->id);
      renderFrame(sprite->frames[0], c);
    } else {
      printf("Not sprite and not shape\n");
      exit(1);
    }
  }
}

void render::renderShape(flash::Shape &shape, Context &context) {
  printf("sx: %lf\n", context.matrix.sx);
  
#define SCALE 8
#define SUBBEZIER 16
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
        segments.push_back(s);
        
        x = xx; y = yy;
      }
    } else segments.push_back(*seg);
  }
  
#define FSAA 4
#define FSAA2 (FSAA * FSAA)
  struct Intersection {
    double twipsX;
    flash::styles::FillStylePtr fill;
    bool operator<(const Intersection &rhs) const { return twipsX < rhs.twipsX; }
  };
  
  for(int y = 0; y < context.h; ++y) {
    std::vector<Intersection> sublines[FSAA];
    uint32_t indices[FSAA];
    
    for(int sy = 0; sy < FSAA; ++sy) {
      indices[sy] = 0;
      
      int twipsY = (y * 20 + sy * (20 / FSAA)) / SCALE / context.matrix.sy + context.matrix.ty;
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
        i.twipsX = (twipsY - seg->y0) * (seg->x1 - seg->x0) / (seg->y1 - seg->y0) + seg->x0;
        i.fill = seg->fillStyle0;
        
        sublines[sy].push_back(i);
      }
      
      std::sort(sublines[sy].begin(), sublines[sy].end());
      
      if(sublines[sy].size() > 0 && sublines[sy][0].twipsX > 0) {
        Intersection transparent;
        transparent.twipsX = 0;
        transparent.fill = flash::styles::FillStylePtr(NULL);
        sublines[sy].insert(sublines[sy].begin(), transparent);
      }
    }
    
    for(int x = 0; x < context.w; ++x) {
      // Calculate each pixel
      
      uint16_t r = 0, g = 0, b = 0, a = 0;
      
      for(int sy = 0; sy < FSAA; ++sy) {
        if(sublines[sy].size() == 0) continue;
        for(int sx = 0; sx < FSAA; ++sx) {
          sb_t twipsX = (x * 20 + sx * (20 / FSAA)) / SCALE / context.matrix.sx + context.matrix.tx;
          while(sublines[sy].size() > indices[sy]+1 && sublines[sy][indices[sy]+1].twipsX < twipsX)
            ++indices[sy];
          
          flash::styles::FillStyle *fill = sublines[sy][indices[sy]].fill.get();
          
          if(fill == NULL) continue;
          if(dynamic_cast<flash::styles::SolidFillStyle *>(fill)) {
            flash::styles::SolidFillStyle *sfill = (flash::styles::SolidFillStyle *)fill;
            r += sfill->color.r;
            g += sfill->color.g;
            b += sfill->color.b;
            a += sfill->color.a;
          }
        }
      }
      
      uint32_t bufferIndex = context.w * y + x;
#define p8(a, i) *((uint8_t *)(&a)+i)
      if(context.clip)
        a = a * p8(context.clip[bufferIndex], 0) / 255.0;
      
      uint32_t colorBefore = context.buffer[bufferIndex];
      uint8_t alphaBefore = p8(colorBefore, 0);
      uint8_t alphaNew = alphaBefore + (255 - alphaBefore) * a / FSAA2 / 255;
      
      double blend = a / FSAA2 / 255.0;
      
#define m8(i, a) p8(pixel, i) = p8(pixel, i) * (1 - blend) + a / FSAA2 * blend;
      uint32_t pixel = colorBefore;
      
      p8(pixel, 0) = alphaNew;
      m8(1, r);
      m8(2, g);
      m8(3, b);
      
      context.buffer[bufferIndex] = pixel;
    }
  }
}