//
//  DefineButton.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineButtonTag_h
#define jswf_DefineButtonTag_h

#include "TagWithReader.h"
#include "TagWithCharacter.h"

#include "Button.h"
#include "Frame.h"

#include <vector>
#include <assert.h>

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Parses a <tt>BUTTON</tt> record that is to be added to the document's <tt>DICTIONARY</tt>.
       */
      class DefineButtonTag : public TagWithReader, public TagWithCharacter {
      protected:
        /**
         * Reads a 'BUTTONRECORD'. The changes to the frame are applied to button.
         * @return <tt>true</tt> if further records follow, <tt>false</tt> if this was the last record.
         * @throw (TODO) if the reserved field was non-zero.
         * @see read
         */
        bool readButtonRecord() {
          uint8_t reserved = reader->readUB(2);
          assert(reserved == 0);
          
          bool hasBlendMode = reader->readUB(1);
          bool hasFilterList = reader->readUB(1);
          
          bool continueReading = false;
          bool stateHitTest = reader->readUB(1); continueReading |= stateHitTest;
          bool stateDown    = reader->readUB(1); continueReading |= stateDown;
          bool stateOver    = reader->readUB(1); continueReading |= stateOver;
          bool stateUp      = reader->readUB(1); continueReading |= stateUp;
          
          if(!continueReading) return false;
          
          DisplayListEntry dlEntry;
          dlEntry.characterId = reader->readU16();
          
          uint16_t depth = reader->readU16();
          
          flashReader.readMatrix(dlEntry.matrix);
          flashReader.readColorTransform(dlEntry.colorTransform, true);
          dlEntry.setsColorTransform = true;
          
          if(hasFilterList) throw "Filter-list not supported.";
          if(hasBlendMode) throw "Blend-mode not supported.";
          
#define appendToFrame(flag) if(state ## flag) button->frames[Button::flag ## State].displayList[depth] = dlEntry;
          appendToFrame(HitTest);
          appendToFrame(Down);
          appendToFrame(Over);
          appendToFrame(Up);
#undef appendToFrame
          
          return true;
        }
        
        /**
         * @see DefineShapeTag::readBetween
         */
        virtual void readBetween() {} // TODO:2014-12-15:alex:Find a better name.
        
        /**
         * Instantiates button and parses the payload.
         * @see readBetween
         * @see readButtonRecord
         */
        void read() {
          button = new Button();
          character.reset(button);
          
          button->id = reader->readU16();
          printf("DefineButton id=%d\n", button->id);
          
          this->readBetween();
          
          while(readButtonRecord());
        }
      public:
        Button *button; //!< The button described by this tag.
        
        DefineButtonTag(tag_type_t t, std::string &p) : TagWithReader(t, p) { read(); }
        DefineButtonTag(tag_type_t t, std::string &p, bool) : TagWithReader(t, p) {}
      };
    }
  }
}

#endif