//
//  Node.h
//  jswf
//
//  Created by Alexander Rath on 28.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#ifndef jswf_Node_h
#define jswf_Node_h

#include "Multiname.h"
#include "MethodInfo.h"

#include <string>
#include <vector>

namespace jswf {
  namespace avm2 {
    namespace ast {
      /**
       * Serves as super-class for all nodes.
       */
      class Node : public std::enable_shared_from_this<Node> {
      public:
        int precedence;
        
        Node(int precedence) : precedence(precedence) {}
        virtual ~Node() {};
        virtual std::string toString() = 0;
        
        std::string coerce_s() { return toString(); }
        Namespace coerce_ns() {
          Namespace ns;
          ns.kind = NamespaceKind::NormalNamespaceKind;
          ns.name = toString();
          return ns;
        }
        
        virtual std::string toIntendedString(int intend) {
          std::string pre = "";
          for(int i = 0; i < intend; ++i) pre += "  ";
          return pre + toString(); // TODO:2014-12-28:alex:Awkward solution.
        }
        
        std::shared_ptr<Node> getProperty(Multiname &mn);
        std::shared_ptr<Node> setProperty(Multiname &mn, std::shared_ptr<Node> &value);
      };
      
      typedef std::shared_ptr<Node> NodePtr;
      
      /**
       * Describes a node that carries an integer literal.
       */
      class IntNode : public Node {
      public:
        int value;
        IntNode(int v) : value(v), Node(0) {}
        
        virtual std::string toString() {
          return std::to_string(value);
        }
      };
      
      /**
       * Describes a node that carries a string literal.
       */
      class StringNode : public Node {
      public:
        std::string value;
        StringNode(const std::string &str) : value(str), Node(0) {}
        
        virtual std::string toString() {
          return "\"" + value + "\"";
        }
      };
      
      class ConstantNode : public Node {
      public:
        std::string value;
        ConstantNode(const std::string &str) : value(str), Node(0) {}
        
        virtual std::string toString() {
          return value;
        }
      };
      
      /**
       * Describes a node that carries a double literal.
       */
      class DoubleNode : public Node {
      public:
        double value;
        DoubleNode(double dbl) : value(dbl), Node(0) {}
        
        virtual std::string toString() {
          return std::to_string(value);
        }
      };
      
  #define wrap(obj) (obj->precedence > precedence ? "(" + obj->toString() + ")" : obj->toString())
      
      /**
       * Describes a monadic operation.
       */
      class MonadicNode : public Node {
      public:
        NodePtr operand;
        std::string op;
        MonadicNode(NodePtr operand, std::string op, int p) : operand(operand), op(op), Node(p) {}
        
        virtual std::string toString() {
          return op + wrap(operand);
        }
      };
      
      /**
       * Describes a diadic operation.
       */
      class DiadicNode : public Node {
      public:
        NodePtr left, right;
        std::string op;
        DiadicNode(NodePtr right, NodePtr left, std::string op, int p) : left(left), right(right), op(op), Node(p) {}
        
        virtual std::string toString() {
          return wrap(left) + " " + op + " " + wrap(right);
        }
      };
      
      /**
       * Describes a property.
       */
      class PropNode : public Node {
      public:
        Multiname multiname;
        PropNode(const Multiname &mn) : multiname(mn), Node(0) {}
        
        virtual std::string toString() {
          return multiname.nameString();
        }
      };
      
      /**
       * Describes.
       * @todo What is this here for?!
       */
      class VoidNode : public Node {
      public:
        VoidNode() : Node(0) {}
        virtual std::string toString() {
          return "( error )";
        }
      };
      
      /**
       * Describes a property retrieved from an object.
       * @todo Why not combine this with object?
       */
      class AttrNode : public PropNode {
      public:
        NodePtr obj;
        AttrNode(NodePtr obj, const Multiname &mn) : obj(obj), PropNode(mn) {}
        
        virtual std::string toString() {
          std::string out = dynamic_cast<VoidNode *>(obj.get()) ? "" : wrap(obj);
          out += (out.empty() ? "" : ".") + multiname.nameString();
          return out;
        }
      };
      
      /**
       * Describes a return statement.
       */
      class ReturnNode : public Node {
      public:
        NodePtr obj; //!< NULL for returnvoid.
        ReturnNode(NodePtr obj) : obj(obj), Node(0) {}
        
        virtual std::string toString() {
          if(obj == NULL) return "return";
          return "return " + obj->toString();
        }
      };
      
      /**
       * Describes a call to a function / method.
       */
      class CallNode : public Node {
      public:
        NodePtr method;
        std::vector<NodePtr> arguments;
        CallNode(NodePtr method, std::vector<NodePtr> args) : method(method), arguments(args), Node(0) {}
        
        virtual std::string toString() {
          std::string out = wrap(method) + "(";
          for(uint32_t i = 0; i < arguments.size(); ++i) {
            if(i) out += ", ";
            out += arguments[i]->toString();
          }
          
          return out + ")";
        }
      };
      
      /**
       * Describes an array literal.
       */
      class ArrayNode : public Node {
      public:
        std::vector<NodePtr> arguments;
        ArrayNode(std::vector<NodePtr> args) : arguments(args), Node(0) {}
        
        virtual std::string toString() {
          std::string out = "[ ";
          for(uint32_t i = 0; i < arguments.size(); ++i) {
            if(i) out += ", ";
            out += arguments[i]->toString();
          }
          
          return out + " ]";
        }
      };
      
      /**
       * Describes a hash literal.
       */
      class ObjectNode : public Node {
      public:
        std::vector<std::pair<NodePtr, NodePtr>> arguments;
        ObjectNode(std::vector<std::pair<NodePtr, NodePtr>> args) : arguments(args), Node(0) {}
        
        virtual std::string toString() {
          std::string out = "{ ";
          for(uint32_t i = 0; i < arguments.size(); ++i) {
            if(i) out += ", ";
            out += arguments[i].first->toString();
            out += ":";
            out += arguments[i].second->toString();
          }
          
          return out + " }";
        }
      };
      
      /**
       * Describes a cast to a primitive type.
       * @todo What about other types?
       */
      class PrimitiveCastNode : public Node {
      public:
        NodePtr value;
        std::string type;
        PrimitiveCastNode(NodePtr value, std::string type) : value(value), type(type), Node(1) {}
        
        virtual std::string toString() {
          return "(" + type + ")" + wrap(value);
        }
      };
      
      /**
       * Describes an activation object.
       * @todo What is this?
       */
      class ActivationNode : public Node {
      public:
        ActivationNode() : Node(0) {}
        virtual std::string toString() {
          return "#";
        }
      };
      
      /**
       * Describes a local register (<tt>this</tt>, arguments and local variables).
       */
      class LocalNode : public Node {
      public:
        bool isTemporary = false;
        
        u30_t index;
        std::string name;
        
        LocalNode(u30_t index, MethodInfo &info) : index(index), Node(0) {
          bool needsArgs = info.flags & MethodInfo::NeedsArgumentsFlag;
          
          if(index == 0) name = "this";
          else if(index <= info.paramCount) name = "arg" + std::to_string(index);
          else if(index == info.paramCount + 1 && needsArgs) name = "arguments";
          else {
            isTemporary = true;
            
            u30_t localIndex = index;
            localIndex -= info.paramCount;
            if(needsArgs) --localIndex;
            name = "loc" + std::to_string(localIndex);
          }
        }
        
        virtual std::string toString() {
          return name;
        }
      };
      
      /**
       * Describes a node that contains statements.
       * @todo Make a statement node?
       */
      class CompoundNode : public Node {
      public:
        CompoundNode *parent;
        std::vector<NodePtr> body;
        
        CompoundNode(int precedence) : Node(precedence) {}
        
        virtual std::string toString() {
          return toIntendedString(0);
        }
        
        virtual std::string toIntendedString(int intend) {
          std::string out = "";
          for(auto it = body.begin(); it != body.end(); ++it) {
            Node &node = **it;
            out += node.toIntendedString(intend) + ";\n";
          }
          
          return out;
        }
      };
      
      /**
       * Describes a declaration of a local variable.
       * @todo This should be <tt>DeclarationNode</tt>
       */
      class DefinitionNode : public Node {
      public:
        NodePtr localNode;
        DefinitionNode(NodePtr n) : localNode(n), Node(0) {}
        
        virtual std::string toString() {
          return "var " + ((LocalNode *)localNode.get())->name + ":*";
        }
      };
      
      /**
       * Describes a function literal.
       */
      class FunctionNode : public CompoundNode {
      public:
        MethodInfo *method;
        std::string name;
        
        FunctionNode(MethodInfo *method, std::string name) : method(method), name(name), CompoundNode(1) {}
        
        virtual std::string toString() {
          return toIntendedString(0);
        }
        
        virtual std::string toIntendedString(int intend) {
          std::string out = "function" + (name.empty() ? "" : " " + name) + "(";
          for(uint32_t k = 0; k < method->paramCount; ++k) {
            if(k) out += ", ";
            out += "arg" + std::to_string(k+1);
            out += ":" + method->paramTypes[k]->nameString();
          } out += "):" + method->returnType->nameString();
          return out;
          
          out += " {\n";
          out += CompoundNode::toIntendedString(intend + 1);
          return out + "}";
        }
      };
      
      /**
       * Describes a <tt>with</tt> statement.
       */
      class WithNode : public CompoundNode {
      public:
        NodePtr value;
        WithNode(NodePtr value) : value(value), CompoundNode(0) {}
        
        virtual std::string toString() {
          return toIntendedString(0);
        }
        
        virtual std::string toIntendedString(int p) {
          std::string pre = "";
          for(int i = 0; i < p; ++i) pre += "  ";
          return pre + "with(" + value->toString() + ") {\n" + CompoundNode::toIntendedString(p + 1) + "}";
        }
      };
      
      class SuperNode : public Node {
      public:
        std::vector<NodePtr> arguments;
        SuperNode(std::vector<NodePtr> args) : arguments(args), Node(0) {}
        
        virtual std::string toString() {
          std::string argString = "";
          for(uint32_t i = 0; i < arguments.size(); ++i) {
            if(i) argString += ", ";
            argString += arguments[i]->toString();
          }
          
          return "super(" + argString + ")";
        }
      };
      
      /**
       * Describes a comment statement.
       */
      class CommentNode : public Node {
      public:
        std::string comment;
        CommentNode(std::string comment) : comment(comment), Node(0) {}
        virtual std::string toString() {
          return "// " + comment;
        }
      };
    }
  }
}

#endif