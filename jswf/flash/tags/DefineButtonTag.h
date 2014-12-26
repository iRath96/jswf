//
//  DefineButton.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineButtonTag_h
#define jswf_DefineButtonTag_h

#include "TagWithDictionaryElement.h"
#include "Button.h"
#include "Frame.h"

#include <vector>
#include <assert.h>

namespace jswf {
  namespace flash {
    namespace tags {
      class DefineButtonTag : public TagWithDictionaryElement {
      protected:
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
          
          DisplayObject displayObject;
          displayObject.characterId = reader->readU16();
          
          uint16_t depth = reader->readU16();
          
          flashReader.readMatrix(displayObject.matrix);
          flashReader.readColorTransform(displayObject.colorTransform, true);
          displayObject.setsColorTransform = true;
          
          if(hasFilterList) throw "Filter-list not supported.";
          if(hasBlendMode) throw "Blend-mode not supported.";
          
#define appendToFrame(flag) if(state ## flag) button->frames[Button::flag ## State].displayList[depth] = displayObject;
          appendToFrame(HitTest);
          appendToFrame(Down);
          appendToFrame(Over);
          appendToFrame(Up);
#undef appendToFrame
          
          return true;
        }
        
        virtual void readBetween() {} // TODO:2014-12-15:alex:Find a better name.
        
        void read() {
          button = new Button();
          element.reset(button);
          
          button->id = reader->readU16();
          printf("DefineButton id=%d\n", button->id);
          
          this->readBetween();
          
          while(readButtonRecord());
        }
      public:
        Button *button;
        
        DefineButtonTag(tag_type_t t, std::string &p) : TagWithDictionaryElement(t, p) { read(); }
        DefineButtonTag(tag_type_t t, std::string &p, bool) : TagWithDictionaryElement(t, p) {}
      };
    }
  }
}

#endif