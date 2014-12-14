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
    class TagFactory {
    public:
      static tags::Tag *create(tags::tag_type_t, std::string &);
    };
  }
}

#endif