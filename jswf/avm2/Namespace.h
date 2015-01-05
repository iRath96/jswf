//
//  Namespace.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_Namespace_h
#define jswf_Namespace_h

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
    
    struct Namespace;
    typedef std::shared_ptr<Namespace> NamespacePtr;
    
    struct Namespace {
      NamespaceKind::Enum kind;
      string name;
      
      bool operator ==(const Namespace &rhs) const {
        return kind == rhs.kind && name == rhs.name;
      }
      
      bool operator !=(const Namespace &rhs) const {
        return kind != rhs.kind || name != rhs.name;
      }
    };
    
    struct NamespaceSet;
    typedef std::shared_ptr<NamespaceSet> NamespaceSetPtr;
    
    struct NamespaceSet {
      std::vector<NamespacePtr> namespaces;
      bool operator ==(const NamespaceSet &rhs) const {
        return namespaces == rhs.namespaces;
      }
      
      bool operator !=(const NamespaceSet &rhs) const {
        return namespaces != rhs.namespaces;
      }
    };
  }
}

#endif