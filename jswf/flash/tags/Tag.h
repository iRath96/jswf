//
//  GenericTag.h
//  jswf
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

/**
 * @file
 * Defines jswf::flash::tags::Tag
 */

#ifndef __jswf__GenericTag__
#define __jswf__GenericTag__

#include <stdio.h>
#include <stdint.h>
#include <string>

namespace jswf {
  namespace flash {
    namespace tags {
      typedef uint16_t tag_type_t;
      
      /**
       * Serves as super-class for all <tt>TAG</tt>s.
       */
      class Tag {
      public:
        tag_type_t type; //!< The tag-type as integer. \todo Make this an enum!
        std::string payload; //!< The payload of this tag as string.
        
        /**
         * Constructs a Tag.
         */
        Tag(tag_type_t type, std::string &payload) : type(type), payload(payload) {}
        virtual ~Tag() {};
      };
    }
  }
}

#endif /* defined(__jswf__GenericTag__) */