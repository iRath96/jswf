//
//  Button.h
//  jswf
//
//  Created by Alexander Rath on 25.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__Button__
#define __jswf__Button__

#include <stdio.h>
#include "DictionaryElement.h"
#include "Frame.h"

namespace jswf {
  namespace flash {
    class Button : public DictionaryElement {
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