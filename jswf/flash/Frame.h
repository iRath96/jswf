//
//  Frame.h
//  jswf
//
//  Created by Alexander Rath on 14.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__Frame__
#define __jswf__Frame__

#include <stdio.h>
#include <map>

#include "Header.h"

namespace jswf {
  namespace avm2 {
    class Object;
    typedef std::shared_ptr<Object> ObjectPtr;
  }
  
  namespace flash {
    /**
     * @todo Should structures implement read/write themselves?
     * @todo This structure is wrong. <tt>usePreviousMatrix</tt> can be replaced with a <tt>getProperty</tt>-test on
     *       our <tt>avm2Object</tt> with a fallback to <tt>matrix</tt>.
     *       Also, <tt>avm2Object</tt> will not properly work in nested environments (tree structures).
     *       Same thing with modifications by AVM2 (adding/removing DisplayObjects).
     */
    struct DisplayListEntry {
      uint16_t characterId;
      
      avm2::ObjectPtr avm2Object = NULL;
      avm2::ObjectPtr onEnterFrame = NULL;
      
      bool usePreviousMatrix = false; //!< Set to <tt>true</tt> if matrix was altered by a script and has to be kept.
      
      bool doesClip = false;
      uint16_t clipDepth;
      
      bool setsColorTransform = false;
      
      Matrix matrix;
      ColorTransform colorTransform;
    };
    
    class Frame {
    public:
      std::map<uint16_t, DisplayListEntry> displayList;
    };
  }
}

#endif /* defined(__jswf__Frame__) */