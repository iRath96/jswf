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
  rootSprite = new Sprite();
  dictionary[0] = std::shared_ptr<Sprite>(rootSprite);
  
  reader.readHeader(header);
  while(true) {
    tags::Tag *tag = reader.readTag();
    tags.push_back(std::shared_ptr<tags::Tag>(tag));
    
    if(dynamic_cast<tags::EndTag *>(tag)) break;
    
    if(dynamic_cast<tags::ITagForDocument *>(tag))
      (dynamic_cast<tags::ITagForDocument *>(tag))->applyToDocument(*this);
    if(dynamic_cast<tags::ITagForSprite *>(tag))
      (dynamic_cast<tags::ITagForSprite *>(tag))->applyToSprite(*rootSprite);
    
    if(dynamic_cast<tags::ITagWithCharacter *>(tag)) // TODO:2014-12-30:alex:This is a dirty hack.
      (dynamic_cast<tags::ITagWithCharacter *>(tag))->character->avm2Class = avm2.getClassByName("flash.display.MovieClip");
  }
}