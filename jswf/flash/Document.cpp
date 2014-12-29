//
//  Document.cpp
//  jswf/flash
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "Document.h"
#include "ITagForSprite.h"
#include "ITagForDocument.h"

#include "EndTag.h"

#include "ITagWithCharacter.h"

using namespace jswf::flash;

void Document::read() {
  reader.readHeader(header);
  while(true) {
    tags::Tag *tag = reader.readTag();
    tags.push_back(std::shared_ptr<tags::Tag>(tag));
    
    if(dynamic_cast<tags::EndTag *>(tag)) break;
    
    if(dynamic_cast<tags::ITagForDocument *>(tag))
      (dynamic_cast<tags::ITagForDocument *>(tag))->applyToDocument(*this);
    if(dynamic_cast<tags::ITagForSprite *>(tag))
      (dynamic_cast<tags::ITagForSprite *>(tag))->applyToSprite(rootSprite);
  }
  
  printf("Okay.\n");
}