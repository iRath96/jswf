//
//  ABCFile.cpp
//  jswf
//
//  Created by Alexander Rath on 27.12.14.
//  Copyright (c) 2014 Alexander Rath. All rights reserved.
//

#include "ABCFile.h"
#include "StringReader.h"

#include <unordered_map>

using namespace jswf;
using namespace jswf::avm2;

namespace jswf {
  namespace avm2 {
    class Node {
    public:
      int precedence;
      
      Node(int precedence) : precedence(precedence) {}
      virtual ~Node() {};
      virtual std::string toString() = 0;
      
      virtual std::string toIntendedString(int intend) {
        std::string pre = "";
        for(int i = 0; i < intend; ++i) pre += "  ";
        return pre + toString(); // TODO:2014-12-28:alex:Awkward solution.
      }
    };

    typedef std::shared_ptr<Node> NodePtr;

    class IntNode : public Node {
    public:
      int value;
      IntNode(int v) : value(v), Node(0) {}
      
      virtual std::string toString() {
        return std::to_string(value);
      }
    };

    class StringNode : public Node {
    public:
      std::string value;
      StringNode(const std::string &str) : value(str), Node(0) {}
      
      virtual std::string toString() {
        return "\"" + value + "\"";
      }
    };

    class DoubleNode : public Node {
    public:
      double value;
      DoubleNode(double dbl) : value(dbl), Node(0) {}
      
      virtual std::string toString() {
        return std::to_string(value);
      }
    };

    #define wrap(obj) (obj->precedence > precedence ? "(" + obj->toString() + ")" : obj->toString())
    
    class MonadicNode : public Node {
    public:
      NodePtr operand;
      std::string op;
      MonadicNode(NodePtr operand, std::string op, int p) : operand(operand), op(op), Node(p) {}
      
      virtual std::string toString() {
        return op + wrap(operand);
      }
    };
    
    class DiadicNode : public Node {
    public:
      NodePtr left, right;
      std::string op;
      DiadicNode(NodePtr right, NodePtr left, std::string op, int p) : left(left), right(right), op(op), Node(p) {}
      
      virtual std::string toString() {
        return wrap(left) + " " + op + " " + wrap(right);
      }
    };

    class PropNode : public Node {
    public:
      NodePtr ns, name;
      MultinameBase *multiname;
      PropNode(MultinameBase *mn, NodePtr ns, NodePtr name) : multiname(mn), ns(ns), name(name), Node(0) {}
      
      virtual std::string toString() {
        std::string ns = "(namespace)"; // ???
        std::string out = "";
        if(multiname->hasName)
          out = multiname->nameString();
        else
          out = "this[" + name->toString() + "]";
        return out;
      }
    };

    class VoidNode : public Node {
    public:
      VoidNode() : Node(0) {}
      virtual std::string toString() {
        return "( error )";
      }
    };

    class AttrNode : public PropNode {
    public:
      NodePtr obj;
      AttrNode(NodePtr obj, MultinameBase *mn, NodePtr ns, NodePtr name) : obj(obj), PropNode(mn, ns, name) {}
      
      virtual std::string toString() {
        std::string ns = "(namespace)"; // ???
        std::string out = dynamic_cast<VoidNode *>(obj.get()) ? "" : wrap(obj);
        
        if(multiname->hasName)
          out += (out.empty() ? "" : ".") + multiname->nameString();
        else
          out += std::string(out.empty() ? "#notsure" : "") + "[" + name->toString() + "]";
        return out;
      }
    };

    class ReturnNode : public Node {
    public:
      NodePtr obj;
      ReturnNode(NodePtr obj) : obj(obj), Node(0) {}
      
      virtual std::string toString() {
        if(obj == NULL) return "return";
        return "return " + obj->toString();
      }
    };

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

    class PrimitiveCastNode : public Node {
    public:
      NodePtr value;
      std::string type;
      PrimitiveCastNode(NodePtr value, std::string type) : value(value), type(type), Node(1) {}
      
      virtual std::string toString() {
        return "(" + type + ")" + wrap(value);
      }
    };

    class ActivationNode : public Node {
    public:
      ActivationNode() : Node(0) {}
      virtual std::string toString() {
        return "#";
      }
    };

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

    class DefinitionNode : public Node {
    public:
      NodePtr localNode;
      DefinitionNode(NodePtr n) : localNode(n), Node(0) {}
      
      virtual std::string toString() {
        return "var " + ((LocalNode *)localNode.get())->name + ":*";
      }
    };

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
        } out += "):" + method->returnType->nameString() + " {\n";
        out += CompoundNode::toIntendedString(intend + 1);
        return out + "}";
      }
    };

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

void ABCFile::decompile(MethodInfo &info, std::string name) {
  CompoundNode compound(0);
  decompile(info, name, &compound);
  printf("%s", compound.toString().c_str());
}

void ABCFile::decompile(MethodInfo &info, std::string name, CompoundNode *compound) {
  CompoundNode *currentCompound = new FunctionNode(&info, name);
  compound->body.push_back(NodePtr(currentCompound));
  
#define __ncast(v) std::shared_ptr<Node>((Node *)v)
#define __push(v) stack.push(__ncast(v))
#define __pop (top = stack.top(), stack.pop(), top)
#define __output(v) currentCompound->body.push_back(__ncast(v)); \
//  printf("    %s;\n", output[output.size()-1]->toString().c_str());
  
#define bc_comment(str) if(printByteCode) __output(new CommentNode(str))
  bool printByteCode = 0;
  
  NodePtr top;
  
  std::stack<NodePtr> stack;
  
#define __needs_definition(localNode) (((LocalNode *)localNode)->isTemporary && \
  defMap.find(((LocalNode *)localNode)->index) == defMap.end())
#define __put_definition(localNode) (defMap[((LocalNode *)localNode)->index] = true)
  std::unordered_map<u30_t, bool> defMap;
  
  io::StringReader codeReader(info.body->code);
#define read_auto(afield, vname) auto vname = constantPool.afield[codeReader.readVU30()];
#define read_multiname_arg(index) \
  MultinameBase *multiname = constantPool.multinames[index].get(); \
  NodePtr name; if(!multiname->hasName) name = __pop; \
  NodePtr ns; if(!multiname->hasNS) ns = __pop;
#define read_multiname read_multiname_arg(codeReader.readVU30())
#define mn_cstr multiname->nameString().c_str()
#define mn_str multiname->nameString()
  
  while(!codeReader.eof()) {
    uint8_t chr = codeReader.readU8();
    switch(chr) {
      case 0x08: {
        u30_t i = codeReader.readVU30();
        bc_comment("kill " + std::to_string(i));
      }; break;
      case 0x1c: {
        WithNode *wn = new WithNode(__pop);
        wn->parent = currentCompound;
        
        bc_comment("pushwith");
        __output(wn);
        
        currentCompound = wn;
      }; break;
      case 0x1d: {
        // TODO:2014-12-27:alex:Scope stack!!!
        // TODO:2014-12-28:alex:Check if this was indeed a with-statement.
        
        bc_comment("popscope");
        currentCompound = currentCompound->parent;
      }; break;
      case 0x24: {
        uint8_t byte = codeReader.readU8();
        bc_comment("pushbyte " + std::to_string(byte));
        
        __push(new IntNode(byte));
      }; break;
      case 0x2a: {
        bc_comment("dup");
        
        NodePtr v = __pop;
        stack.push(v);
        stack.push(v);
      }; break;
      case 0x2c: {
        read_auto(strings, str);
        bc_comment("pushstring \"" + str + "\""); // TODO:2014-12-28:alex:Not correctly escaped.
        
        __push(new StringNode(str));
      }; break;
      case 0x2d: {
        read_auto(integers, i);
        bc_comment("pushint " + std::to_string(i));
        
        __push(new IntNode(i));
      }; break;
      case 0x2f: {
        read_auto(doubles, dbl);
        bc_comment("pushdouble " + std::to_string(dbl));
        
        __push(new DoubleNode(dbl));
      }; break;
      case 0x30: {
        bc_comment("pushscope");
        __pop; // TODO:2014-12-27:alex:And now what?
      }; break;
      case 0x40: {
        u30_t index = codeReader.readVU30();
        bc_comment("newfunction " + std::to_string(index));
        
        decompile(methods[index], "", currentCompound);
        NodePtr functionNode = currentCompound->body.back();
        currentCompound->body.pop_back();
        stack.push(functionNode);
      }; break;
      case 0x47: {
        bc_comment("returnvoid");
        __output(new ReturnNode(NULL));
      }; break;
      case 0x48: {
        bc_comment("returnvalue");
        __output(new ReturnNode(__pop));
      }; break;
      case 0x46:
      case 0x4f: {
        u30_t mnIndex = codeReader.readVU30();
        u30_t argCount = codeReader.readVU30();
        
        std::vector<NodePtr> args;
        for(uint32_t i = 0; i < argCount; ++i) args.insert(args.begin(), __pop);
        
        read_multiname_arg(mnIndex);
        
        NodePtr method((Node *)new AttrNode(__pop, multiname, ns, name));
        CallNode *node = new CallNode(method, args);
        
        bc_comment((chr == 0x46 ? "callproperty[" : "callpropvoid[") + std::to_string(argCount) + "] " + mn_str);
        
        if(chr == 0x46) __push(node);
        else __output(node);
      }; break;
      case 0x55: {
        u30_t argCount = codeReader.readVU30();
        
        std::vector<std::pair<NodePtr, NodePtr>> args;
        for(uint32_t i = 0; i < argCount; ++i) {
          NodePtr value = __pop;
          NodePtr name = __pop;
          args.insert(args.begin(), std::pair<NodePtr, NodePtr>(name, value));
        }
        
        bc_comment("newobject[" + std::to_string(argCount) + "]");
        __push(new ObjectNode(args));
      }; break;
      case 0x56: {
        u30_t argCount = codeReader.readVU30();
        
        std::vector<NodePtr> args;
        for(uint32_t i = 0; i < argCount; ++i) args.insert(args.begin(), __pop);
        
        bc_comment("newarray[" + std::to_string(argCount) + "]");
        __push(new ArrayNode(args));
      }; break;
      case 0x57: {
        bc_comment("newactivation");
        __push(new ActivationNode());
      }; break;
      case 0x5e:
      case 0x5d: {
        read_multiname;
        bc_comment((chr == 0x5e ? "findproperty " : "findpropstrict ") + mn_str);
        
        __push(new VoidNode()); //__push(new PropNode(multiname, ns, name));
      }; break;
      case 0x60: {
        read_multiname;
        bc_comment("getlex " + mn_str);
        
        // TODO:2014-12-27:alex:Verify that the name is qualified.
        __push(new PropNode(multiname, NULL, NULL));
      }; break;
      case 0x61: {
        NodePtr value = __pop;
        read_multiname;
        
        bc_comment("setproperty " + mn_str);
        
        NodePtr attr(new AttrNode(__pop, multiname, ns, name));
        __output(new DiadicNode(value, attr, "=", 20));
      }; break;
      case 0x66: {
        read_multiname;
        bc_comment("getproperty " + mn_str);
        
        __push(new AttrNode(__pop, multiname, ns, name));
      }; break;
      case 0x68: {
        NodePtr value = __pop;
        read_multiname;
        NodePtr object = __pop;
        
        bc_comment("initproperty " + mn_str);
        
        NodePtr prop = __ncast(new AttrNode(object, multiname, ns, name));
        __output(new DiadicNode(value, prop, "=", 20));
      }; break;
      case 0x73: {
        bc_comment("convert_i");
        __push(new PrimitiveCastNode(__pop, "int"));
      }; break;
      case 0x96: {
        bc_comment("not");
        __push(new MonadicNode(__pop, "!", 2));
      }; break;
      case 0xa0: {
        bc_comment("add");
        __push(new DiadicNode(__pop, __pop, "+", 60));
      }; break;
      case 0xa1: {
        bc_comment("subtract"); // TODO:2014-12-28:alex:Right-To-Left bug.
        __push(new DiadicNode(__pop, __pop, "-", 61));
      }; break;
      case 0xa2: {
        bc_comment("multiply");
        __push(new DiadicNode(__pop, __pop, "*", 40));
      }; break;
      case 0xa3: {
        bc_comment("divide");
        __push(new DiadicNode(__pop, __pop, "/", 41));
      }; break;
      case 0xab: {
        bc_comment("equals");
        __push(new DiadicNode(__pop, __pop, "==", 100));
      }; break;
      
      case 0x62: // getlocal with u30
      case 0xd0: case 0xd1: case 0xd2: case 0xd3: { // getlocal_<n>
        u30_t index = chr == 0x62 ? codeReader.readVU30() : chr - 0xd0;
        LocalNode *localNode = new LocalNode(index, info);
        NodePtr localNodePtr(localNode);
        
        bc_comment(std::string("getlocal") + (chr == 0x62 ? " " : "_") + std::to_string(index));
        
        if(__needs_definition(localNode)) {
          __output(new DefinitionNode(localNodePtr));
          __put_definition(localNode);
        }
        
        stack.push(localNodePtr);
      }; break;
        
      case 0x63: // setlocal with u30
      case 0xd4: case 0xd5: case 0xd6: case 0xd7: { // setlocal_<n>
        u30_t index = chr == 0x63 ? codeReader.readVU30() : chr - 0xd4;
        LocalNode *localNode = new LocalNode(index, info);
        NodePtr localNodePtr(localNode);
        
        NodePtr value = __pop;
        
        bc_comment(std::string("setlocal") + (chr == 0x63 ? " " : "_") + std::to_string(index));
        
        if(dynamic_cast<ActivationNode *>(value.get())) {
          if(__needs_definition(localNode)) {
            __output(new DefinitionNode(localNodePtr));
            __put_definition(localNode);
          }
        } else {
          if(__needs_definition(localNode)) {
            localNode->name = "var " + localNode->name + ":*";
            __put_definition(localNode);
          }
          
          __output(new DiadicNode(value, localNodePtr, "=", 20));
        }
      }; break;
      default: bc_comment("[unknown] op_" + std::to_string(chr));
    }
  }
}