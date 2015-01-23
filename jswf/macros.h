//
//  macros.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

/**
 * @file
 * Defines macros used across the software.
 * @see undef-macros.h
 * @todo Better prefix needed.
 */

/**
 * @def __read_array(ename,array,count,code)
 * Resizes a container and iterates given code over all elements.
 * @param ename The name for the reference to the current element in iteration.
 * @param array The name of the container to resize and iterate over.
 * @param count The size to resize the container to (the count of the elements).
 * @param code The code to execute for every element.
 */
#define __read_array(ename,array,count,code) { \
  size_t i = 0, j = (count); \
  array.resize(j); \
  for(; i < j; ++i) { \
    auto &ename = array[i]; \
    code \
  } \
}