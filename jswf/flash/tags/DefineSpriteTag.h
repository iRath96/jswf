//
//  DefineSpriteTag.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__DefineSpriteTag__
#define __jswf__DefineSpriteTag__

#include <stdio.h>
#include "TagWithDictionaryElement.h"
#include "Sprite.h"
#include "Tags.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class DefineSpriteTag : public TagWithDictionaryElement {
      public:
        Sprite *sprite;
        DefineSpriteTag(tag_type_t t, std::string &p) : TagWithDictionaryElement(t, p) {
          sprite = new Sprite();
          element.reset(sprite);
          
          sprite->id = reader->readU16();
          sprite->frameCount = reader->readU16();
          
          Frame frame;
          
          while(true) { // TODO:2014-12-14:alex:Not DRY with flash::Document.
            tags::Tag *tag = flashReader.readTag();
            sprite->tags.push_back(std::shared_ptr<tags::Tag>(tag));
            
            if(dynamic_cast<tags::EndTag *>(tag)) break;
            
            if(dynamic_cast<tags::PlaceObject2Tag *>(tag)) {
              tags::PlaceObject2Tag *po2t = (tags::PlaceObject2Tag *)tag;
              printf("PlaceObject2\n");
              po2t->applyToFrame(frame);
            }
            
            if(dynamic_cast<tags::ShowFrameTag *>(tag)) {
              printf("ShowFrame\n");
              sprite->frames.push_back(frame);
            }
          }
        }
      };
    }
  }
}

#endif /* defined(__jswf__DefineSpriteTag__) */