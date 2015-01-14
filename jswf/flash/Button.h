//
//  Button.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__Button__
#define __jswf__Button__

#include "Character.h"
#include "Frame.h"

namespace jswf {
  namespace flash {
    /**
     * Represents a <tt>BUTTON</tt> character.
     */
    class Button : public Character {
    public:
      enum StateEnum {
        UpState = 0,
        OverState = 1,
        DownState = 2,
        HitTestState = 3
      };
      
      Frame frames[4];
      bool trackAsMenu = false;
    };
  }
}

#endif /* defined(__jswf__Button__) */