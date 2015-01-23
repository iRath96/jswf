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

#include "Opcode.h"
#include "ABCFile.h"
#include "TraitInfo.h"
#include "StringReader.h"
#include "Object.h"
#include "Verifier.h"

namespace jswf {  
  namespace avm2 {
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
    
    class VM : public IConstantSource<Object> {
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
        enterFrame->vindex = api->constantPool.strings.insert("enterFrame");
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
        
        u30_t nullDblIndex = api->constantPool.doubles.insert(0.0);
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
      
      ObjectPtr getConstant(ConstantKind::Enum kind) {
        switch(kind) {
          case ConstantKind::NullKind     : return nullObject;
          case ConstantKind::UndefinedKind: return undefinedObject;
          case ConstantKind::TrueKind     : return std::make_shared<BooleanObject>(this, true);
          case ConstantKind::FalseKind    : return std::make_shared<BooleanObject>(this, false);
          case ConstantKind::NANKind      : return std::make_shared<NumberObject<double>>(this, NAN);
          default: return ObjectPtr(NULL);
        }
      }
      
      ObjectPtr getPoolConstant(ConstantKind::Enum kind, ABCFile *file, u30_t index) {
        switch(kind) {
          case ConstantKind::DoubleKind: return std::make_shared<NumberObject<double>>(this, file->constantPool.doubles[index]);
          case ConstantKind::IntKind   : return std::make_shared<NumberObject<int32_t>>(this, file->constantPool.integers[index]);
          case ConstantKind::UIntKind  : return std::make_shared<NumberObject<uint32_t>>(this, file->constantPool.uintegers[index]);
          case ConstantKind::UTF8Kind  : return std::make_shared<StringObject>(this, file->constantPool.strings[index]);
          case ConstantKind::NormalNamespaceKind: return std::make_shared<NamespaceObject>(this, *file->constantPool.namespaces[index]);
          default: return ObjectPtr(NULL);
        }
      }
      
      ObjectPtr getImmediateConstant(ConstantKind::Enum kind, int32_t imm) {
        return std::make_shared<NumberObject<int32_t>>(this, imm);
      }
      
      void setupSlotDefaults(ObjectPtr &obj, Class *klass) {
        if(klass->parent) setupSlotDefaults(obj, klass->parent);
        for(auto it = klass->iinfo.traits.begin(); it != klass->iinfo.traits.end(); ++it)
          if(dynamic_cast<SlotTraitInfo *>(it->get())) {
            SlotTraitInfo *s = dynamic_cast<SlotTraitInfo *>(it->get());
            
            if(ConstantKind::needsPool(s->vkind))
              obj->slotMap[s->slotId] = getPoolConstant(s->vkind, klass->file, s->vindex);
            else
              obj->slotMap[s->slotId] = getConstant(s->vkind);
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
          Verifier v;
          v.file = method->file;
          v.verifyBytecode(method->body->code);
          
          std::vector<Scope> scopeStack; // capacity should be method->body->maxScopeDepth
          std::stack<ObjectPtr> stack; // capacity should be method->body->maxStack
          std::map<u30_t, ObjectPtr> registers; // capacity should be method->body->localCount
          
          Scope globalScope(globalObject, Scope::GlobalScopeKind);
          scopeStack.push_back(globalScope);
          
          for(size_t i = 0, j = arguments.size(); i < j; ++i)
            registers[(u30_t)i] = arguments[i];
          
          bool show_disassembly = 1;
          
          io::StringReader codeReader(method->body->code);
#define bc_comment(v) if(show_disassembly) { \
  int length = (int)(codeReader.pos - op.pos); \
  codeReader.seek(-length); \
  std::string bytecode = codeReader.readString(length); \
  printf("[vm 0x%04lx]", op.pos); \
  for(size_t bci = 0; bci < length; ++bci) printf(" %02x", (uint8_t)bytecode[bci]); \
  printf(" %s\n", std::string(v).c_str()); \
}

#define __ncast(v) ObjectPtr(v)
#define __push(v) stack.push(__ncast(v))
#define __pop (top = stack.top(), stack.pop(), top)
 
          ObjectPtr top;
          
          OpcodeData<Object> op;
          
          while(!codeReader.eof()) {
            op.read(&codeReader, method->file, this, stack);
            bc_comment(op.disassemble(method->file));
            
            switch(op.code) {
#pragma mark Load and store instructions
              case Opcode::op_getlocal_0:
              case Opcode::op_getlocal_1:
              case Opcode::op_getlocal_2:
              case Opcode::op_getlocal_3: op.registerIndex = op.code - Opcode::op_getlocal_0;
              case Opcode::op_getlocal  : __push(registers.find(op.registerIndex) == registers.end() ? undefinedObject : registers[op.registerIndex]); break;
                
              case Opcode::op_setlocal_0:
              case Opcode::op_setlocal_1:
              case Opcode::op_setlocal_2:
              case Opcode::op_setlocal_3: op.registerIndex = op.code - Opcode::op_setlocal_0;
              case Opcode::op_setlocal  : registers[op.registerIndex] = op.value1; break;
              
              case Opcode::op_kill: registers.erase(op.registerIndex); break;
                
#pragma mark Airthmetic instructions
              case Opcode::op_increment  : __push(new NumberObject<double >(this, op.value1->coerce_d  () + 1)); break;
              case Opcode::op_increment_i: __push(new NumberObject<int32_t>(this, op.value1->coerce_s32() + 1)); break;
                
              case Opcode::op_add: {
                if(cast(NumberBase, op.value1) && cast(NumberBase, op.value2))
                  __push(new NumberObject<double>(this, op.value1->coerce_d() + op.value2->coerce_d()));
                else // concatenation
                  __push(new StringObject(this, op.value1->coerce_s() + op.value2->coerce_s()));
              }; break;
              case Opcode::op_add_i     : __push(new NumberObject<int32_t>(this, op.value1->coerce_s32() + op.value2->coerce_s32())); break;
              case Opcode::op_subtract  : __push(new NumberObject<double >(this, op.value1->coerce_d  () - op.value2->coerce_d  ())); break;
              case Opcode::op_subtract_i: __push(new NumberObject<int32_t>(this, op.value1->coerce_s32() - op.value2->coerce_s32())); break;
              case Opcode::op_multiply  : __push(new NumberObject<double >(this, op.value1->coerce_d  () * op.value2->coerce_d  ())); break;
              case Opcode::op_multiply_i: __push(new NumberObject<int32_t>(this, op.value1->coerce_s32() * op.value2->coerce_s32())); break;
              case Opcode::op_divide    : __push(new NumberObject<double >(this, op.value1->coerce_d  () / op.value2->coerce_d  ())); break;
              case Opcode::op_negate    : __push(new NumberObject<double >(this, -op.value1->coerce_d  ())); break;
              case Opcode::op_negate_i  : __push(new NumberObject<int32_t>(this, -op.value1->coerce_s32())); break;
                
#pragma mark Bit manipulation instructions
                // TODO:2015-01-22:alex:Verify that these are really u32.
              case Opcode::op_bitnot: __push(new NumberObject<uint32_t>(this, ~op.value1->coerce_u32())); break;
              case Opcode::op_bitand: __push(new NumberObject<uint32_t>(this, op.value1->coerce_u32() & op.value2->coerce_u32())); break;
              case Opcode::op_bitor : __push(new NumberObject<uint32_t>(this, op.value1->coerce_u32() | op.value2->coerce_u32())); break;
              case Opcode::op_bitxor: __push(new NumberObject<uint32_t>(this, op.value1->coerce_u32() ^ op.value2->coerce_u32())); break;
                
                // TODO:2015-01-22:alex:Verify that these work, especially sign-extended right shift.
              case Opcode::op_lshift : __push(new NumberObject<uint32_t>(this, op.value1->coerce_u32() << (op.value2->coerce_u32() & 0x1f))); break;
              case Opcode::op_rshift : __push(new NumberObject<uint32_t>(this, op.value1->coerce_s32() >> (op.value2->coerce_u32() & 0x1f))); break;
              case Opcode::op_urshift: __push(new NumberObject<uint32_t>(this, op.value1->coerce_u32() >> (op.value2->coerce_u32() & 0x1f))); break;
                
#pragma mark Type conversion instructions
              case Opcode::op_coerce   : __push(op.value1->coerce(getClassByName(op.multiname.nameString()))); break;
              case Opcode::op_coerce_a : __push(op.value1); break;
              case Opcode::op_convert_i: __push(new NumberObject<int32_t>(this, op.value1->coerce_s32())); break;
              case Opcode::op_convert_d: __push(new NumberObject<double >(this, op.value1->coerce_d  ())); break;
                
#pragma mark Object creation and manipulation instructions
#pragma mark Stack management instructions
                // constants
              case Opcode::op_pushnull     :
              case Opcode::op_pushundefined:
              case Opcode::op_pushtrue     :
              case Opcode::op_pushfalse    :
              case Opcode::op_pushnan      :
                
                // pool constants
              case Opcode::op_pushstring   :
              case Opcode::op_pushint      :
              case Opcode::op_pushdouble   :
              case Opcode::op_pushnamespace:
                
                // immediate values
              case Opcode::op_pushbyte     :
              case Opcode::op_pushshort    : __push(op.constant); break;
                
                // scope management
              case Opcode::op_pushwith: {
                if(dynamic_cast<VoidObject *>(op.value1.get())) // if null or undefined
                  throw "TypeError";
                Scope scope(op.value1, Scope::WithScopeKind);
                scopeStack.push_back(scope);
              }; break;
              case Opcode::op_pushscope: {
                Scope scope(op.value1, Scope::NormalScopeKind);
                scopeStack.push_back(scope);
              }; break;
              case Opcode::op_popscope: scopeStack.pop_back(); break;
                
                // management
              case Opcode::op_pop : break;
              case Opcode::op_dup : stack.push(op.value1); stack.push(op.value1); break;
              case Opcode::op_swap: stack.push(op.value2); stack.push(op.value1); break;
                
#pragma mark Control transfer instructions
#define perform_branch codeReader.seek(op.branchOffset)
#define branch_if(condition) if(condition) perform_branch;
              case Opcode::op_ifnlt: branch_if(!(*op.value1 <  *op.value2)); break;
              case Opcode::op_ifnle: branch_if(!(*op.value1 <= *op.value2)); break;
              case Opcode::op_ifngt: branch_if(!(*op.value1 >  *op.value2)); break;
              case Opcode::op_ifnge: branch_if(!(*op.value1 >= *op.value2)); break;
              
              case Opcode::op_jump   : perform_branch; break;
              case Opcode::op_iftrue : branch_if( op.value1->coerce_b()); break;
              case Opcode::op_iffalse: branch_if(!op.value1->coerce_b()); break;
              
              case Opcode::op_ifeq: branch_if(*op.value1 == *op.value2); break;
              case Opcode::op_ifne: branch_if(*op.value1 != *op.value2); break;
              case Opcode::op_iflt: branch_if(*op.value1 <  *op.value2); break;
              case Opcode::op_ifle: branch_if(*op.value1 <= *op.value2); break;
              case Opcode::op_ifgt: branch_if(*op.value1 >  *op.value2); break;
              case Opcode::op_ifge: branch_if(*op.value1 >= *op.value2); break;
              
              case Opcode::op_ifstricteq: branch_if( op.value1->strictEquals(*op.value2)); break;
              case Opcode::op_ifstrictne: branch_if(!op.value1->strictEquals(*op.value2)); break;
              
              case Opcode::op_label: break; // Do nothing.
              
              case Opcode::op_lookupswitch: { // TODO:2015-01-05:alex:Not tested!
                s24_t defaultOffset = codeReader.readS24();
                u30_t caseCount = codeReader.readVU30();
                
                bc_comment("lookupswitch " + std::to_string(defaultOffset) + " " + std::to_string(caseCount));
                
                // TODO:2015-01-22:alex:Test if cast(int)
                s32_t index = __pop->coerce_s32();
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
              
#undef perform_branch
#undef branch_if

#pragma mark Function invocation and return instructions
                // invocation
              case Opcode::op_call: {
                op.arguments.insert(op.arguments.begin(), op.value2);
                __push(op.value1->ecmaCall(*this, op.arguments));
              }; break;
                
              case Opcode::op_callproperty:
              case Opcode::op_callpropvoid: {
                op.arguments.insert(op.arguments.begin(), op.value1);
                
                ObjectPtr method = op.value1->getProperty(op.multiname);
                ObjectPtr ret = method->ecmaCall(*this, op.arguments);
                
                if(op.code != Opcode::op_callpropvoid) __push(ret);
              }; break;
                
                // return
              case Opcode::op_returnvoid : return undefinedObject;
              case Opcode::op_returnvalue: return op.value1;
                
#pragma mark Exception instructions
#pragma mark Debugging instructions
#pragma mark Property enumeration
              // TODO:2015-01-22:alex:Test if cast(int)
              case Opcode::op_nextname: __push(new StringObject(this, op.value1->getPropertyName(op.value2->coerce_s32()))); break;
              case Opcode::op_hasnext2: {
                u30_t objReg = codeReader.readVU30();
                u30_t indexReg = codeReader.readVU30();
                
                bc_comment("hasnext2 " + std::to_string(objReg) + " " + std::to_string(indexReg));
                
                // TODO:2015-01-22:alex:Test if cast(int)
                u32_t i = registers[indexReg]->coerce_u32();
                
                ObjectPtr obj = registers[objReg];
                bool hasNext = obj->hasNextProperty(i);
                __push(new BooleanObject(this, hasNext));
                
                if(hasNext) {
                  registers[indexReg] = ObjectPtr(new NumberObject<int32_t>(this, i + 1));
                } else {
                  registers[objReg] = nullObject;
                  registers[indexReg] = ObjectPtr(new NumberObject<int32_t>(this, 0));
                }
              }; break;
              case Opcode::op_newfunction: {
                u30_t index = codeReader.readVU30();
                bc_comment("newfunction " + std::to_string(index));
                __push(new FunctionObject(this, file->methods.at(index).get()));
              }; break;
                
              case Opcode::op_newobject: {
                // This is weird.
                
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
              case Opcode::op_newarray: __push(new ArrayObject(this, op.arguments)); break;
              case Opcode::op_newactivation: __push(nullObject); break; // TODO:2015-01-04:alex:Activation object.
              
#pragma mark Accessing properties
              case Opcode::op_findproperty:
              case Opcode::op_findpropstrict:
              case Opcode::op_getlex: {
                ObjectPtr object;
                for(int64_t i = scopeStack.size() - 1; i >= 0; --i)
                  if(scopeStack[i].hasProperty(op.multiname)) {
                    object = scopeStack[i].object;
                    goto gpObjectFound;
                  }
                
              gpObjectNotFound:
                if(op.code == Opcode::op_findproperty) // loose
                  object = globalObject;
                else // strict
                  throw "Don't know what-exception";
                
              gpObjectFound:
                if(op.code == Opcode::op_getlex) {
                  ObjectPtr v = object->getProperty(op.multiname);
                  __push(v == NULL ? undefinedObject : v);
                } else
                  __push(object);
              }; break;
                
              case Opcode::op_setproperty   : // TODO:2015-01-04:alex:initproperty is able to set constant traits!
              case Opcode::op_initproperty  : op.value1->setProperty(op.multiname, op.value2); break;
              case Opcode::op_getproperty   : __push(op.value1->getProperty(op.multiname)); break;
              case Opcode::op_getglobalscope: __push(scopeStack.at(0).object); break;
              case Opcode::op_getscopeobject: {
                uint8_t index = codeReader.readU8();
                bc_comment("getscopeobject " + std::to_string(index));
                __push(scopeStack.at(index).object);
              }; break;
                
              case Opcode::op_getslot: {
                u30_t slotIndex = codeReader.readVU30();
                bc_comment("getslot " + std::to_string(slotIndex));
                
                ObjectPtr v = op.value1->getSlot(slotIndex);
                __push(v == NULL ? undefinedObject : v);
              }; break;
              case Opcode::op_setslot: {
                u30_t slotIndex = codeReader.readVU30();
                bc_comment("setslot " + std::to_string(slotIndex));
                
                op.value1->setSlot(slotIndex, op.value2);
              }; break;
              
              case Opcode::op_not          : __push(new BooleanObject(this, !op.value1->coerce_b())); break;
              case Opcode::op_equals       : __push(new BooleanObject(this, *op.value1 == *op.value2)); break;
              case Opcode::op_lessthan     : __push(new BooleanObject(this, *op.value1 <  *op.value2)); break;
              case Opcode::op_greaterthan  : __push(new BooleanObject(this, *op.value1 >  *op.value2)); break;
              case Opcode::op_lessequals   : __push(new BooleanObject(this, *op.value1 <= *op.value2)); break;
              case Opcode::op_greaterequals: __push(new BooleanObject(this, *op.value1 >= *op.value2)); break;
                
              case Opcode::op_constructsuper: break; // TODO:2015-01-22:alex:Implement this.
                
#pragma mark Default
              default:
                throw "Not implemented";
            } // end switch
            
            // clean up
            op.reset();
          } // end while
          
          return __pop;
        }
      }
    };
  }
}

#endif /* defined(__jswf__VM__) */