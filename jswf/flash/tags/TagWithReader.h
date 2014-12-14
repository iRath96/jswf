//
//  TagWithReader.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__TagWithReader__
#define __jswf__TagWithReader__

#include "Tag.h"
#include "StringReader.h"
#include "Reader.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class TagWithReader : public Tag {
      public:
        tag_type_t type;
        std::string payload;
        
        io::StringReader *reader;
        flash::Reader flashReader;
        
        TagWithReader(tag_type_t type, std::string &payload) : Tag(type, payload),
          reader(new io::StringReader(payload)),
          flashReader(*reader) {}
      };
    }
  }
}

#endif /* defined(__jswf__TagWithReader__) */