//
//  Tag.cpp
//  jswf/flash
//
//  Created by Alexander Rath on 13.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "TagFactory.h"
#include "Tags.h"

using namespace jswf::flash;
using namespace jswf::flash::tags;

tags::Tag *TagFactory::create(tags::tag_type_t type, std::string &payload) {
#define __(type, klass) \
case type: return new klass(type, payload);
  switch(type) {
    __( 0, EndTag);
    __( 1, ShowFrameTag);
    __( 2, DefineShapeTag);
    __( 9, SetBackgroundColorTag);
    __(26, PlaceObject2Tag);
    __(32, DefineShape3Tag);
    __(39, DefineSpriteTag);
    __(69, FileAttributesTag);
  } return new tags::Tag(type, payload);
}