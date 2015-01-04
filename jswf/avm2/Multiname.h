//
//  Multiname.h
//  jswf
//
//  Created by Alexander Rath on 29.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_Multiname_h
#define jswf_Multiname_h

#include "types.h"

namespace jswf {
  namespace avm2 {
    struct Multiname;
    typedef std::shared_ptr<Multiname> MultinamePtr;
    
    struct Multiname {
      enum Kind : u8_t {
        QNameKind       = 0x07,
        QNameAKind      = 0x0D,
        RTQNameKind     = 0x0F,
        RTQNameAKind    = 0x10,
        RTQNameLKind    = 0x11,
        RTQNameLAKind   = 0x12,
        MultinameKind   = 0x09,
        MultinameAKind  = 0x0E,
        MultinameLKind  = 0x1B,
        MultinameLAKind = 0x1C,
        
        InvalidKind = 0
      };
      
      bool isAttribute;
      bool hasName, hasNS, hasNSSet;
      
      string name;
      NamespacePtr ns;
      NamespaceSetPtr nsSet;
      
      Multiname(Kind kind) {
        hasName = hasNS = hasNSSet = false;
        
        switch(kind) {
          case QNameAKind: isAttribute = true;
          case QNameKind:  hasNS = hasName = true; break;
            
          case RTQNameAKind: isAttribute = true;
          case RTQNameKind:  hasName = true; break;
          
          case RTQNameLAKind: isAttribute = true;
          case RTQNameLKind:  break;
            
          case MultinameAKind: isAttribute = true;
          case MultinameKind:  hasName = hasNSSet = true; break;
            
          case MultinameLAKind: isAttribute = true;
          case MultinameLKind:  hasNSSet = true; break;
            
          case InvalidKind: break;
        }
      }
      
      bool operator ==(const Multiname &rhs) const {
        if(hasName  != rhs.hasName ||
           hasNS    != rhs.hasNS   ||
           hasNSSet != rhs.hasNSSet) return false;
        return (hasName && name == rhs.name) && (hasNS && *ns == *rhs.ns) && (hasNSSet && *nsSet == *rhs.nsSet);
      }
      
      void setName(std::string name) {
        hasName = true;
        this->name = name;
      }
      
      void setNS(NamespacePtr ns) {
        hasNS = true;
        this->ns = ns;
      }
      
      Kind getKind() const {
        // if(hasNS && hasNSSet) return InvalidKind;
        if(hasNS) {
          if(hasName) return isAttribute ? QNameAKind : QNameKind;
          return InvalidKind;
        } else if(hasNSSet) {
          if(hasName) return isAttribute ? MultinameAKind : MultinameKind;
          else return isAttribute ? MultinameLAKind : MultinameLKind;
        } else { // no ns, no nsSet
          if(hasName) return isAttribute ? RTQNameAKind : RTQNameKind;
          else return isAttribute ? RTQNameLAKind : RTQNameLKind;
        }
      }
      
      std::string nameString() const {
        if(!hasName) return "(late)";
        if(hasNS) return (ns->name.empty() ? "" : ns->name + ".") + name;
        return name;
      }
    };
  }
}

#endif