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

#include "Node.h"

using namespace jswf::avm2::ast;

void ABCFile::decompile(MethodInfo &info, std::string name) {
  CompoundNode compound(0);
  decompile(info, name, &compound);
  printf("%s", compound.toString().c_str());
}

void ABCFile::decompile(MethodInfo &info, std::string name, CompoundNode *compound) {
  CompoundNode *currentCompound = new FunctionNode(&info, name);
  compound->body.push_back(NodePtr(currentCompound));
  
  return;
  
#ifdef STUPID_CODE
  
#define __ncast(v) std::shared_ptr<Node>((Node *)v)
#define __push(v) stack.push(__ncast(v))
#define __pop (top = stack.top(), stack.pop(), top)
#define __output(v) currentCompound->body.push_back(__ncast(v)); \
//  printf("    %s;\n", output[output.size()-1]->toString().c_str());
  
#define bc_comment(str) if(printByteCode) __output(new CommentNode(str))
#define bc_force_comment(str) __output(new CommentNode(str))
  
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
      case 0x09: {
        bc_comment("label");
        // "Do nothing"
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
      case 0x21: {
        bc_comment("pushundefined");
        __push(new ConstantNode("undefined"));
      }; break;
      case 0x24: {
        uint8_t byte = codeReader.readU8();
        bc_comment("pushbyte " + std::to_string(byte));
        
        __push(new IntNode(byte));
      }; break;
      case 0x26: {
        bc_comment("pushtrue");
        __push(new ConstantNode("true"));
      }; break;
      case 0x27: {
        bc_comment("pushfalse");
        __push(new ConstantNode("false"));
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
      case 0x49: {
        u30_t argCount = codeReader.readVU30();
        bc_comment("constructsuper[" + std::to_string(argCount) + "]");
        
        std::vector<NodePtr> args;
        for(uint32_t i = 0; i < argCount; ++i) args.insert(args.begin(), __pop);
        
        __output(new SuperNode(args));
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
      default:
        bc_force_comment("[unknown] op_" + std::to_string(chr));
    }
  }
  
#endif
}