//
//  macros.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#define __read_array(ename, array, count, code) { \
  size_t i = 0, j = (count); \
  array.resize(j); \
  for(; i < j; ++i) { \
    auto &ename = array[i]; \
    code \
  } \
}