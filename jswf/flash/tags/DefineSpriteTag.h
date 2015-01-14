//
//  DefineSpriteTag.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__DefineSpriteTag__
#define __jswf__DefineSpriteTag__

#include "TagWithReader.h"
#include "TagWithCharacter.h"

#include "ITagForSprite.h"
#include "EndTag.h"

#include "Sprite.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Parses a <tt>SPRITE</tt> record that is to be added to the document's <tt>DICTIONARY</tt>.
       */
      class DefineSpriteTag : public TagWithReader, public TagWithCharacter {
      public:
        Sprite *sprite;
        DefineSpriteTag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          sprite = new Sprite();
          character.reset(sprite);
          
          sprite->id = reader->readU16();
          sprite->frameCount = reader->readU16();
          
          Frame frame;
          
          while(true) {
            tags::Tag *tag = flashReader.readTag();
            sprite->tags.push_back(std::shared_ptr<tags::Tag>(tag));
            
            if(dynamic_cast<tags::EndTag *>(tag)) break;
            if(dynamic_cast<tags::ITagForSprite *>(tag))
              (dynamic_cast<tags::ITagForSprite *>(tag))->applyToSprite(*sprite);
          }
        }
      };
    }
  }
}

#endif /* defined(__jswf__DefineSpriteTag__) */