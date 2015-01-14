//
//  Tag.h
//  jswf/flash
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf_flash__Tag__
#define __jswf_flash__Tag__

#include <stdio.h>
#include <string>

#include "Tag.h"

namespace jswf {
  namespace flash {
    /**
     * Provides methods to create a polymorphistic Tag by a given type identifier.
     */
    class TagFactory {
    public:
      /**
       * Creates a Tag using the specified type identifier and payload.
       * @param [in] type    Type identifier to determine the class to be instantiated for the Tag.
       * @param [in] payload The Tag's payload.
       * @return The constructed Tag.
       */
      static tags::Tag *create(tags::tag_type_t, std::string &);
    };
  }
}

#endif