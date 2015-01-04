//
//  DoABCTag.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DoABCTag_h
#define jswf_DoABCTag_h

#include "ITagForDocument.h"
#include "TagWithReader.h"
#include "ABCFile.h"

namespace jswf {
  namespace flash {
    namespace tags {
      /**
       * Carries an `ABCFile` to be executed by the \ref avm2::VM .
       */
      class DoABCTag : public TagWithReader, public ITagForDocument {
      public:
        uint32_t flags;
        std::string name;
        
        std::shared_ptr<avm2::ABCFile> abc;
        
        DoABCTag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          flags = reader->readU32();
          name = reader->readString();
          
          abc = std::shared_ptr<avm2::ABCFile>(new avm2::ABCFile(reader));
        }
        
        void applyToDocument(Document &document) {
          document.avm2.loadABCFile(abc);
        }
      };
    }
  }
}

#endif