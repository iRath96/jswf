//
//  Document.h
//  jswf/flash
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf_flash__Document__
#define __jswf_flash__Document__

#include <stdio.h>
#include "Reader.h"
#include "Header.h"
#include "DictionaryElement.h"
#include "Tags.h"

#include "Frame.h"

#include <vector>
#include <map>

namespace jswf {
  namespace flash {
    class Document {
    public:
      Header header;
      Frame frame;
      
      std::vector<std::shared_ptr<tags::Tag>> tags;
      std::map<uint16_t, std::shared_ptr<DictionaryElement>> dictionary;
      std::vector<Frame> frames;
      
      Reader reader;
      Document(std::shared_ptr<jswf::io::GenericReader> reader) : reader(reader) { read(); }
      Document(jswf::io::GenericReader *reader) : reader(*reader) { read(); }
    private:
      void read();
    };
  }
}

#endif