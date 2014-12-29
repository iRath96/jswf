//
//  Defineshape->h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineShapeTag_h
#define jswf_DefineShapeTag_h

#include "TagWithReader.h"
#include "ITagWithCharacter.h"

#include "Shape.h"

#include <vector>
#include "Styles.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Parses a `SHAPE` record that is to be added to the document's `DICTIONARY`.
       */
      class DefineShapeTag : public TagWithReader, public ITagWithCharacter {
      protected:
        std::vector<styles::FillStylePtr> fillStyles; //!< FillStyleArray used for drawing operations.
        std::vector<styles::LineStylePtr> lineStyles; //!< LineStyleArray used for drawing operations.
        
        uint32_t fillCounter = 0, //!< Used to assign unique identifiers to styles (drawing order)
                 lineCounter = 0; //!< Used to assign unique identifiers to styles (drawing order)
        
      protected:
        /**
         * Reads a color from the payload stream.
         * Subclasses can override this to add support for other color schemes (e. G. RGBA)
         * @param [out] rgba The RGBA-field to read into.
         * @see readFillStyle
         * @see readLineStyle
         */
        virtual void readColor(RGBA &rgba) {
          flashReader.readRGB(rgba);
        }
        
        /**
         * Reads an array count from the payload stream.
         * Subclasses can override this to add support for longer arrays.
         * @throw (TODO) ExceptionClass when count == 0xff
         * @return The count in [0;0xff)
         * @see readLineStyleArray
         * @see readFillStyleArray
         */
        virtual uint16_t readArrayCount() {
          uint8_t count = reader->readU8();
          if(count == 0xff) throw "Array overflow (count=0xff)";
          return count;
        }
        
        /**
         * Reads a FillStyle from the payload stream.
         * @return A pointer to a newly created FillStyle on the heap.
         * @todo Some of these styles should _only_ be implemented _in subclasses_.
         */
        styles::FillStyle *readFillStyle();
        
        /**
         * Reads the FillStyleArray used by the following drawing operations.
         * @see fillStyles
         * @see readFillStyle
         */
        void readFillStyleArray();
        
        /**
         * Reads a LineStyle from the payload stream.
         * @return A pointer to a newly created LineStyle on the heap.
         */
        virtual styles::LineStyle *readLineStyle();
        
        /**
         * Reads the LineStyleArray used by the following drawing operations.
         * @see lineStyles
         * @see readLineStyle
         */
        void readLineStyleArray();
        
        /**
         * Reads an 'Edge Record', the actions of which are projected on our shape.
         * @see shape
         */
        void readEdgeRecord();
        
        /**
         * Reads a 'SHAPERECORD', the actions of which are projected on our shape.
         * @param [in,out] fbits,lbits nbits for 'FillStyle' and 'LineStyle', changed upon 'StateNewStyles'
         * @return Whether other records follow after this record (false means this was the last record)
         * @see shape
         */
        bool readShapeRecord(uint8_t &fbits, uint8_t &lbits);
        
      protected:
        /**
         * Implemented by subclasses to perform reads between 'Bounds' and 'FillStyleArray'.
         */
        virtual void readBetween() {} // TODO:2014-12-15:alex:Find a better name.
        
        /**
         * Parses the payload.
         */
        void read() {
          shape = new Shape();
          character.reset(shape);
          
          shape->id = reader->readU16();
          printf("DefineShape id=%d\n", shape->id);
          
          flashReader.readRect(shape->bounds);
          shape->edgeBounds = shape->bounds; // Since this is DefineShape version 1
         
          this->readBetween();
          
          // Read SHAPEWITHSTYLE
          
          readFillStyleArray();
          readLineStyleArray();
          
          uint8_t fbits = reader->readUB(4),
                  lbits = reader->readUB(4);
          
          while(readShapeRecord(fbits, lbits));
          
          shape->polygonize();
        }
      public:
        Shape *shape; //!< The shape we are projecting the 'SHAPERECORD's we read upon.
        
        /**
         * Constructs a DefineShapeTag and parses the payload using 'read'
         * @see read
         */
        DefineShapeTag(tag_type_t t, std::string &p) : TagWithReader(t, p) { read(); }
        
        /**
         * Constructs a DefineShapeTag without parsing the payload.
         * Used by subclasses so they can call \ref read with the polymorphistic \ref readBetween .
         * @param [in] bool ignored
         */
        DefineShapeTag(tag_type_t t, std::string &p, bool) : TagWithReader(t, p) {}
      };
    }
  }
}

#endif