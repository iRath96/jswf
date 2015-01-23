//
//  Object.cpp
//  jswf
//
//  Created by Alexander Rath on 22.01.15.
//  Copyright (c) 2015 Alexander Rath. All rights reserved.
//

#include "Object.h"
#include "VM.h"

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

    /**
     * Creates a constructor for a subclass of NativeObject.
     * @param c_name [in] The name of the native class.
     * @param as_name [in] The ActionScript class name (with quotation marks).
     */
#define create_constructor(c_name, as_name) c_name::c_name(VM *vm, const typeof(c_name::value) &value) : value(value), \
    NativeObject(vm, vm->getClassByName(as_name)) {}

    create_constructor(VoidObject      , "void"      );
    create_constructor(NamespaceObject , "Namespace" );
    create_constructor(MultinameObject , "Multiname" );
    create_constructor(ArrayObject     , "Array"     );
    create_constructor(StringObject    , "String"    );
    create_constructor(BooleanObject   , "Boolean"   );

    template<typename T>
    NumberObject<T>::NumberObject(VM *vm, const T &value) : value(value), NumberBaseObject(vm, vm->getClassByName("Number")) {}

#undef create_constructor

#pragma mark - Object

    std::shared_ptr<const Object> Object::ecmaToPrimitive(ECMAHint::Enum hint) const {
      // "Return a default value for the Object. The default value of an object is retrieved by calling the [[DefaultValue]] internal method of the object, passing the optional hint PreferredType. The behaviour of the [[DefaultValue]] internal method is defined by this specification for all native ECMAScript objects in 8.12.8."
      
      if(hint == ECMAHint::StringHint)
        return std::make_shared<StringObject>(vm, "[object " + klass->iinfo.name->nameString() + "]");
      
      throw "TypeError.";
    }

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

    bool Object::operator ==(const Object &rhs) const { // 'Abstract Equality Comparison', ECMA-262, section 11.9.3
      if(klass == rhs.klass) return sameTypeEquals(rhs);
      // Notice: null and undefined share the same class (VoidObject, "void") in this implementation
      
      // 4. and 5.
      if((dynamic_cast<const NumberBaseObject *>(this) && dynamic_cast<const StringObject *>(&rhs)) ||
         (dynamic_cast<const NumberBaseObject *>(&rhs) && dynamic_cast<const StringObject *>(this))) {
        double a = coerce_d();
        double b = rhs.coerce_d();
        return !isnan(a) && !isnan(b) && a == b;
      }
      
      // 6.
      if(dynamic_cast<const BooleanObject *>(this)) {
        NumberObject<double> number(vm, this->coerce_d());
        return number == rhs;
      }
      
      // 7.
      if(dynamic_cast<const BooleanObject *>(&rhs)) {
        NumberObject<double> number(vm, rhs.coerce_d());
        return *this == number;
      }
      
      // 8.
      if((dynamic_cast<const NumberBaseObject *>(this) || dynamic_cast<const StringObject *>(this)) && !dynamic_cast<const NativeObject *>(&rhs))
        return *this == *rhs.ecmaToPrimitive();
      
      // 9.
      if((dynamic_cast<const NumberBaseObject *>(&rhs) || dynamic_cast<const StringObject *>(&rhs)) && !dynamic_cast<const NativeObject *>(this))
        return *this->ecmaToPrimitive() == rhs;
      
      return false;
    }

    Object::AcResult Object::ecmaAbstractCompare(const Object &lhs, const Object &rhs, bool leftFirst) {
      std::shared_ptr<const Object> px, py;
      if(leftFirst) {
        px = lhs.ecmaToPrimitive(ECMAHint::NumberHint);
        py = rhs.ecmaToPrimitive(ECMAHint::NumberHint);
      } else {
        py = rhs.ecmaToPrimitive(ECMAHint::NumberHint);
        px = lhs.ecmaToPrimitive(ECMAHint::NumberHint);
      }
      
      if(cast(const String, px) && cast(const String, py)) {
        String sx = cast(const String, px)->value;
        String sy = cast(const String, py)->value;
        
        return sx < sy ? Object::AcTrueResult : Object::AcFalseResult;
      } else {
        // Convert to Numbers
        
        Number nx = px->coerce_d();
        Number ny = py->coerce_d();
        
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