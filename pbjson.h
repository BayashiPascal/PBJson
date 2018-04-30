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

// ================= Data structure ===================

#define JSONNode GTreeStr
#define JSONArrayVal GSetStr
#define JSONArrayStruct GSetGTreeStr

// ================ Functions declaration ====================

// Free the memory used by the JSON node 'that' and its subnodes
// The memory used by the label of each node is freed too
void JSONFree(JSONNode** that);

// Set the label of the JSON node 'that' to a copy of 'lbl'
#if BUILDMODE != 0
inline
#endif
void JSONSetLabel(JSONNode* that, char* lbl);

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its value is a copy of 'val'
#if BUILDMODE != 0
inline
#endif
void _JSONAddPropStr(JSONNode* that, char* key, char* val);

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its value is the JSON node 'val'
#if BUILDMODE != 0
inline
#endif
void _JSONAddPropObj(JSONNode* that, char* key, JSONNode* val);

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its values are a copy of the values in the GSetStr 'set'
void _JSONAddPropArr(JSONNode* that, char* key, GSetStr* set);

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its values are the GTreeStr in the GSetGTreeStr 'set'
void _JSONAddPropArrObj(JSONNode* that, char* key, GSetGTreeStr* set);

// Save the JSON 'that' on the stream 'stream'
// If 'compact' equals true save in compact form, else save in easily 
// readable form
// Return true if it could save, false else
bool JSONSave(JSONNode* that, FILE* stream, bool compact);

// Load the JSON 'that' from the stream 'stream'
// Return true if it could load, false else
bool JSONLoad(JSONNode* that, FILE* stream);

// Return the JSONNode of the property with label 'lbl' of the 
// JSON 'that'
// If the property doesn't exist return NULL
JSONNode* JSONProperty(JSONNode* that, char* lbl);

// Add a copy of the value 'val' to the array of value 'that'
#if BUILDMODE != 0
inline
#endif
void JSONArrayValAdd(JSONArrayVal* that, char* val);

// Free memory used by the static array of values 'that'
#if BUILDMODE != 0
inline
#endif
void JSONArrayValFlush(JSONArrayVal* that);

// Wrapping of GTreeStr functions
#define JSONCreate() ((JSONNode*)GTreeStrCreate())
#define JSONLabel(Node) GTreeData(Node)
#define JSONAppendVal(Key, Val) GTreeAppendSubtree(Key, Val)
#define JSONProperties(JSON) GTreeSubtrees(JSON)
#define JSONValue(JSON, Index) GTreeSubtree(JSON, Index)
#define JSONGetNbValue(JSON) GSetNbElem(GTreeSubtrees(JSON))

// Wrapping of GSetStr functions
#define JSONArrayValCreateStatic() GSetStrCreateStatic()

// Wrapping of GSetGTreeStr functions
#define JSONArrayStructCreateStatic() GSetGTreeStrCreateStatic()
#define JSONArrayStructAdd(Array, Value) GSetAppend(Array, Value)
#define JSONArrayStructFlush(Array) GSetFlush(Array)

// ================= Polymorphism ==================

#define JSONAddProp(Node, Key, Val) _Generic(Val, \
  char*: _JSONAddPropStr, \
  JSONNode*: _JSONAddPropObj, \
  GSetStr*: _JSONAddPropArr, \
  GSetGTreeStr*: _JSONAddPropArrObj, \
  default: PBErrInvalidPolymorphism) (Node, Key, Val)

// ================ Inliner ====================

#if BUILDMODE != 0
#include "pbjson-inline.c"
#endif


#endif
