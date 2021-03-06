//
//  ABCFile.h
//  jswf
//
//  Created by Alexander Rath on 27.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__ABCFile__
#define __jswf__ABCFile__

#include <set>
#include <cmath>
#include <stack>
#include <map>

#include "stdlib.h"
#include "GenericReader.h"
#include "types.h"

#include "Namespace.h"
#include "Metadata.h"
#include "Multiname.h"
#include "ConstantKind.h"
#include "TraitInfo.h"
#include "MethodInfo.h"

namespace jswf {
  namespace avm2 {
    struct ConstantPool {
      /**
       * Extends std::vector<T> to support set-like behavior.
       * @note This is needed because the constant arrays must be indexable. (std::set would not support this)
       */
      template<typename T>
      class SetU30 : public std::vector<T> {
      public:
        /**
         * Return the index of an element, inserting the element if it is not yet contained in this set.
         * @param [in] value The element to find / insert.
         * @return The index of the element as U30
         */
        u30_t insert(const T &value) {
          size_t i = 0, j = this->size();
          
          for(i = 0; i < j; ++i) if((*this)[i] == value)
            return (u30_t)i; // already in the set
          
          // not yet in the set.
          
          this->resize(j+1);
          (*this)[j] = value;
          return (u30_t)j;
        }
      };
      
      SetU30<s32_t          > integers;
      SetU30<u32_t          > uintegers;
      SetU30<d64_t          > doubles;
      SetU30<string         > strings;
      SetU30<NamespacePtr   > namespaces;
      SetU30<NamespaceSetPtr> namespaceSets;
      SetU30<MultinamePtr   > multinames;
      
      ConstantPool() {
        integers .insert(0);
        uintegers.insert(0);
        doubles  .insert(NAN);
        strings  .insert("");
        
        NamespacePtr ns = std::make_shared<Namespace>();
        ns->kind = NamespaceKind::NormalNamespaceKind;
        ns->name = "";
        namespaces.insert(ns);
        
        MultinamePtr any = std::make_shared<Multiname>(Multiname::RTQNameKind);
        any->name = "";
        multinames.insert(any);
      }
    };
    
    struct InstanceInfo {
      enum Flags : u8_t {
        ClassSealedFlag = 0x01,
        ClassFinalFlag = 0x02,
        ClassInterfaceFlag = 0x04,
        ClassProtectedNsFlag = 0x08
      };
      
      MultinamePtr name, superName;
      
      Flags flags;
      NamespacePtr protectedNs;
      std::vector<MultinamePtr> interfaces;
      
      MethodInfo *initializer; // known as <tt>iinit</tt>
      std::vector<std::shared_ptr<TraitInfo>> traits;
    };
    
    struct ClassInfo {
      MethodInfo *initializer; // known as <tt>cinit</tt>
      std::vector<std::shared_ptr<TraitInfo>> traits;
    };
    
    struct ScriptInfo {
      MethodInfo *initializer; // known as <tt>init</tt>
      std::vector<std::shared_ptr<TraitInfo>> traits;
    };
    
    class Object;
    typedef std::shared_ptr<Object> ObjectPtr;
    typedef std::map<TraitInfo *, ObjectPtr> TraitMap;
    typedef std::map<u30_t, ObjectPtr> SlotMap;
    
    class VM;
    class Class {
    public:
      ABCFile *file;
      
      Class *parent;
      VM *vm;
      
      ClassInfo cinfo;
      InstanceInfo iinfo;
      
      TraitMap traitMap;
      SlotMap slotMap;
    };
    
    // Hallelujah.
    
    namespace ast {
      class CompoundNode;
    }
    
    class ABCFile {
    public:
      u16_t majorVersion, minorVersion;
      ConstantPool constantPool;
      
      std::vector<std::unique_ptr<MethodInfo>> methods;
      std::vector<std::unique_ptr<Metadata>> metadata;
      std::vector<std::unique_ptr<Class>> classes; // contains ClassInfo and InstanceInfo
      std::vector<std::unique_ptr<ScriptInfo>> scripts;
      std::vector<std::unique_ptr<MethodBody>> methodBodies;
      
      std::shared_ptr<io::GenericReader> reader;
      
    public:
      ABCFile(std::shared_ptr<io::GenericReader> reader) : reader(reader) { read(); }
      ABCFile() {}
      
      std::string makeString(const std::string &string) {
        constantPool.strings.insert(string);
        return string;
      }
      
      NamespacePtr makeNamespace(std::string name, NamespaceKind::Enum kind) {
        NamespacePtr ns = std::make_shared<Namespace>();
        ns->name = makeString(name);
        ns->kind = kind;
        constantPool.namespaces.insert(ns);
        return ns;
      }
      
      // TODO:2015-01-17:alex:Ouch. Those should _not_ be shared pointers in the constan pool!
      MultinamePtr makeMultiname(Multiname::Kind kind) {
        MultinamePtr ptr = std::make_shared<Multiname>(kind);
        constantPool.multinames.insert(ptr);
        return ptr;
      }
      
      MultinamePtr makeQName(std::string ns, std::string name) {
        return makeQName(ns, name, NamespaceKind::NormalNamespaceKind);
      }
      
      MultinamePtr makeQName(std::string ns, std::string name, NamespaceKind::Enum nsKind) {
        MultinamePtr ptr = makeMultiname(Multiname::QNameKind);
        ptr->name = makeString(name);
        ptr->ns = makeNamespace(ns, nsKind);
        return ptr;
      }
      
      Class *makeClass(MultinamePtr qname, Class *parent) {
        u30_t i = (u30_t)classes.size();
        classes.resize(i+1);
        
        classes[i] = std::unique_ptr<Class>(new Class());
        classes[i]->parent = parent;
        classes[i]->file = this;
        
        classes[i]->iinfo.name = qname;
        classes[i]->iinfo.initializer = makeMethodInfo();
        classes[i]->cinfo.initializer = makeMethodInfo();
        
        return classes[i].get();
      }
      
      MethodInfo *makeMethodInfo() {
        u30_t i = (u30_t)methods.size();
        methods.resize(i+1);
        methods[i] = std::unique_ptr<MethodInfo>(new MethodInfo());
        methods[i]->file = this;
        methods[i]->body = NULL;
        return methods[i].get();
      }
      
#define u30 reader->readVU30()
#define u30_string &constantPool.strings.at(u30)
#define u30_namespace constantPool.namespaces.at(u30)
#define u30_ns_set constantPool.namespaceSets.at(u30)
#define u30_multiname constantPool.multinames.at(u30)
#define u30_class &classes.at(u30)->cinfo
#define u30_method methods.at(u30).get()
      
    protected:
      void readConstantPool() {
        // read_array is always "plus one" (see avm2overview.pdf)
#define read_array(afield) \
  count = reader->readVU30(); \
  if(count == 0) count = 1; \
  constantPool.afield.resize(count); \
  for(uint32_t i = 1; i < count; ++i)
#define value(afield) constantPool.afield[i]
        u30_t count;
        
        read_array(integers ) value(integers ) = reader->readVS32();
        read_array(uintegers) value(uintegers) = reader->readVU32();
        read_array(doubles  ) value(doubles  ) = reader->readD64();
        
        read_array(strings) {
          u30_t length = reader->readVU30();
          value(strings) = reader->readString(length);
        }
        
        read_array(namespaces) {
          value(namespaces) = std::make_shared<Namespace>();
          value(namespaces)->kind = (NamespaceKind::Enum)reader->readU8();
          value(namespaces)->name = *u30_string;
        }
        
        read_array(namespaceSets) {
          u30_t count = u30;
          value(namespaceSets) = std::make_shared<NamespaceSet>();
          value(namespaceSets)->namespaces.resize(count);
          for(uint32_t j = 0; j < count; ++j)
            value(namespaceSets)->namespaces[j] = u30_namespace;
        }
        
        read_array(multinames) {
          u8_t kind = reader->readU8();
          
          Multiname &mn = *(value(multinames) = std::make_shared<Multiname>((Multiname::Kind)kind));
          
          if(mn.hasNS) mn.ns = u30_namespace;
          if(mn.hasName) mn.name = *u30_string;
          if(mn.hasNSSet) mn.nsSet = u30_ns_set;
        }
        
#undef read_array
#undef value
      }
      
      inline void readMetadata(Metadata &md) {
        md.name = u30_string;
        
        u30_t itemCount = u30;
        for(uint32_t i = 0; i < itemCount; ++i) {
          md.items[i].key = u30_string;
          md.items[i].value = u30_string;
        }
      }
      
      u30_t assignSlotId(void *ptr) {
        return (u30_t)(uint64_t)ptr;
      }
      
      void readTraits(std::vector<std::shared_ptr<TraitInfo>> &traits) {
        u30_t traitCount = u30;
        traits.resize(traitCount);
        for(uint32_t i = 0; i < traitCount; ++i) {
          MultinamePtr name = u30_multiname;
          uint8_t attrAndKind = reader->readU8();
          
          TraitInfo::Kind kind = (TraitInfo::Kind)(attrAndKind & 0x0f);
          TraitInfo::Attributes attributes = (TraitInfo::Attributes)(attrAndKind >> 4);
          
          TraitInfo *trait = NULL;
          
          switch(kind) {
            case TraitInfo::SlotKind:
            case TraitInfo::ConstKind: {
              SlotTraitInfo *r = new SlotTraitInfo();
              r->slotId = u30;
              if(r->slotId == 0) r->slotId = assignSlotId(r);
              
              r->typeName = u30_multiname;
              r->vindex = u30;
              r->vkind = (ConstantKind::Enum)(r->vindex == 0 ? 0 : reader->readU8());
              trait = r;
            }; break;
            case TraitInfo::ClassKind: {
              ClassTraitInfo *r = new ClassTraitInfo();
              r->slotId = u30;
              if(r->slotId == 0) r->slotId = assignSlotId(r);
              
              r->classInfo = u30_class;
              trait = r;
            }; break;
            case TraitInfo::FunctionKind: {
              FunctionTraitInfo *r = new FunctionTraitInfo();
              r->slotId = u30;
              if(r->slotId == 0) r->slotId = assignSlotId(r);
              
              r->methodInfo = u30_method;
              trait = r;
            }; break;
            case TraitInfo::MethodKind:
            case TraitInfo::GetterKind:
            case TraitInfo::SetterKind: {
              MethodTraitInfo *r = new MethodTraitInfo();
              r->dispId = u30;
              r->methodInfo = u30_method;
              trait = r;
            }; break;
            default: {
              throw "VerifyError (unknown kind of trait)";
            };
          }
          
          trait->name = name; // TODO:2014-12-28:alex:Assert this is a QName.
          trait->kind = kind;
          trait->attributes = attributes;
          
          u30_t metadataCount = attributes & TraitInfo::MetadataAttribute ? u30 : 0;
          trait->metadata.resize(metadataCount);
          for(uint32_t j = 0; j < metadataCount; ++j)
            trait->metadata[j] = u30;
          
          traits[i].reset(trait);
        }
      }
      
      void decompile(MethodInfo &info, std::string name);
      void decompile(MethodInfo &info, std::string name, ast::CompoundNode *compound);
      
#define _multiname(u) u->nameString().c_str()
      
      void printConstant(ConstantKind::Enum kind, u30_t index) {
        switch(kind) {
          case ConstantKind::IntKind   : printf("%d", constantPool.integers[index]); break;
          case ConstantKind::DoubleKind: printf("%lf", constantPool.doubles[index]); break;
          
          case ConstantKind::FalseKind : printf("false"); break;
          case ConstantKind::TrueKind  : printf("true"); break;
          
          default:
            printf("?");
        }
      }
      
      void printTrait(TraitInfo *trait) {
        NamespaceKind::Enum nsKind = trait->name->ns->kind;
        
        switch(nsKind) {
          case NamespaceKind::PackageNamespaceKind: printf("public "); break;
          case NamespaceKind::PackageInternalNsKind: printf(""); break;
          case NamespaceKind::ProtectedNamespaceKind: printf("protected "); break;
          case NamespaceKind::PrivateNamespaceKind: printf("private "); break;
          default: printf("namespace? ");
        }
        
        const char *name = _multiname(trait->name);
        
        switch(trait->kind) {
          case TraitInfo::SlotKind: {
            printf("var %s", name);
            SlotTraitInfo *t = (SlotTraitInfo *)trait;
            printf(":%s", _multiname(t->typeName));
            if(t->vkind != ConstantKind::UndefinedKind) {
              printf(" = ");
              printConstant(t->vkind, t->vindex);
            }
            
            printf(";\n");
          }; break;
          case TraitInfo::MethodKind: {
            MethodTraitInfo *t = (MethodTraitInfo *)trait;
            decompile(*t->methodInfo, std::string(name));
          }; break;
          default:
            printf("What is %s?", name);
        }
      }
      
      void read() {
        majorVersion = reader->readU16();
        minorVersion = reader->readU16();
        
        readConstantPool(); // TODO:2014-12-27:alex:Use pointers instead of u30's?
        
#define read_array(afield) \
  count = reader->readVU30(); \
  afield.resize(count); \
  for(uint32_t i = 0; i < count; ++i)
#define value(afield) afield[i]
        u30_t count;
        
        read_array(methods) {
          MethodInfo &info = *(value(methods) = std::unique_ptr<MethodInfo>(new MethodInfo()));
          info.file = this;
          info.paramCount = u30;
          info.returnType = u30_multiname;
          
          info.paramTypes.resize(info.paramCount);
          for(uint32_t j = 0; j < info.paramCount; ++j)
            info.paramTypes[j] = u30_multiname;
          
          info.name = u30_string;
          info.flags = (MethodInfo::Flags)reader->readU8();
          
          u30_t optionCount = info.flags & MethodInfo::HasOptionalFlag ? u30 : 0;
          info.options.resize(optionCount);
          for(uint32_t j = 0; j < optionCount; ++j) {
            info.options[j].value = u30;
            info.options[j].kind = (ConstantKind::Enum)reader->readU8();
          }
          
          u30_t nameCount = info.flags & MethodInfo::HasParamNamesFlag ? info.paramCount : 0;
          info.paramNames.resize(nameCount);
          for(uint32_t j = 0; j < nameCount; ++j)
            info.paramNames[j] = u30_string;
        }
        
        read_array(metadata) readMetadata(*(value(metadata) = std::unique_ptr<Metadata>(new Metadata())));
        
        read_array(classes) {
          Class &klass = *(value(classes) = std::unique_ptr<Class>(new Class()));
          klass.file = this;
          
          InstanceInfo &iinfo = klass.iinfo;
          iinfo.name = u30_multiname;
          iinfo.superName = u30_multiname;
          iinfo.flags = (InstanceInfo::Flags)reader->readU8();
          iinfo.protectedNs = iinfo.flags & InstanceInfo::ClassProtectedNsFlag ? u30_namespace : NULL;
          
          u30_t intrfCount = u30;
          iinfo.interfaces.resize(intrfCount);
          for(uint32_t j = 0; j < intrfCount; ++j)
            iinfo.interfaces[j] = u30_multiname;
          
          iinfo.initializer = u30_method;
          
          readTraits(iinfo.traits);
        }
        
        for(size_t i = 0, j = classes.size(); i < j; ++i) {
          ClassInfo &cinfo = classes[i]->cinfo;
          cinfo.initializer = u30_method;
          readTraits(cinfo.traits);
        }
        
        read_array(scripts) {
          ScriptInfo &info = *(value(scripts) = std::unique_ptr<ScriptInfo>(new ScriptInfo()));
          info.initializer = u30_method;
          readTraits(info.traits);
        }
        
        read_array(methodBodies) {
          MethodBody &body = *(value(methodBodies) = std::unique_ptr<MethodBody>(new MethodBody()));
          body.method = u30_method;
          body.maxStack = u30;
          body.localCount = u30;
          body.initScopeDepth = u30;
          body.maxScopeDepth = u30;
          body.code = reader->readString(u30);
          
          body.method->body = &body;
          
          u30_t exceptionCount = u30;
          body.exceptions.resize(exceptionCount);
          for(uint32_t j = 0; j < exceptionCount; ++j) {
            ExceptionInfo &exc = body.exceptions[j];
            exc.from = u30;
            exc.to = u30;
            exc.target = u30;
            exc.excType = u30;
            exc.varName = u30;
          }
          
          readTraits(body.traits);
        }
        
        for(uint32_t i = 0; i < classes.size(); ++i) {
          InstanceInfo &klass = classes[i]->iinfo;
          printf("dynamic class %s extends %s {\n", _multiname(klass.name), _multiname(klass.superName));
          
          MethodInfo &staticConstructor = *classes[i]->cinfo.initializer;
          printf("  static ");
          decompile(staticConstructor, "");
          printf("\n");
          
          for(uint32_t j = 0; j < classes[i]->cinfo.traits.size(); ++j) {
            printf("  static ");
            printTrait(classes[i]->cinfo.traits[j].get());
          }
          
          MethodInfo &constructor = *klass.initializer;
          
          printf("  public ");
          decompile(constructor, klass.name->nameString());
          printf("\n");
          
          for(uint32_t j = 0; j < klass.traits.size(); ++j) {
            printf("  ");
            printTrait(klass.traits[j].get());
          }
          
          printf("}\n");
        }
        
#undef read_array
#undef value
      }
    };
  }
}

#endif /* defined(__jswf__ABCFile__) */