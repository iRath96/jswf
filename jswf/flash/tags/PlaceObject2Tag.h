//
//  PlaceObject2Tag.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__PlaceObject2Tag__
#define __jswf__PlaceObject2Tag__

#include <stdio.h>
#include "TagWithReader.h"
#include "Header.h"
#include "Frame.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class PlaceObject2Tag : public TagWithReader {
      public:
        bool hasClipActions, hasClipDepth, hasName, hasRatio, hasColorTransform, hasMatrix, hasCharacter, doesMove;
        uint16_t depth;
        
        uint16_t characterId; // if hasCharacter
        Matrix matrix; // if hasMatrix
        uint16_t ratio; // if hasRatio
        std::string name; // if hasName
        uint16_t clipDepth; // if hasClipDepth
        
        void applyToFrame(Frame &frame) {
          if(hasCharacter) frame.displayList[depth].characterId = characterId;
          if(doesMove || hasCharacter) frame.displayList[depth].matrix = matrix;
          if(hasClipDepth) {
            frame.displayList[depth].doesClip = true;
            frame.displayList[depth].clipDepth = clipDepth;
          }
        }
        
        PlaceObject2Tag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          hasClipActions = reader->readUB(1);
          hasClipDepth = reader->readUB(1);
          hasName = reader->readUB(1);
          hasRatio = reader->readUB(1);
          hasColorTransform = reader->readUB(1);
          hasMatrix = reader->readUB(1);
          hasCharacter = reader->readUB(1);
          doesMove = reader->readUB(1);
          
          depth = reader->readU16();
          
          if(hasCharacter) characterId = reader->readU16();
          if(hasMatrix) flashReader.readMatrix(matrix);
          if(hasColorTransform) throw "ColorTransform not supported.";
          if(hasRatio) ratio = reader->readU16();
          if(hasName) name = reader->readString();
          if(hasClipDepth) clipDepth = reader->readU16();
          if(hasClipActions) throw "ClipActions not supported.";
        }
      };
    }
  }
}

#endif