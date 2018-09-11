// ============ PBJSON-INLINE.C ================

// ================ Functions implementation ====================

// Set the label of the JSON node 'that' to a copy of 'lbl'
#if BUILDMODE != 0
inline
#endif
void JSONSetLabel(JSONNode* const that, const char* const lbl) {
#if BUILDMODE == 0
  if (that == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'that' is null");
    PBErrCatch(JSONErr);
  }
  if (lbl == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'lbl' is null");
    PBErrCatch(JSONErr);
  }
#endif
  // If the node already as a label
  if (JSONLabel(that) != NULL)
    // Free the label
    free(JSONLabel(that));
  // Allocate memory for the new label
  char* str = PBErrMalloc(JSONErr, sizeof(char) * (1 + strlen(lbl)));
  // Set the label copy
  strcpy(str, lbl);
  GenTreeSetData(that, str);
}

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its value is a copy of 'val'
#if BUILDMODE != 0
inline
#endif
void _JSONAddPropStr(JSONNode* const that, const char* const key, 
  char* const val) {
#if BUILDMODE == 0
  if (that == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'that' is null");
    PBErrCatch(JSONErr);
  }
  if (key == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'key' is null");
    PBErrCatch(JSONErr);
  }
  if (val == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'val' is null");
    PBErrCatch(JSONErr);
  }
#endif
  // Create a new node for the key
  JSONNode* nodeKey = JSONCreate();
  // Create a new node for the val
  JSONNode* nodeVal = JSONCreate();
  // Set the key and val label
  JSONSetLabel(nodeKey, key);
  JSONSetLabel(nodeVal, val);
  // Attach the val to the key
  JSONAppendVal(nodeKey, nodeVal);
  // Attach the new property to the node 'that'
  JSONAppendVal(that, nodeKey);
}

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its value is the JSON node 'val'
#if BUILDMODE != 0
inline
#endif
void _JSONAddPropObj(JSONNode* const that, const char* const key, 
  JSONNode* const val) {
#if BUILDMODE == 0
  if (that == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'that' is null");
    PBErrCatch(JSONErr);
  }
  if (key == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'key' is null");
    PBErrCatch(JSONErr);
  }
  if (val == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'val' is null");
    PBErrCatch(JSONErr);
  }
#endif
  // Set the key label for the node value
  JSONSetLabel(val, key);
  // Attach the value to the node 'that'
  JSONAppendVal(that, val);
}

// Add a copy of the value 'val' to the array of value 'that'
#if BUILDMODE != 0
inline
#endif
void JSONArrayValAdd(JSONArrayVal* const that, const char* const val) {
#if BUILDMODE == 0
  if (that == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'that' is null");
    PBErrCatch(JSONErr);
  }
  if (val == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'val' is null");
    PBErrCatch(JSONErr);
  }
#endif
  // Create a copy of the value
  char* lbl = PBErrMalloc(JSONErr, sizeof(char) * (1 + strlen(val)));
  strcpy(lbl, val);
  // Add the copy to the set
  GSetAppend(that, lbl);
}

// Free memory used by the static array of values 'that'
#if BUILDMODE != 0
inline
#endif
void JSONArrayValFlush(JSONArrayVal* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'that' is null");
    PBErrCatch(JSONErr);
  }
#endif
  // Free the memory used by the values
  while (GSetNbElem(that) > 0) {
    char* val = GSetPop(that);
    free(val);
  }
}
