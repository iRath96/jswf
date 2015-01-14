//
//  PlaceObject2Tag.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__PlaceObject2Tag__
#define __jswf__PlaceObject2Tag__

#include "ITagForSprite.h"
#include "TagWithReader.h"

#include "Header.h"
#include "Frame.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Used to add a <tt>Character</tt> to the <tt>DisplayList</tt> or to modify an existing
       * <tt>Character</tt> in the <tt>DisplayList</tt>
       * @todo Implement <tt>PlaceObject</tt>
       * @todo Create a super-class for instances of <tt>TAG</tt> that modify the display list.
       */
      class PlaceObject2Tag : public TagWithReader, public ITagForSprite {
      public:
        bool hasClipActions,    //!< Whether <tt>AVM</tt> actions for clip events are specified
             hasClipDepth,      //!< Whether the <tt>Character</tt> is used as mask layer
             hasName,           //!< Whether an instance name for the character is specified
             hasRatio,          //!< Whether a morph-ratio is specified
             hasColorTransform, //!< Whether a <tt>CXFORMALPHA</tt> ColorTransform is specified
             hasMatrix,         //!< Whether a <tt>MATRIX</tt> for transformation is specified
             hasCharacter,      //!< Whether a new <tt>Character</tt> is given, otherwise an existing <tt>Character</tt> is modified
             doesMove;          //!< This field is apparently useless, we read it nevertheless.
        uint16_t depth; //!< The depth in the <tt>displayList</tt> to operate on.
        
        uint16_t characterId; //!< A <tt>DICTIONARY</tt>-identifier if hasCharacter is set. \see Document::dictionary
        Matrix matrix;        //!< A <tt>MATRIX</tt> record for transformation if hasMatrix is set.
        uint16_t ratio;       //!< A morph-ratio if hasRatio is set.
        std::string name;     //!< A string if hasName is set.
        uint16_t clipDepth;   //!< The depth up to which this masks if hasClipDepth is set.
        ColorTransform colorTransform; //!< A <tt>CXFORMWITHALPHA</tt> ColorTransform if hasColorTransform is set.
        
        /**
         * Applies the changes as described by this tag to the displayList of a given frame.
         * @param [in,out] frame The frame, the displayList of which is being altered by this operation.
         */
        void applyToSprite(Sprite &sprite) {
          Frame &frame = sprite.temporaryFrame;
          if(hasCharacter) frame.displayList[depth].characterId = characterId;
          if(hasMatrix) frame.displayList[depth].matrix = matrix;
          if(hasColorTransform) {
            frame.displayList[depth].setsColorTransform = true;
            frame.displayList[depth].colorTransform = colorTransform;
          }
          
          if(hasClipDepth) {
            frame.displayList[depth].doesClip = true;
            frame.displayList[depth].clipDepth = clipDepth;
          }
        }
        
        /**
         * Constructs the <tt>TAG</tt> and parses the payload.
         */
        PlaceObject2Tag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          hasClipActions = reader->readUB(1);
          hasClipDepth = reader->readUB(1);
          hasName = reader->readUB(1);
          hasRatio = reader->readUB(1);
          hasColorTransform = reader->readUB(1);
          hasMatrix = reader->readUB(1);
          hasCharacter = reader->readUB(1);
          doesMove = reader->readUB(1);
          
          // TODO:2014-12-25:alex:What does doesMove do?!
          
          depth = reader->readU16();
          
          if(hasCharacter) characterId = reader->readU16();
          if(hasMatrix) flashReader.readMatrix(matrix);
          if(hasColorTransform) flashReader.readColorTransform(colorTransform, true);
          if(hasRatio) ratio = reader->readU16();
          if(hasName) name = reader->readString();
          if(hasClipDepth) clipDepth = reader->readU16();
          if(hasClipActions) throw "ClipActions not supported yet.";
        }
      };
    }
  }
}

#endif