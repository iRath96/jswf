//
//  ITagForDocument.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_ITagForDocument_h
#define jswf_ITagForDocument_h

#include "Document.h"

namespace jswf {
  namespace flash {
    namespace tags {
      class ITagForDocument {
      public:
        virtual void applyToDocument(Document &document) = 0;
        virtual ~ITagForDocument() {}
      };
    }
  }
}

#endif