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
      /**
       * Serves as super-class for all `TAG`s that use
       * an io::StringReader and/or a flash::Reader to parse their
       * payload.
       */
      class TagWithReader : public Tag {
      public:
        tag_type_t type;
        std::string payload;
        
        std::shared_ptr<io::StringReader> reader; //!< The io::StringReader that operates on \ref payload
        flash::Reader flashReader; //!< The flash::Reader that operates on \ref reader
        
        TagWithReader(tag_type_t type, std::string &payload) : Tag(type, payload),
          reader(new io::StringReader(payload)),
          flashReader(std::shared_ptr<io::GenericReader>(reader)) {}
      };
    }
  }
}

#endif /* defined(__jswf__TagWithReader__) */