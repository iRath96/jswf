//
//  Metadata.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_Metadata_h
#define jswf_Metadata_h

#include <string>
#include <vector>

namespace jswf {
  namespace avm2 {
    struct MetadataItem {
      std::string *key, *value;
    };
    
    struct Metadata {
      std::string *name;
      std::vector<MetadataItem> items;
    };
  }
}

#endif