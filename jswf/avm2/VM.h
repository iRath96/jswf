//
//  VM.h
//  jswf
//
//  Created by Alexander Rath on 28.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__VM__
#define __jswf__VM__

#include <string>
#include <map>

#include "ABCFile.h"
#include "TraitInfo.h"
#include "StringReader.h"

namespace jswf {
  namespace flash {
    struct DisplayListEntry;
  }
  
  namespace avm2 {
    class Object;
    typedef std::shared_ptr<Object> ObjectPtr;
    
    class Object {
    public:
      Object(Class *klass) : klass(klass) {}
      
      Class *klass;
      std::map<std::string, ObjectPtr> properties;
      std::map<u30_t, ObjectPtr> slots;
      
      // TODO:2015-01-04:alex:Initialize slots!
      // TODO:2015-01-01:alex:Property chain!
      // TODO:2015-01-04:alex:Slots of parent classes. Ouch.
      
      virtual ObjectPtr getProperty(const Multiname &property) const {
        if(hasDeclaredProperty(property))
          return getSlot(getSlotByName(property)); // TODO:2015-01-04:alex:This will fail for non-slot traits.
        
        std::string name = property.nameString();
        if(properties.find(name) != properties.end()) return properties.at(name);
        return NULL;
      }
      
      u30_t getSlotByName(const Multiname &name) const {
        for(auto it = klass->iinfo.traits.begin(); it != klass->iinfo.traits.end(); ++it)
          if(*(*it)->name == name && dynamic_cast<SlotTraitInfo *>(it->get()))
            return dynamic_cast<SlotTraitInfo *>(it->get())->slotId;
        throw "No such slot.";
      }
      
      virtual void setProperty(const Multiname &property, const ObjectPtr &value) {
        if(hasDeclaredProperty(property)) {
          setSlot(getSlotByName(property), value);
          return;
        }
        
        properties[property.nameString()] = value;
      }
      
      void setSlot(u30_t slotIndex, const ObjectPtr &value) {
        // TODO:2015-01-04:alex:Check that the type is correct.
        slots[slotIndex] = value;
      }
      
      ObjectPtr getSlot(u30_t slotIndex) const {
        if(slots.find(slotIndex) == slots.end()) return NULL;
        return slots.at(slotIndex);
      }
      
      virtual ObjectPtr ecmaCall(VM &vm, std::vector<ObjectPtr> &args) const {
        throw "Could not ecmaCall";
      }
      
      bool hasDeclaredProperty(const Multiname &property) const {
        for(auto it = klass->iinfo.traits.begin(); it != klass->iinfo.traits.end(); ++it)
          if(*(*it)->name == property) return true;
        return false;
      }
      
      bool hasDynamicProperty(const Multiname &property) const {
        std::string name = property.nameString();
        return properties.find(name) != properties.end();
      }
      
      bool hasProperty(const Multiname &property) const {
        return hasDeclaredProperty(property) || hasDynamicProperty(property);
      }
      
      virtual std::string toString() const {
        return "[object " + klass->iinfo.name->nameString() + "];";
      }
      
      virtual bool operator ==(const Object &rhs) const { // ECMA-262, section 11.9.3
        if(klass == rhs.klass && this == &rhs) return true;
        return false;
      }
      
      virtual bool coerce_b() const { return true; } // ECMA-262, section 9.2
      virtual s32_t coerce_i() const { throw "Cannot coerce_i"; }
      virtual std::string coerce_s() const { return "[object " + klass->iinfo.name->nameString() + "]"; }
      virtual Namespace coerce_ns() const { throw "Cannot coerce_ns"; }
      virtual Multiname coerce_multiname() const { throw "Cannot coerce_multiname"; }
      virtual double coerce_d() const { throw "Cannot coerce_d"; }
      
      virtual ObjectPtr coerce(Class *newKlass) const {
        // TODO:2015-01-01:alex:Implement this!
        return std::make_shared<Object>(newKlass);
      }
      
      std::string getPropertyName(int index) const {
        auto it = properties.begin();
        --index;
        while(index) {
          if(++it == properties.end()) return "";
          --index;
        } return it->first;
      }
      
      bool hasNextProperty(int index) const {
        auto it = properties.begin();
        while(index) {
          if(++it == properties.end()) return false;
          --index;
        } return true;
      }
    };
    
    typedef ObjectPtr builtin_method_t(VM &, MethodInfo *, std::vector<ObjectPtr> &);
    
    builtin_method_t builtin_trace;
    builtin_method_t builtin_getQualifiedClassName;
    
    class BuiltinMethodObject : public Object {
    public:
      builtin_method_t *value;
      BuiltinMethodObject(Class *klass, builtin_method_t *value) : value(value), Object(klass) {}
      
      std::string coerce_s() const { return "function Function() {}"; }
      
      ObjectPtr ecmaCall(VM &vm, std::vector<ObjectPtr> &args) const {
        return value(vm, NULL, args);
      }
    };
    
    class ClassObject : public Object {
    public:
      Class *value;
      ClassObject(Class *klass, Class *value) : value(value), Object(klass) {}
      
      std::string coerce_s() const {
        return "[class " + value->iinfo.name->nameString() + "]";
      }
    };
    
    class DisplayObject : public Object {
    public:
      flash::DisplayListEntry *entry;
      DisplayObject(Class *klass, flash::DisplayListEntry *entry) : entry(entry), Object(klass) {}
      
      void setProperty(const Multiname &property, const ObjectPtr &value);
    };
    
    class NativeObject : public Object {
    public:
      NativeObject(Class *klass) : Object(klass) {}
    };
    
    class VoidObject : public NativeObject { // TODO:2015-01-01:alex:Differentiate null and undefined.
    public:
      VoidObject(Class *klass) : NativeObject(klass) {}
      bool operator ==(const Object &rhs) {
        return dynamic_cast<const VoidObject *>(&rhs);
      }
      
      bool coerce_b() const { return false; }
      std::string coerce_s() const { return "undefined"; }
      double coerce_d() const { return NAN; } // TODO:2015-01-01:alex:+0 for null.
    };
    
#define make_native_class(name, value_type, coerce_name, code) \
  class name : public NativeObject { \
  public: \
    value_type value; \
    name(Class *klass, const value_type &value) : value(value), NativeObject(klass) {} \
    bool operator ==(const Object &rhs) const { \
      const name *casted = dynamic_cast<const name *>(&rhs); \
      return casted && value == casted->value; \
    } \
    value_type coerce_name() const { return value; } \
    code \
  };
    
    make_native_class(NamespaceObject, Namespace, coerce_ns,);
    make_native_class(MultinameObject, Multiname, coerce_multiname,);
    make_native_class(ArrayObject, std::vector<ObjectPtr>, coerce_a,);
    make_native_class
     (NumberObject, double, coerce_d,
      bool coerce_b() const { return !isnan(value) && value != 0; }
    );
    make_native_class
     (StringObject, std::string, coerce_s,
      bool coerce_b() const { return !value.empty(); }
    );
    make_native_class
     (BooleanObject, bool, coerce_b,
      std::string coerce_s() const { return value ? "true" : "false"; }
      double coerce_d() const { return value ? 1 : +0; }
    );
    make_native_class // TODO:2015-01-04:alex:Not DRY with IntObject
     (UIntObject, u32_t, coerce_u,
      std::string coerce_s() const { return std::to_string(value); }
      bool coerce_b() const { return value != 0; }
      double coerce_d() const { return value; }
    );
    make_native_class
     (IntObject, s32_t, coerce_i,
      std::string coerce_s() const { return std::to_string(value); }
      bool coerce_b() const { return value != 0; }
      double coerce_d() const { return value; }
    );
    
    class Scope {
    public:
      enum Kind {
        NormalScopeKind = 0,
        WithScopeKind,
        GlobalScopeKind
      };
      
      ObjectPtr object;
      Kind kind;
      
      Scope() : object(NULL) {}
      Scope(const ObjectPtr &object, Kind kind) : object(object), kind(kind) {}
      
      bool hasProperty(const Multiname &property) const {
        if(object->hasDeclaredProperty(property)) return true;
        if(kind == NormalScopeKind) return false;
        return object->hasDynamicProperty(property);
      }
    };
    
    class VM {
    public:
      std::map<std::string, Class *> classes; // TODO:2014-12-29:alex:Could be QName indexed.
      std::vector<std::shared_ptr<ABCFile>> files;
      
      ObjectPtr globalObject;
      ObjectPtr undefinedObject;
      
      VM() {
        std::shared_ptr<ABCFile> api = std::make_shared<ABCFile>();
        
        Class *parent = NULL, *object = NULL;
        parent = object = api->makeClass(api->makeQName("", "Object"), parent);
        parent = api->makeClass(api->makeQName("flash.events", "EventDispatcher"), parent);
        parent = api->makeClass(api->makeQName("flash.display", "DisplayObject"), parent);
        parent = api->makeClass(api->makeQName("flash.displa", "InteractiveObject"), parent);
        parent = api->makeClass(api->makeQName("flash.display", "DisplayObjectContainer"), parent);
        parent = api->makeClass(api->makeQName("flash.display", "Sprite"), parent);
        parent = api->makeClass(api->makeQName("flash.display", "MovieClip"), parent);
        
        api->makeClass(api->makeQName("", "void"), object);
        api->makeClass(api->makeQName("", "null"), object);
        
        api->makeClass(api->makeQName("", "Boolean"), object);
        api->makeClass(api->makeQName("", "Number"), object);
        api->makeClass(api->makeQName("", "int"), object);
        api->makeClass(api->makeQName("", "uint"), object);
        api->makeClass(api->makeQName("", "Function"), object);
        api->makeClass(api->makeQName("", "String"), object);
        
        Class *classClass = api->makeClass(api->makeQName("", "Class"), object);
        ObjectPtr objectClass = std::make_shared<ClassObject>(classClass, object);
        
        Class *builtinMethod = api->makeClass(api->makeQName("", "builtin.as$0::MethodClosure"), NULL);
        ObjectPtr traceMethod = std::make_shared<BuiltinMethodObject>(builtinMethod, &builtin_trace);
        ObjectPtr gQCNMethod = std::make_shared<BuiltinMethodObject>(builtinMethod, &builtin_getQualifiedClassName);
        
        Class *globalClass = api->makeClass(api->makeQName("", "Global"), object);
        globalObject = std::make_shared<Object>(globalClass);
        
        globalObject->setProperty(*api->makeQName("", "trace"), traceMethod);
        globalObject->setProperty(*api->makeQName("flash.utils", "getQualifiedClassName"), gQCNMethod);
        
        globalObject->setProperty(*api->makeQName("", "Object"), objectClass);
        
        loadABCFile(api);
        
        undefinedObject = ObjectPtr(makeVoidObject());
      }
      
#define maker(klass, flashKlass) \
  klass *make ## klass(typeof(IntObject::value) value) { return new klass(getClassByName(flashKlass), value); }
      maker(IntObject    , "int");
      maker(UIntObject   , "uint");
      maker(BooleanObject, "Boolean");
      maker(NumberObject , "Number");
      
      StringObject *makeStringObject(std::string value) { return new StringObject(getClassByName("String"), value); }
      ArrayObject *makeArrayObject(std::vector<ObjectPtr> value) { return new ArrayObject(getClassByName("Array"), value); }
      VoidObject *makeVoidObject() { return new VoidObject(getClassByName("void")); }
      
      void loadABCFile(std::shared_ptr<ABCFile> file) {
        files.push_back(file);
        
        for(size_t i = 0, j = file->classes.size(); i < j; ++i) {
          Class &klass = *file->classes[i].get();
          klass.vm = this;
          
          MultinamePtr &name = klass.iinfo.name;
          classes[name->nameString()] = &klass;
          
          if(klass.cinfo.initializer->body) {
            ObjectPtr arg0 = std::make_shared<Object>(getClassByName("Class"));
            runMethod(klass.cinfo.initializer, std::vector<ObjectPtr>{ arg0 });
          }
        }
      }
      
      Class *getClassByName(std::string name) {
        return classes.at(name);
      }
      
      ObjectPtr instantiateClass(std::string klassName) {
        return instantiateClass(getClassByName(klassName));
      }
      
      ObjectPtr instantiateClass(Class *klass) {
        printf("Should create an instance of %s.\n", klass->iinfo.name->nameString().c_str());
        
        ObjectPtr obj = std::make_shared<Object>(klass);
        runMethod(klass->iinfo.initializer, std::vector<ObjectPtr>{ obj });
        
        return obj;
      }
      
      ObjectPtr instantiateDisplayClass(Class *klass, flash::DisplayListEntry *entry) {
        printf("Should create a display instance of %s.\n", klass->iinfo.name->nameString().c_str());
        
        ObjectPtr obj = std::make_shared<DisplayObject>(klass, entry);
        runMethod(klass->iinfo.initializer, std::vector<ObjectPtr>{ obj });
        
        return obj;
      }
      
      ObjectPtr runMethod(MethodInfo *method, std::vector<ObjectPtr> arguments) {
        ABCFile *file = method->file;
        
        printf("runMethod\n");
        
        MethodBody *body = method->body;
        if(body == NULL) {
          printf("  native fallback\n");
          // TODO:2015-01-01:alex:Do something.
          return std::make_shared<Object>(getClassByName("Object"));
        } else {
          printf("  VM stuff.");
          printf("  code = %s\n", body->code.c_str());
          
          std::vector<Scope> scopeStack; // capacity should be method->body->maxScopeDepth
          std::stack<ObjectPtr> stack; // capacity should be method->body->maxStack
          std::map<u30_t, ObjectPtr> registers; // capacity should be method->body->localCount
          
          Scope globalScope(globalObject, Scope::GlobalScopeKind);
          scopeStack.push_back(globalScope);
          
          for(size_t i = 0, j = arguments.size(); i < j; ++i)
            registers[(u30_t)i] = arguments[i];
          
          bool show_disassembly = 0;
          
          io::StringReader codeReader(method->body->code);
#define read_auto(afield, vname) auto vname = file->constantPool.afield[codeReader.readVU30()];
#define read_multiname_arg(index) \
  Multiname multiname = *file->constantPool.multinames[index]; \
  if(!multiname.hasName) multiname.setName(__pop->coerce_s()); \
  if(!multiname.hasNS && !multiname.hasNSSet) multiname.setNS(std::make_shared<Namespace>(__pop->coerce_ns()));
#define read_multiname read_multiname_arg(codeReader.readVU30())
#define mn_cstr multiname.nameString().c_str()
#define mn_str multiname.nameString()
#define bc_comment(v) if(show_disassembly) printf("[vm 0x%04lx] %s\n", pos, std::string(v).c_str());
#define read_diadic \
  ObjectPtr v2 = __pop; \
  ObjectPtr v1 = __pop;
#define cast(name, obj) dynamic_cast<name ## Object *>(obj.get())

#define __ncast(v) ObjectPtr(v)
#define __push(v) stack.push(__ncast(v))
#define __pop (top = stack.top(), stack.pop(), top)
          ObjectPtr top;
          
          while(!codeReader.eof()) {
            size_t pos = codeReader.pos;
            uint8_t chr = codeReader.readU8();
            switch(chr) {
              case 0x08: {
                u30_t i = codeReader.readVU30();
                registers.erase(i);
                bc_comment("kill " + std::to_string(i));
              }; break;
              case 0x09: {
                bc_comment("label");
                // "Do nothing"
              }; break;
              case 0x10: {
                s24_t offset = codeReader.readS24();
                bc_comment("jump " + std::to_string(offset));
                codeReader.seek(offset);
              }; break;
              case 0x11: {
                s24_t offset = codeReader.readS24();
                bc_comment("iftrue " + std::to_string(offset));
                
                bool shouldBranch = __pop->coerce_b();
                if(shouldBranch) codeReader.seek(offset);
              }; break;
              case 0x12: {
                s24_t offset = codeReader.readS24();
                bc_comment("iffalse " + std::to_string(offset));
                
                bool shouldBranch = !__pop->coerce_b();
                if(shouldBranch) codeReader.seek(offset);
              }; break;
              case 0x1c: {
                bc_comment("pushwith");
                // TODO:2015-01-04:alex:See below:
                // "A TypeError is thrown if scope_obj is null or undefined."
                
                Scope scope(__pop, Scope::WithScopeKind);
                scopeStack.push_back(scope);
              }; break;
              case 0x1d: {
                bc_comment("popscope");
                scopeStack.pop_back();
              }; break;
              case 0x1e: {
                bc_comment("nextname");
                ObjectPtr index = __pop;
                ObjectPtr obj = __pop;
                __push(makeStringObject(obj->getPropertyName(index->coerce_i())));
              }; break;
              case 0x20: {
                bc_comment("pushnull");
                __push(undefinedObject); // TODO:2015-01-04:alex:Undefined!
              }; break;
              case 0x21: {
                bc_comment("pushundefined");
                __push(undefinedObject); // undefined!
              }; break;
              case 0x24: {
                uint8_t byte = codeReader.readU8();
                bc_comment("pushbyte " + std::to_string(byte));
                
                __push(makeIntObject(byte));
              }; break;
              case 0x26: {
                bc_comment("pushtrue");
                __push(makeBooleanObject(true));
              }; break;
              case 0x27: {
                bc_comment("pushfalse");
                __push(makeBooleanObject(false));
              }; break;
              case 0x2a: {
                bc_comment("dup");
                
                ObjectPtr v = __pop;
                stack.push(v);
                stack.push(v);
              }; break;
              case 0x2c: {
                read_auto(strings, str);
                bc_comment("pushstring \"" + str + "\""); // TODO:2014-12-28:alex:Not correctly escaped.
                
                __push(makeStringObject(str));
              }; break;
              case 0x2d: {
                read_auto(integers, i);
                bc_comment("pushint " + std::to_string(i));
                
                __push(makeIntObject(i));
              }; break;
              case 0x2f: {
                read_auto(doubles, dbl);
                bc_comment("pushdouble " + std::to_string(dbl));
                
                __push(makeNumberObject(dbl));
              }; break;
              case 0x30: {
                bc_comment("pushscope");
                
                Scope scope(__pop, Scope::NormalScopeKind);
                scopeStack.push_back(scope);
              }; break;
              case 0x32: {
                u30_t objReg = codeReader.readVU30();
                u30_t indexReg = codeReader.readVU30();
                
                bc_comment("hasnext2 " + std::to_string(objReg) + " " + std::to_string(indexReg));
                
                u32_t i = registers[indexReg]->coerce_i();
                
                ObjectPtr obj = registers[objReg];
                bool hasNext = obj->hasNextProperty(i);
                __push(makeBooleanObject(hasNext));
                
                if(hasNext) {
                  registers[indexReg] = ObjectPtr(makeIntObject(i + 1));
                } else {
                  registers[objReg] = ObjectPtr(makeVoidObject()); // TODO:2015-01-01:alex:Automatically wrap into ObjectPtr?
                  registers[indexReg] = ObjectPtr(makeIntObject(0));
                }
              }; break;
              case 0x47: {
                bc_comment("returnvoid");
                return ObjectPtr(makeVoidObject());
              }; break;
              case 0x48: {
                bc_comment("returnvalue");
                return __pop;
              }; break;
              case 0x49: {
                u30_t argCount = codeReader.readVU30();
                bc_comment("constructsuper[" + std::to_string(argCount) + "]");
                
                std::vector<ObjectPtr> args;
                for(uint32_t i = 0; i < argCount; ++i) args.insert(args.begin(), __pop);
                
                bc_comment("[ TODO ]");
              }; break;
              case 0x46:
              case 0x4f: {
                u30_t mnIndex = codeReader.readVU30();
                u30_t argCount = codeReader.readVU30();
                
                std::vector<ObjectPtr> args;
                for(uint32_t i = 0; i < argCount; ++i) args.insert(args.begin(), __pop);
                
                read_multiname_arg(mnIndex);
                
                bc_comment((chr == 0x46 ? "callproperty[" : "callpropvoid[") + std::to_string(argCount) + "] " + mn_str);
                ObjectPtr method = __pop->getProperty(multiname);
                ObjectPtr ret = method->ecmaCall(*this, args);
                
                if(chr == 0x46) __push(ret);
              }; break;
              case 0x55: {
                u30_t argCount = codeReader.readVU30();
                bc_comment("newobject[" + std::to_string(argCount) + "]");
                
                ObjectPtr obj(instantiateClass("Object"));
                
                std::vector<std::pair<ObjectPtr, ObjectPtr>> args;
                for(uint32_t i = 0; i < argCount; ++i) {
                  auto value = __pop;
                  auto name = __pop;
                  obj->setProperty(name->coerce_multiname(), value);
                }
                
                stack.push(obj);
              }; break;
              case 0x56: {
                u30_t argCount = codeReader.readVU30();
                bc_comment("newarray[" + std::to_string(argCount) + "]");
                
                std::vector<ObjectPtr> args;
                for(uint32_t i = 0; i < argCount; ++i) args.insert(args.begin(), __pop);
                
                __push(makeArrayObject(args));
              }; break;
              case 0x57: {
                bc_comment("newactivation");
                __push(makeVoidObject()); // TODO:2015-01-04:alex:Activation object.
              }; break;
              case 0x5e:
              case 0x5d:
              case 0x60: {
                read_multiname;
                bc_comment((chr == 0x5e ? "findproperty " : (chr == 0x60 ? "getlex " : "findpropstrict ")) + mn_str);
                
                ObjectPtr object;
                for(int64_t i = scopeStack.size() - 1; i >= 0; --i)
                  if(scopeStack[i].hasProperty(multiname)) {
                    object = scopeStack[i].object;
                    goto gpObjectFound;
                  }
                
              gpObjectNotFound:
                if(chr == 0x5d || chr == 0x60) // findpropstrict
                  throw "Don't know what-exception";
                else
                  object = globalObject;
                
              gpObjectFound:
                if(chr == 0x60) { // getlex
                  ObjectPtr v = object->getProperty(multiname);
                  __push(v == NULL ? undefinedObject : v);
                } else // findproperty, findpropstrict
                  __push(object);
              }; break;
              case 0x61:
              case 0x68: {
                ObjectPtr value = __pop;
                read_multiname;
                ObjectPtr obj = __pop;
                
                bc_comment((chr == 0x68 ? "initproperty " : "setproperty ") + mn_str);
                
                // TODO:2015-01-04:alex:initproperty is able to set constant traits!
                obj->setProperty(multiname, value);
              }; break;
              case 0x66: {
                read_multiname;
                bc_comment("getproperty " + mn_str);
                
                ObjectPtr v = __pop->getProperty(multiname);
                __push(v == NULL ? undefinedObject : v);
              }; break;
                
              case 0x65: {
                uint8_t index = codeReader.readU8();
                bc_comment("getscopeobject");
                __push(scopeStack.at(index).object);
              }; break;
                
              case 0x6c: {
                u30_t slotIndex = codeReader.readVU30();
                bc_comment("getslot " + std::to_string(slotIndex));
                
                ObjectPtr v = __pop->getSlot(slotIndex);
                __push(v == NULL ? undefinedObject : v);
              }; break;
              case 0x6d: {
                u30_t slotIndex = codeReader.readVU30();
                bc_comment("setslot " + std::to_string(slotIndex));
                
                ObjectPtr value = __pop;
                ObjectPtr object = __pop;
                object->setSlot(slotIndex, value);
              }; break;
                
              case 0x73: {
                bc_comment("convert_i");
                __push(makeIntObject(__pop->coerce_i()));
              }; break;
              case 0x75: {
                bc_comment("convert_d");
                __push(makeNumberObject(__pop->coerce_d()));
              }; break;
              case 0x80: {
                read_multiname; // TODO:2015-01-01:alex:"Must not be a runtime multiname"
                bc_comment("coerce " + mn_str);
                __push(__pop->coerce(getClassByName(multiname.nameString())));
              }; break;
              case 0x82: {
                bc_comment("coerce_a"); // TODO:2015-01-01:alex:Assert that we have at least one stack element.
              }; break;
              case 0x96: {
                bc_comment("not");
                __push(makeBooleanObject(!__pop->coerce_b()));
              }; break;
              case 0xa0: {
                bc_comment("add");
                
                read_diadic;
                
                if(cast(Number, v1) && cast(Number, v2))
                  __push(makeNumberObject(cast(Number, v1)->value + cast(Number, v2)->value));
                else
                  __push(makeStringObject(v1->coerce_s() + v2->coerce_s()));
              }; break;
              case 0xa1: {
                bc_comment("subtract"); // TODO:2014-12-28:alex:Right-To-Left bug.
                
                read_diadic;
                __push(makeNumberObject(v1->coerce_d() - v2->coerce_d()));
              }; break;
              case 0xa2: {
                bc_comment("multiply");
                
                read_diadic;
                __push(makeNumberObject(v1->coerce_d() * v2->coerce_d()));
              }; break;
              case 0xa3: {
                bc_comment("divide");
                
                read_diadic;
                __push(makeNumberObject(v1->coerce_d() / v2->coerce_d()));
              }; break;
              case 0xab: {
                bc_comment("equals");
                read_diadic;
                __push(makeBooleanObject(*v1 == *v2));
              }; break;
                
              case 0x62: // getlocal with u30
              case 0xd0: case 0xd1: case 0xd2: case 0xd3: { // getlocal_<n>
                u30_t index = chr == 0x62 ? codeReader.readVU30() : chr - 0xd0;
                bc_comment(std::string("getlocal") + (chr == 0x62 ? " " : "_") + std::to_string(index));
                __push(registers[index]);
              }; break;
                
              case 0x63: // setlocal with u30
              case 0xd4: case 0xd5: case 0xd6: case 0xd7: { // setlocal_<n>
                u30_t index = chr == 0x63 ? codeReader.readVU30() : chr - 0xd4;
                bc_comment(std::string("setlocal") + (chr == 0x63 ? " " : "_") + std::to_string(index));
                
                registers[index] = __pop;
              }; break;
              default:
                bc_comment("[unknown] op_" + std::to_string(chr));
            }
          }
          
          return __pop;
        }
      }
    };
  }
}

#endif /* defined(__jswf__VM__) */