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
    /**
     * FrameLabels for the main timeline.
     */
    struct FrameLabel {
      u32_t frame; //!< The frame-index, starting at 0
      string label; //!< The label for the frame
    };
    
    /**
     * Scenes for the main timeline.
     */
    struct Scene {
      u32_t offset; //!< The frame-offset of the scene, starting at 0
      string name; //!< The name for the scene
    };
    
    namespace tags {
      /**
       * Carries `Scene`s and `FrameLabel`s.
       */
      class DefineSceneAndFrameLabelDataTag : public TagWithReader {
      public:
        std::vector<Scene> scenes; //!< The \ref Scene "scenes" that are described by this tag.
        std::vector<FrameLabel> frameLabels; //!< The \ref FrameLabel "frame labels" that are described by this tag.
        
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