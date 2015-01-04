//
//  VM.cpp
//  jswf
//
//  Created by Alexander Rath on 28.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "VM.h"
#include "Frame.h"

namespace jswf {
  namespace avm2 {
    ObjectPtr builtin_trace(VM &vm, MethodInfo *method, std::vector<ObjectPtr> &arguments) {
      for(size_t i = 0, j = arguments.size(); i < j; ++i) {
        if(i) printf(" ");
        printf("%s", arguments[i]->coerce_s().c_str());
      } printf("\n");
      return vm.undefinedObject;
    }
    
    ObjectPtr builtin_getQualifiedClassName(VM &vm, MethodInfo *method, std::vector<ObjectPtr> &arguments) {
      return ObjectPtr(vm.makeStringObject(arguments[0]->klass->iinfo.name->nameString()));
    }
    
    void DisplayObject::setProperty(const Multiname &property, const ObjectPtr &value) {
      Object::setProperty(property, value);
      std::string name = property.nameString();
      if(name == "rotation") {
        double rad = value->coerce_d() / 180 * M_PI;
        // x' = x * sx + y * r1
        // y' = y * sy + y * r0
        
        entry->matrix.sx = cos(rad);
        entry->matrix.sy = cos(rad);
        
        entry->matrix.r1 = -sin(rad);
        entry->matrix.r0 = +sin(rad);
      } else if(name == "scaleX") {
        double v = value->coerce_d();
        entry->matrix.sx = v;
      } else if(name == "x") {
        double v = value->coerce_d();
        entry->matrix.tx = v * 20;
      }
    }
  }
}