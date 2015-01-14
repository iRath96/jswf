//
//  FileAttributesTag.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_FileAttributesTag_h
#define jswf_FileAttributesTag_h

#include "TagWithReader.h"

// TODO:2014-12-28:alex:There are a lot of "Used to [...]" in the documentation. I don't like that.

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Represents <tt>FileAttribute</tt> tags.
       */
      class FileAttributesTag : public TagWithReader {
      public:
        bool use_direct_blit, use_gpu, has_metadata, is_as3, use_network;
        FileAttributesTag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          reader->readUB(1);
          use_direct_blit = reader->readUB(1);
          use_gpu         = reader->readUB(1);
          has_metadata    = reader->readUB(1);
          is_as3          = reader->readUB(1);
          
          reader->readUB(2);
          use_network = reader->readUB(1);
        }
      };
    }
  }
}

#endif