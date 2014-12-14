//
//  DefineShapeTag.cpp
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include <stdio.h>
#include "DefineShapeTag.h"

using namespace jswf::flash::styles;
using namespace jswf::flash::tags;

FillStyle *DefineShapeTag::readFillStyle() {
  uint8_t type = reader->readU8();
  switch(type) { // TODO:2014-12-14:alex:I need to create an enum for this.
    case 0x00: { // Solid Fill
      SolidFillStyle *style = new SolidFillStyle();
      readColor(style->color);
      return (FillStyle *)style;
    }; break;
    case 0x10: // Linear Gradient Fill
    case 0x12: { // Radial Gradient Fill
      GradientFillStyle *style = new GradientFillStyle();
      flashReader.readMatrix(style->matrix);
      // TODO!
      return (FillStyle *)style;
    }; break;
    case 0x13: {
      FocalGradientFillStyle *style = new FocalGradientFillStyle();
      flashReader.readMatrix(style->matrix);
      // TODO!
      return (FillStyle *)style;
    }; break;
    case 0x40: // repeating bitmap fill
    case 0x41: // clipped bitmap fill
    case 0x42: // non-smoothed repeating bitmap fill
    case 0x43: { // non-smoothed clipped bitmap fill
      BitmapFillStyle *style = new BitmapFillStyle();
      style->bitmapId = reader->readU16();
      style->repeat = !(type & 1);
      style->smooth = !(type & 2);
      flashReader.readMatrix(style->matrix);
      return (FillStyle *)style;
    }; break;
    default: {
      printf("Unknown fill style: %x\n", type);
      throw "Unknown fill style.";
    };
  }
}

void DefineShapeTag::readFillStyleArray() {
  fillStyles.clear();
  fillStyles.push_back(styles::FillStylePtr(NULL));
  
  uint8_t count = reader->readU8();
  if(count == 0xff) throw "FillStyleArray overflow."; // We need one spare id for NoFillStyle
  
  for(uint16_t id = 1; id <= count; ++id) {
    styles::FillStyle *style = readFillStyle();
    fillStyles.push_back(styles::FillStylePtr(style));
  }
}

LineStyle *DefineShapeTag::readLineStyle() {
  LineStyle *style = new LineStyle();
  style->width = reader->readU16();
  readColor(style->color);
  return style;
}

void DefineShapeTag::readLineStyleArray() {
  lineStyles.clear();
  lineStyles.push_back(LineStylePtr(NULL));
  
  uint16_t count = reader->readU8();
  if(count == 0xff) {
    count = reader->readU16();
    if(count == 0xffff) throw "LineStyleArray overflow."; // We need one spare id for NoLineStyle.
  }
  
  for(uint16_t id = 1; id <= count; ++id) {
    LineStyle *style = readLineStyle();
    lineStyles.push_back(LineStylePtr(style));
  }
}

void DefineShapeTag::readEdgeRecord() {
  bool isStraight = reader->readUB(1);
  uint8_t nbits = reader->readUB(4) + 2;
  printf("a %s edge, %d nbits\n", isStraight ? "straight" : "curved", nbits);
  
  if(isStraight) { // StraightEdgeRecord
    bool glFlag = reader->readUB(1); // general line line
    bool vlFlag = glFlag ? 0 : reader->readUB(1); // vertical line flag
    
    printf("  %s\n", glFlag ? "general" : (vlFlag ? "vertical" : "horizontal"));
    sb_t dx = glFlag || !vlFlag ? reader->readSB(nbits) : 0; // delta x
    sb_t dy = glFlag ||  vlFlag ? reader->readSB(nbits) : 0; // delta y
    
    printf("  to %lld, %lld\n", dx, dy);
    shape->lineTo(dx, dy);
  } else { // CurvedEdgeRecord (quadratic bezier)
    // control point
    sb_t cx = reader->readSB(nbits),
    cy = reader->readSB(nbits);
    
    // anchor point
    sb_t ax = reader->readSB(nbits),
    ay = reader->readSB(nbits);
    
    printf("  to %lld, %lld | %lld, %lld\n", cx, cy, ax, ay);
    shape->qlineTo(cx, cy, ax, ay);
  }
}

bool DefineShapeTag::readShapeRecord(uint8_t &fbits, uint8_t &lbits) {
  if(reader->readUB(1)) readEdgeRecord(); // EdgeRecord
  else { // StyleChange or EndRecord
    bool continueReading = false;
    
    bool stateNewStyles  = reader->readUB(1); continueReading |= stateNewStyles;
    bool stateLineStyle  = reader->readUB(1); continueReading |= stateLineStyle;
    bool stateFillStyle1 = reader->readUB(1); continueReading |= stateFillStyle1;
    bool stateFillStyle0 = reader->readUB(1); continueReading |= stateFillStyle0;
    bool stateMoveTo     = reader->readUB(1); continueReading |= stateMoveTo;
    
    if(!continueReading) // All flags are 0, so this is our EndRecord.
      return false;
    
    if(stateMoveTo) {
      uint8_t nbits = reader->readUB(5);
      sb_t x = reader->readSB(nbits),
      y = reader->readSB(nbits);
      shape->moveTo(x, y);
    }
    
    if(stateFillStyle0) {
      ub_t styleId = reader->readUB(fbits);
      shape->setFillStyle0(fillStyles[styleId]);
    }
    
    if(stateFillStyle1) {
      ub_t styleId = reader->readUB(fbits);
      shape->setFillStyle1(fillStyles[styleId]);
    }
    
    if(stateLineStyle) {
      ub_t styleId = reader->readUB(lbits);
      shape->setLineStyle(lineStyles[styleId]);
    }
    
    if(stateNewStyles) {
      readFillStyleArray();
      readLineStyleArray();
      
      fbits = reader->readUB(4);
      lbits = reader->readUB(4);
    }
  }
  
  return true;
}