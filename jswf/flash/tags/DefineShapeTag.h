//
//  Defineshape->h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineShapeTag_h
#define jswf_DefineShapeTag_h

#include "TagWithDictionaryElement.h"
#include "Shape.h"

#include <vector>
#include "Styles.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class DefineShapeTag : public TagWithDictionaryElement {
        std::vector<styles::FillStylePtr> fillStyles;
        std::vector<styles::LineStylePtr> lineStyles;
        
        uint32_t fillCounter = 0, lineCounter = 0;
        
      protected:
        virtual void readColor(RGBA &rgba) {
          flashReader.readRGB(rgba);
        }
        
        virtual uint16_t readArrayCount() {
          uint8_t count = reader->readU8();
          if(count == 0xff) throw "Array overflow (count=0xff)";
          return count;
        }
        
        /**
         * Reads a FillStyle from our payload stream.
         * @return A pointer to a newly created FillStyle on the heap.
         */
        styles::FillStyle *readFillStyle();
        
        /**
         * Reads the FillStyle-array used by the following drawing operations into our fillStyles-member.
         */
        void readFillStyleArray();
        
        /**
         * Reads a LineStyle from our payload stream.
         * @return A pointer to a newly created LineStyle on the heap.
         */
        virtual styles::LineStyle *readLineStyle();
        
        /**
         * Reads the LineStyle-array used by the following drawing operations into our lineStyles-member.
         */
        void readLineStyleArray();
        
        /**
         * Reads an 'Edge Record', the actions of which are projected on our shape-member.
         */
        void readEdgeRecord();
        
        /**
         * Reads a 'SHAPERECORD', the actions of which are projected on our shape-member.
         * @param [in,out] nbits for 'FillStyle', changed upon 'StateNewStyles'
         * @param [in,out] nbits for 'LineStyle', changed upon 'StateNewStyles'
         * @return Whether to continue reading (false means this was the last record)
         */
        bool readShapeRecord(uint8_t &fbits, uint8_t &lbits);
        
      protected:
        virtual void readBetween() { // TODO:2014-12-15:alex:Find a better name.
          printf("nothing.");
        }
        
        void read() {
          shape = new Shape();
          element.reset(shape);
          
          shape->id = reader->readU16();
          printf("DefineShape id=%d\n", shape->id);
          
          flashReader.readRect(shape->bounds);
          shape->edgeBounds = shape->bounds; // Since this is DefineShape version 1
          
          printf("Now readbetween\n");
          this->readBetween();
          
          // Read SHAPEWITHSTYLE
          
          readFillStyleArray();
          readLineStyleArray();
          
          uint8_t fbits = reader->readUB(4),
                  lbits = reader->readUB(4);
          
          while(readShapeRecord(fbits, lbits));
        }
      public:
        Shape *shape;
        
        DefineShapeTag(tag_type_t t, std::string &p) : TagWithDictionaryElement(t, p) { read(); }
        DefineShapeTag(tag_type_t t, std::string &p, bool) : TagWithDictionaryElement(t, p) {}
      };
    }
  }
}

#endif