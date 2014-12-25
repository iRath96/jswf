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
      class RemoveObject2Tag : public TagWithReader {
      public:
        uint16_t depth;
        
        void applyToFrame(Frame &frame) {
          frame.displayList.erase(depth);
        }
        
        RemoveObject2Tag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          depth = reader->readU16();
        }
      };
    }
  }
}

#endif