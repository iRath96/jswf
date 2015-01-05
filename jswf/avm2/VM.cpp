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
#define _(name) Object(vm, vm->getClassByName(name)) {}
#define __(name, ...) name ## Object::name ## Object(VM *vm, __VA_ARGS__)
    __(Function      , MethodInfo *value                 ) : value(value),          _("Function")
    __(Method        , const ObjectPtr &r, MethodInfo *v ) : receiver(r), value(v), _("builtin.as$0::MethodClosure")
    __(BuiltinMethod , builtin_method_t *value           ) : value(value),          _("builtin.as$0::MethodClosure")
    __(Class         , Class *value                      ) : value(value),          _("Class")
#undef _
    
#pragma mark - NativeObject constructors
#define create_constructor(c_name, as_name) c_name::c_name(VM *vm, const typeof(c_name::value) &value) : value(value), \
  NativeObject(vm, vm->getClassByName(as_name)) {}
    
    create_constructor(VoidObject      , "void"      );
    create_constructor(NamespaceObject , "Namespace" );
    create_constructor(MultinameObject , "Multiname" );
    create_constructor(ArrayObject     , "Array"     );
    create_constructor(StringObject    , "String"    );
    create_constructor(BooleanObject   , "Boolean"   );
 
#undef create_constructor
#define create_constructor(c_name, as_name) c_name::c_name(VM *vm, const typeof(c_name::value) &value) : value(value), \
NumberObject(vm, vm->getClassByName(as_name)) {}
    
    create_constructor(DoubleObject    , "Number"    );
    create_constructor(UIntObject      , "Number"    );
    create_constructor(IntObject       , "Number"    );
    
#undef create_constructor

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
#pragma mark - Object
    
    ObjectPtr Object::getSlot(u30_t slotIndex) const {
      if(slotMap.find(slotIndex) == slotMap.end()) return vm->undefinedObject;
      return slotMap.at(slotIndex);
    }
    
    ObjectPtr Object::getProperty(const Multiname &property) {
      if(hasDeclaredProperty(property)) {
        TraitMatch t = getTraitByName(property);
        switch(t.trait->kind) {
          case TraitInfo::GetterKind:
            return vm->runMethod(dynamic_cast<MethodTraitInfo *>(t.trait)->methodInfo, { shared_from_this() });
          case TraitInfo::SetterKind:
            throw "ReferenceError: Error #1077: Illegal read of write-only property {property name} on {class name}.";
          case TraitInfo::ConstKind:
          case TraitInfo::SlotKind:
            return getSlot(dynamic_cast<SlotTraitInfo *>(t.trait)->slotId);
          case TraitInfo::MethodKind:
            return std::make_shared<MethodObject>(vm, shared_from_this(), dynamic_cast<MethodTraitInfo *>(t.trait)->methodInfo);
          default:
            return NULL;
        }
      }
      
      std::string name = property.nameString();
      if(properties.find(name) != properties.end()) return properties.at(name);
        return NULL;
    }
    
    void Object::setProperty(const Multiname &property, const ObjectPtr &value) {
      if(hasDeclaredProperty(property)) {
        TraitMatch t = getTraitByName(property);
        switch(t.trait->kind) {
          case TraitInfo::SlotKind:
            setSlot(dynamic_cast<SlotTraitInfo *>(t.trait)->slotId, value);
            return;
          case TraitInfo::SetterKind:
            vm->runMethod(dynamic_cast<MethodTraitInfo *>(t.trait)->methodInfo, { shared_from_this(), value });
            return;
          case TraitInfo::GetterKind:
          case TraitInfo::ConstKind:
            throw "ReferenceError: Error #1074: Illegal write to read-only property {property name} on {class name}.";
          case TraitInfo::MethodKind:
            throw "ReferenceError: Error #1037: Cannot assign to a method {property name} on {class name}.";
          default:
            return;
        }
      }
      
      properties[property.nameString()] = value;
    }
    
    typedef double Number;
    typedef string String;
    
    Object::AcResult Object::abstractCompare(Object &rhs, bool leftFirst) {
      ObjectPtr px, py;
      if(leftFirst) {
        px = ecmaToPrimitive(ECMAHint::NumberHint);
        py = rhs.ecmaToPrimitive(ECMAHint::NumberHint);
      } else {
        py = rhs.ecmaToPrimitive(ECMAHint::NumberHint);
        px = ecmaToPrimitive(ECMAHint::NumberHint);
      }
      
      if(cast(String, px) && cast(String, py)) {
        String sx = cast(String, px)->value;
        String sy = cast(String, py)->value;
        
        return sx < sy ? Object::AcTrueResult : Object::AcFalseResult;
      } else {
        Number nx = px->ecmaToNumber();
        Number ny = py->ecmaToNumber();
        
        if(isnan(nx) || isnan(ny)) return Object::AcUndefinedResult;
        
        return nx < ny ? Object::AcTrueResult : Object::AcFalseResult;
      }
    }
    
#pragma mark - ecmaCall
    
    ObjectPtr FunctionObject::ecmaCall(VM &vm, std::vector<ObjectPtr> &args) const {
      return vm.runMethod(value, args);
    }
    
    ObjectPtr MethodObject::ecmaCall(VM &vm, std::vector<ObjectPtr> &args) const {
      args[0] = receiver;
      return vm.runMethod(value, args);
    }
    
#pragma mark - DisplayObject
    
    void DisplayObject::setProperty(const Multiname &property, const ObjectPtr &rhs) {
      Object::setProperty(property, rhs);
      
      std::string name = property.nameString();
      printf("Setting %s to %s\n", name.c_str(), rhs->coerce_s().c_str());
      
      if(name == "rotation") {
        rotation = rhs->coerce_d();
        double rad = rotation / 180 * M_PI;
        // x' = x * sx + y * r1
        // y' = y * sy + x * r0
        
        value->matrix.sx = cos(rad) * scaleX;
        value->matrix.sy = cos(rad) * scaleY;
        
        value->matrix.r1 = -sin(rad) * scaleX;
        value->matrix.r0 = +sin(rad) * scaleY;
      } else if(name == "scaleX") {
        double rad = rotation / 180 * M_PI;
        scaleX = rhs->coerce_d();
        value->matrix.sx = cos(rad) * scaleX;
        value->matrix.r1 = -sin(rad) * scaleX;
      } else if(name == "scaleY") {
        double rad = rotation / 180 * M_PI;
        scaleY = rhs->coerce_d();
        value->matrix.sy = cos(rad) * scaleY;
        value->matrix.r0 = +sin(rad) * scaleY;
      } else if(name == "x") {
        double v = rhs->coerce_d();
        value->matrix.tx = v * 20;
      } else if(name == "y") {
        double v = rhs->coerce_d();
        value->matrix.ty = v * 20;
      }
    }
  }
}