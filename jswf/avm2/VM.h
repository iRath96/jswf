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
        NumberHint = 1
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
      
      // TODO:2015-01-04:alex:Hardcode class names for native objects??
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
      
      virtual bool operator ==(const Object &rhs) const { // ECMA-262, section 11.9.3
        if(klass == rhs.klass && this == &rhs) return true;
        return false;
      }
      
      virtual ObjectPtr ecmaToPrimitive(ECMAHint::Enum hint = ECMAHint::NoHint) {
        // TODO:2015-01-05:alex:Implement this?
        return shared_from_this();
      }
      
      enum AcResult {
        AcUndefinedResult = 0,
        AcFalseResult = 1,
        AcTrueResult = 2
      };
      
      virtual AcResult abstractCompare(Object &rhs, bool leftFirst = true); // ECMA-262, section 11.8.5
      
      virtual bool isNaN() const { return false; }
      
      virtual bool coerce_b() const { return true; } // ECMA-262, section 9.2
      virtual s32_t coerce_i() const { throw "Cannot coerce_i"; }
      virtual std::string coerce_s() const { return "[object " + klass->iinfo.name->nameString() + "]"; }
      virtual Namespace coerce_ns() const { throw "Cannot coerce_ns"; }
      virtual Multiname coerce_multiname() const { throw "Cannot coerce_multiname"; }
      virtual double coerce_d() const { throw "Cannot coerce_d"; }
      
      double ecmaToNumber() const { return this->coerce_d(); }
      
      virtual ObjectPtr coerce(Class *newKlass, const ObjectPtr &recv) {
        // TODO:2015-01-01:alex:Implement this!
        klass = newKlass; // <tt>coerce Function</tt> will actually make a <tt>builtin.as$0::MethodClosure</tt> a <tt>Function</tt>.
        return recv;
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
    };
    
    class VoidObject : public NativeObject {
    public:
      enum Kind {
        NullValue = 0,
        UndefinedValue
      };
      
      Kind value;
      
      VoidObject(VM *vm, const Kind &value);
      bool operator ==(const Object &rhs) {
        return dynamic_cast<const VoidObject *>(&rhs);
      }
      
      bool coerce_b() const { return false; }
      std::string coerce_s() const { return value == NullValue ? "null" : "undefined"; }
      double coerce_d() const { return value == NullValue ? +0 : NAN; } // TODID:2015-01-01:alex:+0 for null.
    };
    
#define make_native_class(name, parent_class, value_type, coerce_name, code) \
  class name : public parent_class { \
  public: \
    value_type value; \
    name(VM *vm, const value_type &value); \
    bool operator ==(const Object &rhs) const { \
      const name *casted = dynamic_cast<const name *>(&rhs); \
      return casted && value == casted->value; \
    } \
    value_type coerce_name() const { return value; } \
    code \
  };
    
    make_native_class(NamespaceObject, NativeObject, Namespace, coerce_ns,);
    make_native_class(MultinameObject, NativeObject, Multiname, coerce_multiname,);
    make_native_class(ArrayObject, NativeObject, std::vector<ObjectPtr>, coerce_a,);
    make_native_class
     (StringObject, NativeObject, std::string, coerce_s,
      bool coerce_b() const { return !value.empty(); }
    );
    make_native_class
     (BooleanObject, NativeObject, bool, coerce_b,
      std::string coerce_s() const { return value ? "true" : "false"; }
      double coerce_d() const { return value ? 1 : +0; }
    );
    
    class NumberObject : public NativeObject {
    public:
      NumberObject(VM *vm, Class *klass) : NativeObject(vm, klass) {}
    };
    
    make_native_class
    (DoubleObject, NumberObject, double, coerce_d,
     bool coerce_b() const { return !isnan(value) && value != 0; }
     std::string coerce_s() const { return std::to_string(value); }
     );
    make_native_class // TODO:2015-01-04:alex:Not DRY with IntObject
     (UIntObject, NumberObject, u32_t, coerce_u,
      std::string coerce_s() const { return std::to_string(value); }
      bool coerce_b() const { return value != 0; }
      double coerce_d() const { return value; }
    );
    make_native_class
     (IntObject, NumberObject, s32_t, coerce_i,
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
      ObjectPtr undefinedObject, nullObject;
      
      VM() {
        std::shared_ptr<ABCFile> api = std::make_shared<ABCFile>();
        
        Class *parent = NULL, *object = NULL;
        parent = object = api->makeClass(api->makeQName("", "Object"), parent);
        
        api->makeClass(api->makeQName("", "Boolean"), object);
        Class *numberClass = api->makeClass(api->makeQName("", "Number"), object);
        api->makeClass(api->makeQName("", "int"), object);
        api->makeClass(api->makeQName("", "uint"), object);
        api->makeClass(api->makeQName("", "Function"), object);
        api->makeClass(api->makeQName("", "String"), object);
        
        api->makeClass(api->makeQName("", "builtin.as$0::MethodClosure"), NULL);
        
        Class *eventClass = api->makeClass(api->makeQName("flash.events", "Event"), object);
        std::shared_ptr<SlotTraitInfo> enterFrame = std::make_shared<SlotTraitInfo>();
        enterFrame->kind = TraitInfo::ConstKind;
        enterFrame->typeName = api->makeQName("", "String");
        api->makeString("enterFrame", &enterFrame->vindex);
        enterFrame->vkind = ConstantKind::UTF8Kind;
        eventClass->iinfo.traits.push_back(enterFrame);
        
        Class *eventDispatcherClass = parent = api->makeClass(api->makeQName("flash.events", "EventDispatcher"), parent);
        MethodInfo *aELMethod = api->makeMethodInfo();
        aELMethod->nativeImpl = &builtin_addEventListener;
        
        std::shared_ptr<MethodTraitInfo> t = std::make_shared<MethodTraitInfo>();
        t->kind = TraitInfo::MethodKind;
        t->name = api->makeQName("", "addEventListener", NamespaceKind::PackageNamespaceKind);
        t->methodInfo = aELMethod;
        eventDispatcherClass->iinfo.traits.push_back(t);
        
        parent = api->makeClass(api->makeQName("flash.display", "DisplayObject"), parent);
        
        std::vector<std::string> dispObjProps = { "rotation", "x", "y", "xScale", "yScale", "alpha" };
        // TODO:2015-01-04:alex:Correct default values
        
        u30_t nullDblIndex = api->constantPool.indexDouble(0.0);
        u30_t slotId = 0;
        for(auto it = dispObjProps.begin(); it != dispObjProps.end(); ++it) {
          parent->iinfo.traits.push_back(
            std::make_shared<SlotTraitInfo>(slotId++,
                                            api->makeQName("", *it, NamespaceKind::PackageNamespaceKind),
                                            numberClass->iinfo.name,
                                            nullDblIndex,
                                            ConstantKind::DoubleKind
                                            )
          );
        }
        
        parent = api->makeClass(api->makeQName("flash.display", "InteractiveObject"), parent);
        parent = api->makeClass(api->makeQName("flash.display", "DisplayObjectContainer"), parent);
        parent = api->makeClass(api->makeQName("flash.display", "Sprite"), parent);
        parent = api->makeClass(api->makeQName("flash.display", "MovieClip"), parent);
        
        api->makeClass(api->makeQName("", "void"), object);
        api->makeClass(api->makeQName("", "null"), object);
        
        api->makeClass(api->makeQName("", "Class"), object);
        
        Class *globalClass = api->makeClass(api->makeQName("", "global"), object);
        globalObject = std::make_shared<Object>(this, globalClass);
        
        loadABCFile(api);
        
        // ...
        
        ObjectPtr traceMethod = std::make_shared<BuiltinMethodObject>(this, &builtin_trace);
        ObjectPtr gQCNMethod = std::make_shared<BuiltinMethodObject>(this, &builtin_getQualifiedClassName);
        
        // TODO:2015-04-04:alex:These should be traits.
        globalObject->setProperty(*api->makeQName("", "trace"), traceMethod);
        globalObject->setProperty(*api->makeQName("flash.utils", "getQualifiedClassName"), gQCNMethod);
        
        undefinedObject = ObjectPtr(new VoidObject(this, VoidObject::UndefinedValue));
        nullObject = ObjectPtr(new VoidObject(this, VoidObject::NullValue));
      }
      
      void loadABCFile(std::shared_ptr<ABCFile> file) {
        files.push_back(file);
        
        for(size_t i = 0, j = file->classes.size(); i < j; ++i) {
          Class &klass = *file->classes[i].get();
          klass.vm = this;
          if(klass.iinfo.superName != NULL)
            klass.parent = getClassByName(klass.iinfo.superName->nameString());
          
          MultinamePtr &name = klass.iinfo.name;
          classes[name->nameString()] = &klass;
          
          ObjectPtr classObject = std::make_shared<ClassObject>(this, &klass);
          globalObject->setProperty(*klass.iinfo.name, classObject);
          
          for(auto it = klass.cinfo.traits.begin(); it != klass.cinfo.traits.end(); ++it); // TODO
          
          if(klass.cinfo.initializer->body) {
            ObjectPtr arg0 = std::make_shared<Object>(this, getClassByName("Class"));
            runMethod(klass.cinfo.initializer, std::vector<ObjectPtr>{ arg0 });
          }
        }
      }
      
      Class *getClassByName(std::string name) const {
        if(classes.find(name) == classes.end()) return NULL;
        return classes.at(name);
      }
      
      ObjectPtr instantiateClass(std::string klassName) {
        return instantiateClass(getClassByName(klassName));
      }
      
      void setupSlotDefaults(ObjectPtr &obj, Class *klass) {
        if(klass->parent) setupSlotDefaults(obj, klass->parent);
        for(auto it = klass->iinfo.traits.begin(); it != klass->iinfo.traits.end(); ++it)
          if(dynamic_cast<SlotTraitInfo *>(it->get())) {
            SlotTraitInfo *s = dynamic_cast<SlotTraitInfo *>(it->get());
            
            ObjectPtr value;
            switch(s->vkind) {
              case ConstantKind::DoubleKind:
                value.reset(new DoubleObject(this, klass->file->constantPool.doubles[s->vindex]));
                break;
              default:
                value = undefinedObject;
            }
            
            obj->slotMap[s->slotId] = value;
          }
      }
      
      ObjectPtr instantiateClass(Class *klass) {
        printf("Should create an instance of %s.\n", klass->iinfo.name->nameString().c_str());
        
        ObjectPtr obj = std::make_shared<Object>(this, klass);
        setupSlotDefaults(obj, klass);
        runMethod(klass->iinfo.initializer, std::vector<ObjectPtr>{ obj });
        
        return obj;
      }
      
      ObjectPtr instantiateDisplayClass(Class *klass, flash::DisplayListEntry *entry) {
        printf("Should create a display instance of %s.\n", klass->iinfo.name->nameString().c_str());
        
        ObjectPtr obj = std::make_shared<DisplayObject>(this, klass, entry);
        setupSlotDefaults(obj, klass);
        // TODO:2015-01-05:alex:Set x, y, etc. here.
        runMethod(klass->iinfo.initializer, std::vector<ObjectPtr>{ obj });
        
        return obj;
      }
      
      ObjectPtr runMethod(MethodInfo *method, std::vector<ObjectPtr> arguments) {
        ABCFile *file = method->file;
        
        //printf("runMethod\n");
        
        MethodBody *body = method->body;
        if(body == NULL) {
          if(method->nativeImpl) return method->nativeImpl(*this, method, arguments);
          return std::make_shared<Object>(this, getClassByName("Object"));
        } else {
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
              case 0x0d: {
                s24_t offset = codeReader.readS24();
                bc_comment("ifnle " + std::to_string(offset));
                read_diadic;
                
                if(v2->abstractCompare(*v1) == Object::AcTrueResult)
                  codeReader.seek(offset);
              }; break;
              case 0x0f: {
                s24_t offset = codeReader.readS24();
                bc_comment("ifnge " + std::to_string(offset));
                read_diadic;
                
                if(v1->abstractCompare(*v2) != Object::AcFalseResult)
                  codeReader.seek(offset);
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
              case 0x1b: { // TODO:2015-01-05:alex:Not tested!
                s24_t defaultOffset = codeReader.readS24();
                u30_t caseCount = codeReader.readVU30();
                
                bc_comment("lookupswitch " + std::to_string(defaultOffset) + " " + std::to_string(caseCount));
                
                s32_t index = __pop->coerce_i();
                if(index < 0 || index > caseCount) {
                  codeReader.seek((caseCount + 1) * 3);
                  codeReader.seek(defaultOffset);
                } else {
                  codeReader.seek(index * 3);
                  s24_t offset = codeReader.readS24();
                  codeReader.seek((caseCount - index) * 3);
                  codeReader.seek(offset);
                }
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
                __push(new StringObject(this, obj->getPropertyName(index->coerce_i())));
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
                // This apparently is a signed byte.
                // The specification might be wrong on this.
                
                int8_t byte = codeReader.readS8();
                bc_comment("pushbyte " + std::to_string(byte));
                
                __push(new IntObject(this, byte));
              }; break;
              case 0x25: {
                u30_t value = codeReader.readVU30();
                bc_comment("pushshort " + std::to_string(value));
                
                __push(new IntObject(this, value));
              }; break;
              case 0x26: {
                bc_comment("pushtrue");
                __push(new BooleanObject(this, true));
              }; break;
              case 0x27: {
                bc_comment("pushfalse");
                __push(new BooleanObject(this, false));
              }; break;
              case 0x29: {
                bc_comment("pop");
                __pop;
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
                
                __push(new StringObject(this, str));
              }; break;
              case 0x2d: {
                read_auto(integers, i);
                bc_comment("pushint " + std::to_string(i));
                
                __push(new IntObject(this, i));
              }; break;
              case 0x2f: {
                read_auto(doubles, dbl);
                bc_comment("pushdouble " + std::to_string(dbl));
                
                __push(new DoubleObject(this, dbl));
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
                __push(new BooleanObject(this, hasNext));
                
                if(hasNext) {
                  registers[indexReg] = ObjectPtr(new IntObject(this, i + 1));
                } else {
                  registers[objReg] = nullObject;
                  registers[indexReg] = ObjectPtr(new IntObject(this, 0));
                }
              }; break;
              case 0x40: {
                u30_t index = codeReader.readVU30();
                bc_comment("newfunction " + std::to_string(index));
                __push(new FunctionObject(this, file->methods.at(index).get()));
              }; break;
              case 0x47: {
                bc_comment("returnvoid");
                return undefinedObject;
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
                
              case 0x41: {
                u30_t argCount = codeReader.readVU30();
                bc_comment("call[" + std::to_string(argCount) + "]");
                
                std::vector<ObjectPtr> args;
                for(uint32_t i = 0; i <= argCount; ++i) // <tt><=</tt> because we read one more: receiver (arg0 = <tt>this</tt>)
                  args.insert(args.begin(), __pop);
                
                ObjectPtr method = __pop;
                __push(method->ecmaCall(*this, args));
              }; break;
                
              case 0x46:
              case 0x4f: {
                u30_t mnIndex = codeReader.readVU30();
                u30_t argCount = codeReader.readVU30();
                read_multiname_arg(mnIndex);
                
                bc_comment((chr == 0x46 ? "callproperty[" : "callpropvoid[") + std::to_string(argCount) + "] " + mn_str);
                
                std::vector<ObjectPtr> args;
                for(uint32_t i = 0; i < argCount; ++i) args.insert(args.begin(), __pop);
                
                ObjectPtr object = __pop;
                args.insert(args.begin(), object); // arg0 = <tt>this</tt>
                
                ObjectPtr method = object->getProperty(multiname);
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
                
                __push(new ArrayObject(this, args));
              }; break;
              case 0x57: {
                bc_comment("newactivation");
                __push(nullObject); // TODO:2015-01-04:alex:Activation object.
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
                
                ObjectPtr obj = __pop;
                ObjectPtr v = obj->getProperty(multiname);
                __push(v);
              }; break;
                
              case 0x64: {
                bc_comment("getglobalscope");
                __push(scopeStack.at(0).object);
              }; break;
              case 0x65: {
                uint8_t index = codeReader.readU8();
                bc_comment("getscopeobject " + std::to_string(index));
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
                __push(new IntObject(this, __pop->coerce_i()));
              }; break;
              case 0x75: {
                bc_comment("convert_d");
                __push(new DoubleObject(this, __pop->coerce_d()));
              }; break;
              case 0x80: {
                read_multiname; // TODO:2015-01-01:alex:"Must not be a runtime multiname"
                bc_comment("coerce " + mn_str);
                
                ObjectPtr obj = __pop;
                __push(obj->coerce(getClassByName(multiname.nameString()), obj));
              }; break;
              case 0x82: {
                bc_comment("coerce_a"); // TODO:2015-01-01:alex:Assert that we have at least one stack element.
              }; break;
              case 0x96: {
                bc_comment("not");
                __push(new BooleanObject(this, !__pop->coerce_b()));
              }; break;
              case 0xa0: {
                bc_comment("add");
                
                read_diadic;
                
                // TODO:2015-01-05:alex:This code is crap.
                
                if(cast(Number, v1) && cast(Number, v2))
                  __push(new DoubleObject(this, v1->coerce_d() + v2->coerce_d()));
                else
                  __push(new StringObject(this, v1->coerce_s() + v2->coerce_s()));
              }; break;
              case 0xa1: {
                bc_comment("subtract");
                
                read_diadic;
                __push(new DoubleObject(this, v1->coerce_d() - v2->coerce_d()));
              }; break;
              case 0xa2: {
                bc_comment("multiply");
                
                read_diadic;
                __push(new DoubleObject(this, v1->coerce_d() * v2->coerce_d()));
              }; break;
              case 0xa3: {
                bc_comment("divide");
                
                read_diadic;
                __push(new DoubleObject(this, v1->coerce_d() / v2->coerce_d()));
              }; break;
              case 0xab: {
                bc_comment("equals");
                read_diadic;
                __push(new BooleanObject(this, *v1 == *v2));
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