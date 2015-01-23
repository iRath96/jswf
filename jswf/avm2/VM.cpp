//
//  VM.cpp
//  jswf
//
//  Created by Alexander Rath on 28.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "VM.h"
#include "Frame.h"

// name = {$VAR->iinfo.name.__ptr_->name}:s
// klass = {$VAR->klass->iinfo.name.__ptr_->name}:s

// jswf::avm2::MultinamePtr => name = {$VAR.__ptr_->name}:s, ns = {$VAR.__ptr_->ns.__ptr_->name}:s
// jswf::avm2::TraitInfo => {$VAR->name}:s, kind = {$VAR->kind}

namespace jswf {
  namespace avm2 {
#pragma mark - Builtin methods
#define builtin(name) ObjectPtr builtin_##name(VM &vm, MethodInfo *method, std::vector<ObjectPtr> &arguments)
    
    builtin(trace) {
      for(size_t i = 1, j = arguments.size(); i < j; ++i) {
        if(i != 1) printf(" ");
        printf("%s", arguments[i]->coerce_s().c_str());
      } printf("\n");
      return vm.undefinedObject;
    }
    
    builtin(getQualifiedClassName) {
      return ObjectPtr(new StringObject(&vm, arguments[1]->klass->iinfo.name->nameString()));
    }
    
    builtin(addEventListener) {
      if(dynamic_cast<DisplayObject *>(arguments[0].get())) {
        DisplayObject *dObj = dynamic_cast<DisplayObject *>(arguments[0].get());
        dObj->value->onEnterFrame = arguments[2];
      }
      
      return vm.undefinedObject;
    }
    
#undef builtin
  }
}