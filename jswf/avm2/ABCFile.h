//
//  ABCFile.h
//  jswf
//
//  Created by Alexander Rath on 27.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef __jswf__ABCFile__
#define __jswf__ABCFile__

#include <vector>
#include <cmath>
#include <stack>

#include "stdlib.h"
#include "GenericReader.h"
#include "types.h"

namespace jswf {
  namespace avm2 {
    struct NamespaceKind {
      enum Enum : u8_t {
        NormalNamespaceKind = 0x08,
        PackageNamespaceKind = 0x16,
        PackageInternalNsKind = 0x17,
        ProtectedNamespaceKind = 0x18,
        ExplicitNamespaceKind = 0x19,
        StaticProtectedNsKind = 0x1a,
        PrivateNamespaceKind = 0x05
      };
    };
    
    struct Namespace {
      NamespaceKind::Enum kind;
      string *name;
    };
    
    struct NamespaceSet {
      std::vector<u30_t> namespaces;
    };
    
    struct MultinameBase {
      enum Kind : u8_t {
        QNameKind = 0x07,
        QNameAKind = 0x0D,
        RTQNameKind = 0x0F,
        RTQNameAKind = 0x10,
        RTQNameLKind = 0x11,
        RTQNameLAKind = 0x12,
        MultinameKind = 0x09,
        MultinameAKind = 0x0E,
        MultinameLKind = 0x1B,
        MultinameLAKind = 0x1C
      };
      
      Kind kind;
      bool isAttribute;
      
      bool hasNS, hasName;
      MultinameBase(bool hasNS, bool hasName) : hasNS(hasNS), hasName(hasName) {}
      
      virtual string nameString() { return "(late)"; }
      virtual ~MultinameBase() {}
    };
    
    struct QName : MultinameBase {
      Namespace *ns;
      string *name;
      
      QName() : MultinameBase(true, true) {}
      virtual string nameString() { return (ns->name->empty() ? "" : *ns->name + ".") + *name; }
    };
    
    struct RTQName : MultinameBase {
      string *name;
      
      RTQName() : MultinameBase(false, true) {}
      virtual string nameString() { return name->empty() ? "*" : *name; }
    };
    
    struct RTQNameL : MultinameBase {
      RTQNameL() : MultinameBase(false, false) {}
    };
    
    struct Multiname : MultinameBase {
      string *name;
      NamespaceSet *nsSet;
      
      Multiname() : MultinameBase(true, true) {}
      
      virtual string nameString() { return *name; }
    };
    
    struct MultinameL : MultinameBase {
      NamespaceSet *nsSet;
      MultinameL() : MultinameBase(true, false) {}
    };
    
    struct ConstantPool {
      std::vector<s32_t> integers;
      std::vector<u32_t> uintegers;
      std::vector<d64_t> doubles;
      std::vector<string> strings;
      std::vector<Namespace> namespaces;
      std::vector<NamespaceSet> namespaceSets;
      std::vector<std::shared_ptr<MultinameBase>> multinames;
    };
    
    struct ConstantKind : NamespaceKind {
      enum Enum : u8_t {
        IntKind = 0x03,
        UIntKind = 0x04,
        DoubleKind = 0x06,
        UTF8Kind = 0x01,
        
        // constants
        
        FalseKind = 0x0a,
        TrueKind = 0x0b,
        NullKind = 0x0c,
        UndefinedKind = 0x00,
      };
    };
    
    struct OptionDetail {
      u30_t value;
      ConstantKind::Enum kind;
    };
    
    struct MethodBody;
    struct MethodInfo {
      enum Flags : u8_t {
        NeedsArgumentsFlag = 0x01,
        NeedsActivationFlag = 0x02,
        NeedsRestFlag = 0x04,
        HasOptionalFlag = 0x08,
        SetDxnsFlag = 0x40,
        HasParamNamesFlag = 0x80
      };
      
      u30_t paramCount; // Hey, you can pass up to 1,073,741,823 parameters to a function!
      MultinameBase *returnType;
      std::vector<MultinameBase *> paramTypes;
      
      string *name;
      Flags flags;
      
      std::vector<OptionDetail> options;
      std::vector<string *> paramNames;
      
      MethodBody *body = NULL;
    };
    
    struct MetadataItem {
      string *key, *value;
    };
    
    struct Metadata {
      string *name;
      std::vector<MetadataItem> items;
    };
    
    struct TraitInfo {
      enum Kind : u8_t {
        SlotKind = 0,
        MethodKind = 1,
        GetterKind = 2,
        SetterKind = 3,
        ClassKind = 4,
        FunctionKind = 5,
        ConstKind = 6
      };
      
      enum Attributes : u8_t {
        FinalAttribute = 0x1,
        OverrideAttribute = 0x2,
        MetadataAttribute = 0x4
      };
      
      MultinameBase *name;
      
      Kind kind;
      Attributes attributes;
      
      // u8_t data[]
      
      std::vector<u30_t> metadata;
      
      virtual ~TraitInfo() {}
    };
    
    struct SlotTraitInfo : TraitInfo { // kind = 0, 6
      u30_t slotId;
      MultinameBase *typeName;
      
      u30_t vindex;
      ConstantKind::Enum vkind;
    };
    
    typedef u30_t class_index_t;
    typedef u30_t method_index_t;
    
    struct ClassInfo;
    struct ClassTraitInfo : TraitInfo { // kind = 4
      u30_t slotId;
      ClassInfo *classInfo;
    };
    
    struct FunctionTraitInfo : TraitInfo { // kind = 5
      u30_t slotId;
      MethodInfo *methodInfo;
    };
    
    struct MethodTraitInfo : TraitInfo { // kind = 1, 2, 3
      u30_t dispId;
      MethodInfo *methodInfo;
    };
    
    struct InstanceInfo {
      enum Flags : u8_t {
        ClassSealedFlag = 0x01,
        ClassFinalFlag = 0x02,
        ClassInterfaceFlag = 0x04,
        ClassProtectedNsFlag = 0x08
      };
      
      MultinameBase *name, *superName;
      Flags flags;
      Namespace *protectedNs;
      std::vector<MultinameBase *> interfaces;
      u30_t iinit;
      std::vector<std::shared_ptr<TraitInfo>> traits;
    };
    
    struct ClassInfo {
      u30_t cinit;
      std::vector<std::shared_ptr<TraitInfo>> traits;
    };
    
    struct ScriptInfo {
      u30_t init;
      std::vector<std::shared_ptr<TraitInfo>> traits;
    };
    
    struct ExceptionInfo {
      u30_t from, to;
      u30_t target;
      u30_t excType;
      u30_t varName;
    };
    
    struct MethodBody {
      MethodInfo *method;
      
      u30_t maxStack, localCount;
      u30_t initScopeDepth, maxScopeDepth;
      string code;
      std::vector<ExceptionInfo> exceptions;
      std::vector<std::shared_ptr<TraitInfo>> traits;
    };
    
    // Hallelujah.
    
    class CompoundNode;
    class ABCFile {
    public:
      u16_t majorVersion, minorVersion;
      ConstantPool constantPool;
      std::vector<MethodInfo> methods;
      std::vector<Metadata> metadata;
      std::vector<InstanceInfo> instances;
      std::vector<ClassInfo> classes;
      std::vector<ScriptInfo> scripts;
      std::vector<MethodBody> methodBodies;
      
      std::shared_ptr<io::GenericReader> reader;
      ABCFile(std::shared_ptr<io::GenericReader> reader) : reader(reader) { read(); }
      
#define u30 reader->readVU30()
#define u30_string &constantPool.strings[u30]
#define u30_namespace &constantPool.namespaces[u30]
#define u30_ns_set &constantPool.namespaceSets[u30]
#define u30_multiname constantPool.multinames[u30].get()
#define u30_class &classes[u30]
#define u30_method &methods[u30]
      
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
        
        constantPool.integers[0] = 0;
        constantPool.uintegers[0] = 0;
        constantPool.doubles[0] = nan("");
        
        read_array(strings) {
          u30_t length = reader->readVU30();
          value(strings) = reader->readString(length);
        }
        
        constantPool.strings[0] = "";
        
        read_array(namespaces) {
          value(namespaces).kind = (NamespaceKind::Enum)reader->readU8();
          value(namespaces).name = u30_string;
        }
        
        constantPool.namespaces[0].kind = NamespaceKind::NormalNamespaceKind;
        constantPool.namespaces[0].name = 0;
        
        read_array(namespaceSets) {
          u30_t count = u30;
          value(namespaceSets).namespaces.resize(count);
          for(uint32_t j = 0; j < count; ++j) // Sadly, read_array cannot be nested.
            value(namespaceSets).namespaces[j] = reader->readVU30();
        }
        
        read_array(multinames) {
          u8_t kind = reader->readU8();
          MultinameBase *m = NULL;
          
          switch(kind) {
            case MultinameBase::QNameKind:
            case MultinameBase::QNameAKind: {
              QName *r = new QName();
              r->ns = u30_namespace;
              r->name = u30_string;
              r->isAttribute = kind == MultinameBase::QNameAKind;
              
              m = r;
            }; break;
            case MultinameBase::RTQNameKind:
            case MultinameBase::RTQNameAKind: {
              RTQName *r = new RTQName();
              r->name = u30_string;
              r->isAttribute = kind == MultinameBase::RTQNameAKind;
              
              m = r;
            }; break;
            case MultinameBase::RTQNameLKind:
            case MultinameBase::RTQNameLAKind: {
              RTQNameL *r = new RTQNameL();
              r->isAttribute = kind == MultinameBase::RTQNameLAKind;
              m = r;
            }; break;
            case MultinameBase::MultinameKind:
            case MultinameBase::MultinameAKind: {
              Multiname *r = new Multiname();
              r->name = u30_string;
              r->nsSet = u30_ns_set;
              r->isAttribute = kind == MultinameBase::MultinameAKind;
              m = r;
            }; break;
            case MultinameBase::MultinameLKind:
            case MultinameBase::MultinameLAKind: {
              MultinameL *r = new MultinameL();
              r->nsSet = u30_ns_set;
              r->isAttribute = kind == MultinameBase::MultinameLAKind;
              m = r;
            }; break;
            default: {
              throw "Oh no!";
            }; break;
          }
          
          m->kind = (MultinameBase::Kind)kind;
          value(multinames) = std::shared_ptr<MultinameBase>(m);
        }
        
        RTQName *any = new RTQName();
        any->name = &constantPool.strings[0];
        constantPool.multinames[0].reset(any);
        
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
      
      void readTraits(std::vector<std::shared_ptr<TraitInfo>> &traits) {
        u30_t traitCount = u30;
        traits.resize(traitCount);
        for(uint32_t i = 0; i < traitCount; ++i) {
          MultinameBase *name = u30_multiname;
          uint8_t attrAndKind = reader->readU8();
          
          TraitInfo::Kind kind = (TraitInfo::Kind)(attrAndKind & 0x0f);
          TraitInfo::Attributes attributes = (TraitInfo::Attributes)(attrAndKind >> 4);
          
          TraitInfo *trait = NULL;
          
          switch(kind) {
            case TraitInfo::SlotKind:
            case TraitInfo::ConstKind: {
              SlotTraitInfo *r = new SlotTraitInfo();
              r->slotId = u30;
              r->typeName = u30_multiname;
              r->vindex = u30;
              r->vkind = (ConstantKind::Enum)(r->vindex == 0 ? 0 : reader->readU8());
              trait = r;
            }; break;
            case TraitInfo::ClassKind: {
              ClassTraitInfo *r = new ClassTraitInfo();
              r->slotId = u30;
              r->classInfo = u30_class;
              trait = r;
            }; break;
            case TraitInfo::FunctionKind: {
              FunctionTraitInfo *r = new FunctionTraitInfo();
              r->slotId = u30;
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
          
          trait->name = name;
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
      void decompile(MethodInfo &info, std::string name, CompoundNode *compound);
      
#define _multiname(u) u->nameString().c_str()
      
      void printTrait(TraitInfo *trait) {
        NamespaceKind::Enum kind = dynamic_cast<QName *>(trait->name)->ns->kind;
        
        switch(kind) {
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
            printf(":%s;\n", _multiname(t->typeName));
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
          MethodInfo &info = value(methods);
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
        
        read_array(metadata) readMetadata(value(metadata));
        
        count = reader->readVU30();
        instances.resize(count);
        for(uint32_t i = 0; i < count; ++i) {
          InstanceInfo &info = instances[i];
          info.name = u30_multiname;
          info.superName = u30_multiname;
          info.flags = (InstanceInfo::Flags)reader->readU8();
          info.protectedNs = info.flags & InstanceInfo::ClassProtectedNsFlag ? u30_namespace : NULL;
          
          u30_t intrfCount = u30;
          info.interfaces.resize(intrfCount);
          for(uint32_t j = 0; j < intrfCount; ++j)
            info.interfaces[j] = u30_multiname;
          
          info.iinit = u30;
          
          readTraits(info.traits);
        }
        
        classes.resize(count);
        for(uint32_t i = 0; i < count; ++i) {
          ClassInfo &info = classes[i];
          info.cinit = u30;
          readTraits(info.traits);
        }
        
        read_array(scripts) {
          ScriptInfo &info = value(scripts);
          info.init = u30;
          readTraits(info.traits);
        }
        
        read_array(methodBodies) {
          MethodBody &body = value(methodBodies);
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
        
        const char *codes[16];
        for(uint32_t i = 0; i < methodBodies.size(); ++i)
          codes[i] = methodBodies[i].code.c_str();
        
        for(uint32_t i = 0; i < instances.size(); ++i) {
          InstanceInfo &klass = instances[i];
          printf("dynamic class %s extends %s {\n", _multiname(klass.name), _multiname(klass.superName));
          
          MethodInfo &staticConstructor = methods[classes[i].cinit];
          printf("  static ");
          decompile(staticConstructor, "");
          printf("\n");
          
          for(uint32_t j = 0; j < classes[i].traits.size(); ++j) {
            printf("  static ");
            printTrait(classes[i].traits[j].get());
          }
          
          MethodInfo &constructor = methods[klass.iinit];
          
          printf("  public ");
          decompile(constructor, klass.name->nameString());
          printf("\n");
          
          for(uint32_t j = 0; j < klass.traits.size(); ++j) {
            printf("  ");
            printTrait(klass.traits[j].get());
          }
          
          printf("}\n");
        }
        
        exit(1);
#undef read_array
#undef value
      }
    };
  }
}

#endif /* defined(__jswf__ABCFile__) */