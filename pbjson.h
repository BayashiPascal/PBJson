// ============ PBJSON.H ================

#ifndef PBJSON_H
#define PBJSON_H

// ================= Include =================

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "pberr.h"
#include "gtree.h"

// ================= Define ==================

#define PBJSON_INDENT "  "
#define PBJSON_MAXLENGTHLBL 500
#define PBJSON_CONTEXTSIZE 10

// ================= Data structure ===================

#define JSONNode GenTreeStr
#define JSONArrayVal GSetStr
#define JSONArrayStruct GSetGenTreeStr

// ================ Functions declaration ====================

// Free the memory used by the JSON node 'that' and its subnodes
// The memory used by the label of each node is freed too
void JSONFree(JSONNode** that);

// Set the label of the JSON node 'that' to a copy of 'lbl'
#if BUILDMODE != 0
inline
#endif
void JSONSetLabel(JSONNode* const that, const char* const lbl);

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its value is a copy of 'val'
#if BUILDMODE != 0
inline
#endif
void _JSONAddPropStr(JSONNode* const that, const char* const key, 
  char* const val);

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its value is the JSON node 'val'
#if BUILDMODE != 0
inline
#endif
void _JSONAddPropObj(JSONNode* const that, const char* const key, 
  JSONNode* const val);

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its values are a copy of the values in the GSetStr 'set'
void _JSONAddPropArr(JSONNode* const that, const char* const key, 
  const GSetStr* const set);

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its values are the GenTreeStr in the GSetGenTreeStr 'set'
void _JSONAddPropArrObj(JSONNode* const that, const char* const key, 
  const GSetGenTreeStr* const set);

// Save the JSON 'that' on the stream 'stream'
// If 'compact' equals true save in compact form, else save in easily 
// readable form
// Return true if it could save, false else
bool JSONSave(const JSONNode* const that, FILE* const stream, 
  const bool compact);

// Load the JSON 'that' from the stream 'stream'
// Return true if it could load, false else
bool JSONLoad(JSONNode* const that, FILE* const stream);

// Load the JSON 'that' from the string 'str' seen as a stream
// Return true if it could load, false else
bool JSONLoadFromStr(JSONNode* const that, const char* const str);

// Save the JSON 'that' in the string 'str'
// If 'compact' equals true save in compact form, else save in easily 
// readable form
// Return true if it could save, false else
bool JSONSaveToStr(const JSONNode* const that, char* const str, 
  const size_t strLen, const bool compact);

// Return the JSONNode of the property with label 'lbl' of the 
// JSON 'that'
// If the property doesn't exist return NULL
JSONNode* JSONProperty(const JSONNode* const that, const char* const lbl);

// Add a copy of the value 'val' to the array of value 'that'
#if BUILDMODE != 0
inline
#endif
void JSONArrayValAdd(JSONArrayVal* const that, const char* const val);

// Free memory used by the static array of values 'that'
#if BUILDMODE != 0
inline
#endif
void JSONArrayValFlush(JSONArrayVal* const that);

// Wrapping of GenTreeStr functions
#define JSONCreate() ((JSONNode*)GenTreeStrCreate())
#define JSONLabel(Node) GenTreeData(Node)
#define JSONAppendVal(Key, Val) GenTreeAppendSubtree(Key, Val)
#define JSONProperties(JSON) GenTreeSubtrees(JSON)
#define JSONValue(JSON, Index) GenTreeSubtree(JSON, Index)
#define JSONGetNbValue(JSON) GSetNbElem(GenTreeSubtrees(JSON))

// Wrapping of GSetStr functions
#define JSONArrayValCreateStatic() GSetStrCreateStatic()

// Wrapping of GSetGenTreeStr functions
#define JSONArrayStructCreateStatic() GSetGenTreeStrCreateStatic()
#define JSONArrayStructAdd(Array, Value) GSetAppend(Array, Value)
#define JSONArrayStructFlush(Array) GSetFlush(Array)

// ================= Polymorphism ==================

#define JSONAddProp(Node, Key, Val) _Generic(Val, \
  char*: _JSONAddPropStr, \
  const char*: _JSONAddPropStr, \
  JSONNode*: _JSONAddPropObj, \
  const JSONNode*: _JSONAddPropObj, \
  GSetStr*: _JSONAddPropArr, \
  const GSetStr*: _JSONAddPropArr, \
  GSetGenTreeStr*: _JSONAddPropArrObj, \
  const GSetGenTreeStr*: _JSONAddPropArrObj, \
  default: PBErrInvalidPolymorphism) (Node, Key, Val)

// ================ Inliner ====================

#if BUILDMODE != 0
#include "pbjson-inline.c"
#endif


#endif
