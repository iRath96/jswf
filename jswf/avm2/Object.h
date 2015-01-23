//
//  Object.h
//  jswf
//
//  Created by Alexander Rath on 22.01.15.
//  Copyright (c) 2015 Alexander Rath. All rights reserved.
//

#ifndef __jswf__Object__
#define __jswf__Object__

#include <string>
#include <map>

#include "Opcode.h"
#include "ABCFile.h"
#include "TraitInfo.h"
#include "StringReader.h"
#include "Frame.h"

namespace jswf {
  namespace avm2 {
#define cast(name, obj) dynamic_cast<name ## Object *>(obj.get())
    
    class Object;
    typedef std::shared_ptr<Object> ObjectPtr;
    
    typedef std::map<u30_t, ObjectPtr> SlotMap;
    
    struct TraitMatch {
      bool isStatic;
      TraitMap *dataStore;
      TraitInfo *trait;
    };
    
    struct ECMAHint {
      enum Enum {
        NoHint = 0,
        NumberHint,
        StringHint
      };
    };
    
    class Object : public std::enable_shared_from_this<Object> {
    public:
      Object(VM *vm, Class *klass) : vm(vm), klass(klass) {}
      
      VM *vm;
      Class *klass;
      
      std::map<std::string, ObjectPtr> properties;
      // std::map<u30_t, ObjectPtr> slots;
      TraitMap traitMap;
      SlotMap slotMap;
      
      uint16_t coerce_u16() const { return coerce_d(); }
      uint32_t coerce_u32() const { return coerce_d(); }
      int32_t  coerce_s32() const { return coerce_d(); }
      
      // TODO:2015-01-04:alex:Initialize slots!
      // TODO:2015-01-01:alex:Property chain!
      // TODO:2015-01-04:alex:Slots of parent classes. Ouch.
      
      TraitMatch getTraitByName(const Multiname &name) {
        Class *k = klass;
        while(k != NULL) {
          // Search instance traits.
          for(auto it = k->iinfo.traits.begin(); it != k->iinfo.traits.end(); ++it)
            if(*(*it)->name == name)
              return { .isStatic = false, .trait = it->get(), .dataStore = &traitMap };
          
          // Search class traits.
          for(auto it = k->cinfo.traits.begin(); it != k->cinfo.traits.end(); ++it)
            if(*(*it)->name == name)
              return { .isStatic = true, .trait = it->get(), .dataStore = &klass->traitMap };
          
          k = k->parent;
        } return { .trait = NULL, .dataStore = NULL };
      }
      
      TraitMatch getTraitBySlotId(u30_t slotId) {
        Class *k = klass;
        while(k != NULL) {
          // Search instance traits.
          for(auto it = k->iinfo.traits.begin(); it != k->iinfo.traits.end(); ++it)
            if(dynamic_cast<SlotTraitInfo *>(it->get()) && dynamic_cast<SlotTraitInfo *>(it->get())->slotId == slotId)
              return { .isStatic = false, .trait = it->get(), .dataStore = &traitMap };
          
          // Search class traits.
          for(auto it = k->cinfo.traits.begin(); it != k->cinfo.traits.end(); ++it)
            if(dynamic_cast<SlotTraitInfo *>(it->get()) && dynamic_cast<SlotTraitInfo *>(it->get())->slotId == slotId)
              return { .isStatic = true, .trait = it->get(), .dataStore = &klass->traitMap };
          
          k = k->parent;
        } return { .trait = NULL, .dataStore = NULL };
      }
      
      virtual ObjectPtr getProperty(const Multiname &property);
      virtual void setProperty(const Multiname &property, const ObjectPtr &value);
      
      void setSlot(u30_t slotIndex, const ObjectPtr &value) {
        // TODO:2015-01-04:alex:Check that the type is correct.
        slotMap[slotIndex] = value;
      }
      
      ObjectPtr getSlot(u30_t slotIndex) const;
      
      virtual ObjectPtr ecmaCall(VM &vm, std::vector<ObjectPtr> &args) const {
        throw "TypeError: Error #1006: value is not a function.";
      }
      
      // ecmaConstruct
      // ecmaDescendants
      // ecmaHasProperty
      
      // ecmaGetOwnProperty
      // ecmaGetProperty
      // ecmaGet
      // ecmaCanPut
      // ecmaPut
      // ecmaHasProperty
      // ecmaDelete
      // ecmaDefaultValue
      // ecmaDefineOwnProperty
      
      // ecmaCall
      // ecmaConstruct
      // ecmaThrowTypeError
      
      // ecmaClass
      // ecmaPrototype
      // ecmaCall
      // ecmaScope
      
      bool hasDeclaredProperty(const Multiname &property) {
        return getTraitByName(property).trait != NULL;
      }
      
      bool hasDynamicProperty(const Multiname &property) {
        std::string name = property.nameString();
        return properties.find(name) != properties.end();
      }
      
      bool hasProperty(const Multiname &property) {
        return hasDeclaredProperty(property) || hasDynamicProperty(property);
      }
      
      virtual std::string toString() const {
        return "[object " + klass->iinfo.name->nameString() + "];";
      }
      
      virtual bool sameTypeEquals(const Object &rhs) const { return this == &rhs; }
      virtual bool operator ==(const Object &rhs) const; // 'Abstract Equality Comparison', ECMA-262, section 11.9.3
      virtual bool operator !=(const Object &rhs) const { return !(*this == rhs); }
      
      enum AcResult {
        AcUndefinedResult = 0,
        AcFalseResult = 1,
        AcTrueResult = 2
      };
      
      static AcResult ecmaAbstractCompare(const Object &lhs, const Object &rhs, bool leftFirst = true); // ECMA-262, section 11.8.5
      
      virtual bool operator < (const Object &rhs) const { return Object::ecmaAbstractCompare(*this, rhs, true ) == AcTrueResult ; }
      virtual bool operator > (const Object &rhs) const { return Object::ecmaAbstractCompare(rhs, *this, false) == AcTrueResult ; }
      virtual bool operator <=(const Object &rhs) const { return Object::ecmaAbstractCompare(rhs, *this, false) == AcFalseResult; }
      virtual bool operator >=(const Object &rhs) const { return Object::ecmaAbstractCompare(*this, rhs, true ) == AcFalseResult; }
      
      virtual bool sameTypeStrictEquals(const Object &rhs) const { return this == &rhs; }
      virtual bool strictEquals(const Object &rhs) const { // 'Strict Equality Comparison', ECMA-262, Section 11.9.6
        if(klass == rhs.klass) return sameTypeStrictEquals(rhs);
        return this == &rhs;
      }
      
      virtual std::shared_ptr<const Object> ecmaToPrimitive(ECMAHint::Enum hint = ECMAHint::NoHint) const;
      
      virtual ObjectPtr ecmaToPrimitive(ECMAHint::Enum hint = ECMAHint::NoHint) {
        return std::const_pointer_cast<Object>(static_cast<const Object *>(this)->ecmaToPrimitive(hint));
      }
      
      virtual bool isNaN() const { return false; }
      
      virtual bool coerce_b() const { return true; } // [[ToBoolean]], ECMA-262, section 9.2
      //virtual s32_t coerce_i() const { // [[ToInteger]], ECMA-262, section 9.4
      //  double number = coerce_d();
      //  if(isnan(number)) return +0;
      //  return number;
      //}
      
      virtual std::string coerce_s() const {
        std::shared_ptr<const Object> primitive = ecmaToPrimitive(ECMAHint::StringHint);
        return primitive->coerce_s();
      }
      
      virtual Namespace coerce_ns() const { throw "Cannot coerce_ns"; }
      virtual Multiname coerce_multiname() const { throw "Cannot coerce_multiname"; }
      virtual double coerce_d() const { // [[ToNumber]], ECMA-262, section 9.3
        std::shared_ptr<const Object> primitive = ecmaToPrimitive(ECMAHint::NumberHint);
        return primitive->coerce_d();
      }
      
      virtual ObjectPtr coerce(Class *newKlass) {
        // TODO:2015-01-01:alex:Implement this!
        klass = newKlass; // <tt>coerce Function</tt> will actually make a <tt>builtin.as$0::MethodClosure</tt> a <tt>Function</tt>.
        return shared_from_this();
      }
      
      std::string getPropertyName(int index) const {
        if(properties.size() == 0) return "";
        
        auto it = properties.begin();
        --index;
        while(index) {
          if(++it == properties.end()) return "";
          --index;
        } return it->first;
      }
      
      bool hasNextProperty(int index) const {
        if(properties.size() == 0) return false;
        
        auto it = properties.begin();
        while(index) {
          if(++it == properties.end()) return false;
          --index;
        } return true;
      }
    };
    
    builtin_method_t builtin_trace;
    builtin_method_t builtin_getQualifiedClassName;
    builtin_method_t builtin_addEventListener;
    
    /**
     * Represents a <tt>function() {}</tt> that was created using <tt>newfunction</tt>.
     */
    class FunctionObject : public Object {
    public:
      // TODO:2015-01-04:alex:Saved scope.
      
      MethodInfo *value;
      FunctionObject(VM *vm, MethodInfo *value);
      
      std::string coerce_s() const { return "function Function() {}"; }
      
      ObjectPtr ecmaCall(VM &vm, std::vector<ObjectPtr> &args) const;
    };
    
    /**
     * Represents a method that was created as a trait of an instance or a class.
     * Note that executing <tt>[[Call]]</tt> on a MethodObject will always override the
     * implicit (receiver) argument.
     */
    class MethodObject : public Object {
    public:
      ObjectPtr receiver;
      
      MethodInfo *value;
      MethodObject(VM *vm, const ObjectPtr &recv, MethodInfo *value);
      
      std::string coerce_s() const { return "function Function() {}"; }
      
      ObjectPtr ecmaCall(VM &vm, std::vector<ObjectPtr> &args) const;
    };
    
    /**
     * Represents a method that is provided by the runtime.
     */
    class BuiltinMethodObject : public Object {
    public:
      builtin_method_t *value;
      BuiltinMethodObject(VM *vm, builtin_method_t *value);
      
      std::string coerce_s() const { return "function Function() {}"; }
      
      ObjectPtr ecmaCall(VM &vm, std::vector<ObjectPtr> &args) const {
        return value(vm, NULL, args);
      }
    };
    
    /**
     * Represents a class.
     */
    class ClassObject : public Object {
    public:
      Class *value;
      ClassObject(VM *vm, Class *value);
      
      std::string coerce_s() const {
        return "[class " + value->iinfo.name->nameString() + "]";
      }
    };
    
    class DisplayObject : public Object {
    public:
      double rotation = 1, scaleX = 1, scaleY = 1;
      
      flash::DisplayListEntry *value;
      DisplayObject(VM *vm, Class *klass, flash::DisplayListEntry *value) : value(value), Object(vm, klass) {}
      
      void setProperty(const Multiname &property, const ObjectPtr &value);
    };
    
    class NativeObject : public Object {
    public:
      NativeObject(VM *vm, Class *klass) : Object(vm, klass) {}
      virtual std::shared_ptr<const Object> ecmaToPrimitive(ECMAHint::Enum hint = ECMAHint::NoHint) const {
        return shared_from_this();
      }
    };
    
    class VoidObject : public NativeObject {
    public:
      enum Kind {
        NullValue = 0,
        UndefinedValue
      };
      
      Kind value;
      
      VoidObject(VM *vm, const Kind &value);
      bool operator == (const Object &rhs) const { return dynamic_cast<const VoidObject *>(&rhs); }
      bool sameTypeStrictEquals(const Object &rhs) const { return value == static_cast<const VoidObject *>(&rhs)->value; }
      
      bool        coerce_b() const { return false; }
      std::string coerce_s() const { return value == NullValue ? "null" : "undefined"; }
      double      coerce_d() const { return value == NullValue ? +0 : NAN; }
    };
    
#define make_native_class(name, parent_class, value_type, coerce_name, code) \
class name : public parent_class { \
public: \
value_type value; \
name(VM *vm, const value_type &value); \
value_type coerce_name() const { return value; } \
code \
};
    
    make_native_class(NamespaceObject, NativeObject, Namespace, coerce_ns,);
    make_native_class(MultinameObject, NativeObject, Multiname, coerce_multiname,);
    make_native_class(ArrayObject, NativeObject, std::vector<ObjectPtr>, coerce_a,);
    make_native_class
    (StringObject, NativeObject, std::string, coerce_s,
     bool coerce_b() const { return !value.empty(); }
     
     bool sameTypeEquals(const Object &rhs) const { return value == static_cast<const StringObject *>(&rhs)->value; }
     bool sameTypeStrictEquals(const Object &rhs) const { return value == static_cast<const StringObject *>(&rhs)->value; }
    // TODO:2015-01-18:alex:Implement ECMA-262 Section 9.3.1
    );
    make_native_class
    (BooleanObject, NativeObject, bool, coerce_b,
     std::string coerce_s() const { return value ? "true" : "false"; }
     double      coerce_d() const { return value ? 1 : +0; }
     
     bool sameTypeEquals(const Object &rhs) const { return value == static_cast<const BooleanObject *>(&rhs)->value; }
     bool sameTypeStrictEquals(const Object &rhs) const { return value == static_cast<const BooleanObject *>(&rhs)->value; }
     );
    
    class NumberBaseObject : public NativeObject {
    public:
      NumberBaseObject(VM *vm, Class *klass) : NativeObject(vm, klass) {}
    };
    
    template<typename T>
    class NumberObject : public NumberBaseObject {
    public:
      T value;
      
      NumberObject(VM *vm, const T &value);
      
      bool        coerce_b() const { return !isnan(value) && value != 0; }
      std::string coerce_s() const { return std::to_string(value); }
      double      coerce_d() const { return value; }
      
      bool sameTypeEquals(const Object &rhs) const {
        double a = value;
        double b = rhs.coerce_d();
        return !isnan(a) && !isnan(b) && a == b;
      }
      
      bool sameTypeStrictEquals(const Object &rhs) const {
        double a = value;
        double b = rhs.coerce_d();
        return !isnan(a) && !isnan(b) && a == b;
      }
    };
  }
}

#endif /* defined(__jswf__Object__) */