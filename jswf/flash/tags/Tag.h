//
//  GenericTag.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__GenericTag__
#define __jswf__GenericTag__

#include <stdio.h>
#include <stdint.h>
#include <string>

namespace jswf {
  namespace flash {
    namespace tags {
      typedef uint16_t tag_type_t;
      
      class Tag {
      public:
        tag_type_t type;
        std::string payload;
        
        Tag(tag_type_t type, std::string &payload) : type(type), payload(payload) {}
        virtual ~Tag() {};
      };
    }
  }
}

#endif /* defined(__jswf__GenericTag__) */