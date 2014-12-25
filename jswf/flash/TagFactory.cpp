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
    __( 7, DefineButtonTag);
    __( 9, SetBackgroundColorTag);
    __(22, DefineShape2Tag);
    __(26, PlaceObject2Tag);
    __(28, RemoveObject2Tag);
    __(32, DefineShape3Tag);
    __(34, DefineButton2Tag);
    __(39, DefineSpriteTag);
    __(69, FileAttributesTag);
    __(83, DefineShape4Tag);
  } return new tags::Tag(type, payload);
}