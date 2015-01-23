/**
 * @file
 * Documents the meaning of namespaces used throughout this software.
 */

//! Contains all members of jswf.
namespace jswf {
  //! Contains structures and classes described by the SWF specfication.
  namespace flash {
    //! Contains classes that describe instances of <tt>TAG</tt>.
    namespace tags {}
    
    //! Contains classes that describe line- and fill-styles.
    namespace styles {}
  }
  
  //! Contains classes to read/write primitives from/to streams.
  namespace io {}
  
  //! Contains structures and classes described by the AVM2 specification.
  namespace avm2 {
    //! Contains classes that represent the abstract syntax-tree for compilation / decompilation.
    namespace ast {}
    
    //! Contains native implementations of ActionScript 3.0 libraries.
    namespace lib {}
  }
  
  //! Contains structures and classes to render SWF shapes.
  namespace render {}
}

