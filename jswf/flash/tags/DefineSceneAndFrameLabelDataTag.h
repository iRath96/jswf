//
//  DefineSceneAndFrameLabelDataTag.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_DefineSceneAndFrameLabelDataTag_h
#define jswf_DefineSceneAndFrameLabelDataTag_h

#include "TagWithReader.h"
#include "Reader.h"
#include "Header.h"
#include "macros.h"

namespace jswf {
  namespace flash {
    struct FrameLabel {
      u32_t frame;
      string label;
    };
    
    struct Scene {
      u32_t offset;
      string name;
    };
    
    namespace tags {
      class DefineSceneAndFrameLabelDataTag : public TagWithReader {
      public:
        std::vector<Scene> scenes;
        std::vector<FrameLabel> frameLabels;
        
        DefineSceneAndFrameLabelDataTag(tag_type_t t, std::string &p) : TagWithReader(t, p) {
          __read_array(scene, scenes, reader->readVU32(), {
            scene.offset = reader->readVU32();
            scene.name = reader->readString();
          });
          
          __read_array(label, frameLabels, reader->readVU32(), {
            label.frame = reader->readVU32();
            label.label = reader->readString();
          });
        }
      };
    }
  }
}

#include "undef-macros.h"

#endif