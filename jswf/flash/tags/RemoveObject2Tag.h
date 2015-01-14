//
//  RemoveObject2Tag.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_RemoveObject2Tag_h
#define jswf_RemoveObject2Tag_h

#include "TagWithReader.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Used to remove a <tt>Character</tt> at a given depth from the <tt>DisplayList</tt>.
       * @todo Implement <tt>RemoveObject</tt>
       */
      class RemoveObject2Tag : public TagWithReader, public ITagForSprite {
      public:
        uint16_t depth; //!< The depth at which to delete at.
        
        void applyToSprite(Sprite &sprite) {
          sprite.temporaryFrame.displayList.erase(depth);
        }
        
        RemoveObject2Tag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          depth = reader->readU16();
        }
      };
    }
  }
}

#endif