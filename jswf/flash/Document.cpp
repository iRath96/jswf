//
//  Document.cpp
//  jswf/flash
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "Document.h"

using namespace jswf::flash;

void Document::read() {
  reader.readHeader(header);
  while(true) {
    tags::Tag *tag = reader.readTag();
    tags.push_back(std::shared_ptr<tags::Tag>(tag));
    
    if(dynamic_cast<tags::EndTag *>(tag)) break;
    
    if(dynamic_cast<tags::TagWithDictionaryElement *>(tag)) {
      tags::TagWithDictionaryElement *twde = (tags::TagWithDictionaryElement *)tag;
      dictionary[twde->element->id] = twde->element;
    }
    
    if(dynamic_cast<tags::PlaceObject2Tag *>(tag)) {
      tags::PlaceObject2Tag *po2t = (tags::PlaceObject2Tag *)tag;
      printf("PlaceObject2\n");
      po2t->applyToFrame(frame);
    }
    
    if(dynamic_cast<tags::ShowFrameTag *>(tag)) {
      printf("ShowFrame\n");
      frames.push_back(frame);
    }
  }
  
  printf("Okay.\n");
}