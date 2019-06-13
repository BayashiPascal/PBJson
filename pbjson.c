// ============ PBJSON.C ================

// ================= Include =================

#include "pbjson.h"
#if BUILDMODE == 0
#include "pbjson-inline.c"
#endif

// ================ Functions implementation ====================

// Save recursively the JSON tree 'that' into the stream 'stream'
// Return true if it could save, false else
bool JSONSaveRec(const JSONNode* const that, FILE* const stream, 
  const bool compact, int depth);

// Return true if the JSON node 'that' is a value (ie its subtree is 
// empty)
inline bool JSONIsValue(JSONNode* const that);

// Scan the 'stream' char by char until the next significant char
// ie anything else than a space or a new line or a tab and store the 
// result in 'c'
// Return false if there has been an I/O error
inline bool JSONGetNextChar(FILE* stream, char* c);

// Load a struct in the JSON 'that' from the stream 'stream'
// Return true if it could load, false else
bool JSONLoadStruct(JSONNode* const that, FILE* stream);

// Load an array in the JSON 'that' from the stream 'stream'
// Return true if it could load, false else
bool JSONLoadArr(JSONNode* const that, FILE* stream, char* key);

// Load the string 'str' from the 'stream'
// Return false if there has been an I/O error
bool JSONLoadStr(FILE* stream, char* str);

// Load the array of values of property 'prop' in the JSON 'that' 
// Return true if it could load, false else
bool JSONAddArr(JSONNode* const that, char* prop, FILE* stream);

// Load the array of structs of property 'prop' in the JSON 'that' 
// Return true if it could load, false else
bool JSONAddArrStruct(JSONNode* const that, char* prop, FILE* stream);

// Get the characters around the current position in the 'stream'
void JSONGetContextStream(FILE* stream, char* buffer);

// ================ Functions implementation ====================

// Free the memory used by the JSON node 'that' and its subnodes
// The memory used by the label of each node is freed too
void JSONFree(JSONNode** that) {
  // Check arguments
  if (that == NULL || *that == NULL)
    // Nothing to do
    return;
  // Free all the char* in the tree
  if (JSONLabel(*that) != NULL)
    free(JSONLabel(*that));
  GenTreeIterDepth iter = GenTreeIterDepthCreateStatic((GenTreeStr*)(*that));
  if (!GenTreeIterIsLast(&iter)) {
    do {
      char* label = GenTreeIterGetData(&iter);
      free(label);
    } while (GenTreeIterStep(&iter));
  }
  GenTreeIterFreeStatic(&iter);
  // Free memory
  GenTreeFree(that);
}

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its values are a copy of the values in the GSetStr 'set'
void _JSONAddPropArr(JSONNode* const that, const char* const key, 
  const GSetStr* const set) {
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
  if (set == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'set' is null");
    PBErrCatch(JSONErr);
  }
#endif
  // Create a new node for the key
  JSONNode* nodeKey = JSONCreate();
  // Set the key label
  JSONSetLabel(nodeKey, key);
  int nbElem = GSetNbElem(set);
  if (nbElem > 0) {
    // For each val in the set
    GSetIterForward iter = GSetIterForwardCreateStatic(set);
    do {
      // Get the value
      char* val = GSetIterGet(&iter);
      // Create a new node for the val
      JSONNode* nodeVal = JSONCreate();
      // Set the val label
      JSONSetLabel(nodeVal, val);
      // Attach the val to the key
      JSONAppendVal(nodeKey, nodeVal);
    } while (GSetIterStep(&iter));
  }
  // Add empty nodes to ensure it has at least 1 nodes and is viewed 
  // as a property when saving 
  while (nbElem < 1) {
    // Create a new empty node 
    JSONNode* nodeVal = JSONCreate();
    // Attach the empty node to the key
    JSONAppendVal(nodeKey, nodeVal);
    ++nbElem;
  }
  // Attach the new property to the node 'that'
  JSONAppendVal(that, nodeKey);
}

// Add a property to the node 'that'. The property's key is a copy of a 
// 'key' and its values are the GenTreeStr in the GSetGenTreeStr 'set'
void _JSONAddPropArrObj(JSONNode* const that, const char* const key, 
  const GSetGenTreeStr* const set) {
  // Create a new node for the key
  JSONNode* nodeKey = JSONCreate();
  // Set the key label with '[]' as prefix
  char buffer[PBJSON_MAXLENGTHLBL + 3];
  buffer[0] = '[';buffer[1] = ']';
  sprintf(buffer + 2, "%s", key);
  JSONSetLabel(nodeKey, buffer);
  // GEt the number of value
  int nbElem = GSetNbElem(set);
  // If the array is not empty
  if (nbElem > 0) {
    // For each val in the set
    GSetIterForward iter = GSetIterForwardCreateStatic(set);
    do {
      // Get the value
      GenTreeStr* val = GSetIterGet(&iter);
      // Attach the val to the key
      JSONAppendVal(nodeKey, val);
    } while (GSetIterStep(&iter));
  }
  // Add empty nodes to ensure it has at least 1 nodes and is viewed 
  // as a property when saving 
  while (nbElem < 1) {
    // Create a new empty node 
    JSONNode* nodeVal = JSONCreate();
    // Attach the empty node to the key
    JSONAppendVal(nodeKey, nodeVal);
    ++nbElem;
  }
  // Attach the new property to the node 'that'
  JSONAppendVal(that, nodeKey);
}

// Function to add indentation in beautiful mode
inline bool JSONIndent(FILE* stream, int depth) {
  for (int i = depth; i--;)
    if (!PBErrPrintf(JSONErr, stream, "%s", PBJSON_INDENT))
      return false;
  return true;
}

// Return true if the JSON node 'that' is a value (ie its subtree is 
// empty)
inline bool JSONIsValue(JSONNode* const that) {
  return (GSetNbElem(GenTreeSubtrees(that)) == 0);
}

// Save the JSON 'that' in the string 'str'
// If 'compact' equals true save in compact form, else save in easily 
// readable form
// Return true if it could save, false else
bool JSONSaveToStr(const JSONNode* const that, char* const str, 
  const bool compact) {
#if BUILDMODE == 0
  if (that == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'that' is null");
    PBErrCatch(JSONErr);
  }
  if (str == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'str' is null");
    PBErrCatch(JSONErr);
  }
#endif
  // Open the string as a stream
  FILE* stream = fmemopen((void*)str, strlen(str), "w");
  
  // Save the JSON as with a normal stream
  bool ret = JSONSave(that, stream, compact);
  
  // Close the stream
  fclose(stream);
  
  // Return the success code
  return ret;
}

// Save the JSON 'that' on the stream 'stream'
// If 'compact' equals true save in compact form, else save in easily 
// readable form
// Return true if it could save, false else
bool JSONSave(const JSONNode* const that, FILE* const stream, 
  const bool compact) {
#if BUILDMODE == 0
  if (that == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'that' is null");
    PBErrCatch(JSONErr);
  }
  if (stream == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'stream' is null");
    PBErrCatch(JSONErr);
  }
#endif
  // Start the recursion at depth 0
  return JSONSaveRec(that, stream, compact, 0);
}

// Save recursively the JSON tree 'that' into the stream 'stream'
// Return true if it could save, false else
bool JSONSaveRec(const JSONNode* const that, FILE* const stream, 
  const bool compact, int depth) {
  // Declare a flag to memorize if the current node is a key for an 
  // array of object
  bool flagArrObj = false;
  // Declare a variable to memorize the opening and closing char
  char openChar[2] = "{";
  char closeChar[2] = "}";
  // Print the label of the property if it's not null
  if (JSONLabel(that) != NULL && strlen(JSONLabel(that)) > 0) {
    if (!compact && !JSONIndent(stream, depth)) 
      return false;
    char* lbl = JSONLabel(that);
    if (lbl[0] == '[' && lbl[1] == ']') {
      flagArrObj = true;
      lbl += 2;
      openChar[0] = '[';
      closeChar[0] = ']';
    }
    if (!PBErrPrintf(JSONErr, stream, "\"%s\":", lbl))
      return false;
  }
  // Loop on properties
  GSetIterForward iter = 
    GSetIterForwardCreateStatic(JSONProperties(that));
  // Get the first property
  JSONNode* firstProp = GSetIterGet(&iter);
  // Declare a flag to escape opening and closing bracket in case of a 
  // single array
  bool flagEscapeBracket = (depth == 0 && 
    GSetNbElem(JSONProperties(that)) == 1 && 
    (JSONLabel(firstProp) == NULL || strlen(JSONLabel(firstProp)) == 0));
  // Print the opening char if the first prop is not a value
  // It's enough to check on the first prop as the json is supposed
  // to be well formed, meaning if the first prop is not a value then
  // all the others too
  if (!JSONIsValue(firstProp) && !flagEscapeBracket) {
    if (!PBErrPrintf(JSONErr, stream, "%s", openChar))
      return false;
    if (!compact && !PBErrPrintf(JSONErr, stream, "%s", "\n")) 
      return false;
    if (!compact && flagArrObj && !JSONIndent(stream, depth + 1))
      return false;
  }
  // Declare a flag to manage comma between values
  bool flagComma = false;
  // Loop on properties
  do {
    // Get the property
    JSONNode* prop = GSetIterGet(&iter);
    // If it's not a value (ie not a leaf)
    if (!JSONIsValue(prop)) {
      // Save the property's values
      if (!JSONSaveRec(prop, stream, compact, depth + 1))
        return false;
      if (!GSetIterIsLast(&iter)) {
        if (!PBErrPrintf(JSONErr, stream, "%s", ","))
          return false;
      }
      if (!compact && !PBErrPrintf(JSONErr, stream, "%s", "\n")) 
        return false;
      if (!compact && flagArrObj && !GSetIterIsLast(&iter) && 
        !JSONIndent(stream, depth + 1))
        return false;
    // Else, it's a value
    } else {
      if (GSetNbElem(JSONProperties(that)) > 1 && GSetIterIsFirst(&iter))
        if (!PBErrPrintf(JSONErr, stream, "%s", "["))
          return false;
      if (flagComma) {
        if (!PBErrPrintf(JSONErr, stream, "%s", ","))
          return false;
      }
      if (JSONLabel(prop) != NULL) {
        if (!PBErrPrintf(JSONErr, stream, "\"%s\"", 
          JSONLabel(prop)))
          return false;
      } else {
        if (!PBErrPrintf(JSONErr, stream, "%s", "\"\""))
          return false;
      }
      flagComma = true;
      if (GSetNbElem(JSONProperties(that)) > 1 && GSetIterIsLast(&iter))
        if (!PBErrPrintf(JSONErr, stream, "%s", "]")) 
          return false;
    }
  } while (GSetIterStep(&iter));
  // Print the closing char if the first prop is not a value
  if (!JSONIsValue(firstProp) && !flagEscapeBracket) {
    if (!compact && !JSONIndent(stream, depth)) 
      return false;
    if (!PBErrPrintf(JSONErr, stream, "%s", closeChar)) 
      return false;
  }
  if (depth == 0 && !PBErrPrintf(JSONErr, stream, "%s", "\n"))
    return false;
  // Return the success code
  return true;
}

// Scan the 'stream' char by char until the next significant char
// ie anything else than a space or a new line or a tab or a comma 
// and store the result in 'c'
// Return false if there has been an I/O error
inline bool JSONGetNextChar(FILE* stream, char* c) {
  // Loop until the next significant char
  do {
    // If we coudln't read the next character
    if (fscanf(stream, "%c", c) == EOF) {
      JSONErr->_type = PBErrTypeIOError;
      sprintf(JSONErr->_msg, 
        "Premature end of file or fscanf error in JSONGetNextChar");
      return false;
    }
  } while (*c == ' ' || *c == '\n' || *c == '\t' || *c == ',');
  // Return the success code
  return true;
}

// Load the string 'str' from the 'stream'
// Return false if there has been an I/O error
bool JSONLoadStr(FILE* stream, char* str) {
  // Declare a variable ot memorize the position in the string
  int i = 0;
  // Declare a flag to manage escape character
  bool flagEsc = false;
  // Loop on character of the string
  do {
    // If the previous char was an escaped char 
    if (flagEsc && i > 0 && str[i - 1] == '"')
      // Reset the flag
      flagEsc = false;
    // Read one character
    if (fscanf(stream, "%c", str + i) == EOF) {
      JSONErr->_type = PBErrTypeIOError;
      sprintf(JSONErr->_msg, 
        "Premature end of file or fscanf error in JSONLoadStr");
      return false;
    }
    // If it's an escape char
    if (str[i] == '\\')
      // Set the flag
      flagEsc = true;
    // Increment the position in the string
    ++i;
  // Loop until the buffer is full or we reached the final double quote
  } while ((flagEsc || str[i - 1] != '"') && i < PBJSON_MAXLENGTHLBL);
  // Add the null character at the end of the string
  str[i - 1] = '\0';
  // Return the success code
  return true;
}

// Load the array of values of property 'prop' in the JSON 'that' 
// Return true if it could load, false else
bool JSONAddArr(JSONNode* const that, char* prop, FILE* stream) {
  // Declare the array of values
  JSONArrayVal set = JSONArrayValCreateStatic();
  // Declare a buffer for the value
  char bufferValue[PBJSON_MAXLENGTHLBL + 1] = {'\0'};
  // Declare a char to memorize the next significant char
  char c = '\0';
  // Loop on values
  do {
    // Load the value
    if (!JSONLoadStr(stream, bufferValue))
      return false;
    // Add the string to the array
    JSONArrayValAdd(&set, bufferValue);
    // Move to the next significant char
    if (!JSONGetNextChar(stream, &c))
      return false;
    // Check the next significant character is '"' or ']'
    if (c != '"' && c != ']') {
      JSONErr->_type = PBErrTypeInvalidData;
      char ctx[2 * PBJSON_CONTEXTSIZE + 1];
      JSONGetContextStream(stream, ctx);
      sprintf(JSONErr->_msg, 
        "JSONAddArr: Expected '\"' or ']' but found '%c' near ...%s...", 
        c, ctx);
      return false;
    }
  } while (c != ']');
  // Add the property to the JSON
  JSONAddProp(that, prop, &set);
  // Flush the array
  JSONArrayValFlush(&set);
  // Return the success code
  return true;
}

// Load the array of structs of property 'prop' in the JSON 'that' 
// Return true if it could load, false else
bool JSONAddArrStruct(JSONNode* const that, char* prop, FILE* stream) {
  // Declare the array of values
  JSONArrayStruct set = JSONArrayStructCreateStatic();
  // Declare a char to memorize the next significant char
  char c = '\0';
  // Loop on values
  do {
    // Allocate memory for the next object
    JSONNode* obj = JSONCreate();
    // Rewind one char as JSONLoad expect to read '{'
    if (fseek(stream, -1, SEEK_CUR) != 0) {
      JSONErr->_type = PBErrTypeIOError;
      sprintf(JSONErr->_msg, "fseek error in JSONAddArrStruct");
      return false;
    } 
    // Load the value
    if (!JSONLoad(obj, stream))
      return false;
    // Add the string to the array
    JSONArrayStructAdd(&set, obj);
    // Move the next significant char
    if (!JSONGetNextChar(stream, &c))
      return false;
    // check the next significant character is '{' or ']'
    if (c != '{' && c != ']') {
      JSONErr->_type = PBErrTypeInvalidData;
      char ctx[2 * PBJSON_CONTEXTSIZE + 1];
      JSONGetContextStream(stream, ctx);
      sprintf(JSONErr->_msg, 
        "JSONAddStruct: Expected '{' or ']' but found '%c' near ...%s...", 
        c, ctx);
      return false;
    }
  } while (c != ']');
  // Add the property to the JSON
  JSONAddProp(that, prop, &set);
  // Flush the array
  JSONArrayStructFlush(&set);
  // Return the success code
  return true;
}

// Load a key/value in the JSON 'that' from the stream 'stream'
// Return true if it could load, false else
bool JSONLoadProp(JSONNode* const that, FILE* stream) {
  // Declare a buffer to read the key
  char bufferKey[PBJSON_MAXLENGTHLBL + 1] = {'\0'};
  // Read the property's key
  if (!JSONLoadStr(stream, bufferKey))
    return false;
  // Read the next significant character which must be a ':'
  char c;
  if (!JSONGetNextChar(stream, &c))
    return false;
  if (c != ':') {
    JSONErr->_type = PBErrTypeInvalidData;
    char ctx[2 * PBJSON_CONTEXTSIZE + 1];
    JSONGetContextStream(stream, ctx);
    sprintf(JSONErr->_msg, 
      "JSONLoadProp: Expected ':' but found '%c' near ...%s...", 
      c, ctx);
    return false;
  }
  // Read the next significant character
  if (!JSONGetNextChar(stream, &c))
    return false;
  // If the next character is a double quote
  if (c == '"') {
    // Read the property's value
    char bufferVal[PBJSON_MAXLENGTHLBL + 1] = {'\0'};
    if (!JSONLoadStr(stream, bufferVal))
      return false;
    // Add the property to the JSON
    JSONAddProp(that, bufferKey, bufferVal);
  // Else, if the next character is a square bracket
  } else if (c == '[') {
    JSONLoadArr(that, stream, bufferKey);
  // Else, if the next character is an accolade
  } else if (c == '{') {
    // This property is an object
    // Create a new node for the object
    JSONNode* prop = JSONCreate();
    // Set the property name
    JSONSetLabel(prop, bufferKey);
    // Add the new node to the JSON
    JSONAppendVal(that, prop);
    // Load the object
    JSONLoadStruct(prop, stream);
  // Else, it's not a valid file
  } else {
    // Return the failure code
    JSONErr->_type = PBErrTypeInvalidData;
    char ctx[2 * PBJSON_CONTEXTSIZE + 1];
    JSONGetContextStream(stream, ctx);
    sprintf(JSONErr->_msg, 
      "JSONLoadProp: Expected '\"','{' or '[' but found '%c' near ...%s...", 
      c, ctx);
    return false;
  }
  // Return the success code
  return true;
}

// Get the characters around the current position in the 'stream'
void JSONGetContextStream(FILE* stream, char* buffer) {
  int pos = fseek(stream, -PBJSON_CONTEXTSIZE, SEEK_CUR);
  (void)pos;
  int nb = fread(buffer, sizeof(char), 2 * PBJSON_CONTEXTSIZE, stream);
  (void)nb;
  buffer[2 * PBJSON_CONTEXTSIZE] = '\0';
}

// Load a struct in the JSON 'that' from the stream 'stream'
// Return true if it could load, false else
bool JSONLoadStruct(JSONNode* const that, FILE* stream) {
  char c = '\0';
  // Loop until the end of the structure
  while (c != '}') {
    // Read the next significant character
    if (!JSONGetNextChar(stream, &c))
      return false;
    // If it's not the end of the struct
    if (c != '}') {
      // Load the pair key/value
      if (!JSONLoadProp(that, stream))
        return false;
    } 
  }
  // Return the success code
  return true;
}

// Load an array in the JSON 'that' from the stream 'stream'
// Return true if it could load, false else
bool JSONLoadArr(JSONNode* const that, FILE* stream, char* key) {
  // Declare a variable ot memorize the next significant char
  char c;
  // Read the next significant character
  if (!JSONGetNextChar(stream, &c))
    return false;
  // If the next character is a double quote
  if (c == '"') {
    // Load the array of value
    if (!JSONAddArr(that, key, stream))
      return false;
  // Else, if the next character is a closing square bracket
  } else if (c == ']') {
    // It's an empty array
    // Declare the empty array
    JSONArrayVal set = JSONArrayValCreateStatic();
    // Add the property to the JSON
    JSONAddProp(that, key, &set);
  // Else, if the next character is a bracket
  } else if (c == '{') {
    // This property is an array of structs
    // Load the array of structs
    if (!JSONAddArrStruct(that, key, stream))
      return false;
  // Else, it's not a valid file
  } else {
    // Return the failure code
    JSONErr->_type = PBErrTypeInvalidData;
    char ctx[2 * PBJSON_CONTEXTSIZE + 1];
    JSONGetContextStream(stream, ctx);
    sprintf(JSONErr->_msg, 
      "JSONLoadArr: Expected '\"' or '{' but found '%c' near ...%s...", 
      c, ctx);
    return false;
  }
  // Return the success code
  return true;
}

// Load the JSON 'that' from the stream 'stream'
// Return true if it could load, false else
bool JSONLoad(JSONNode* const that, FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'that' is null");
    PBErrCatch(JSONErr);
  }
  if (stream == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'stream' is null");
    PBErrCatch(JSONErr);
  }
#endif
  char c;
  // Read the first significant character
  if (!JSONGetNextChar(stream, &c))
    return false;
  // If the file starts with a '{'
  if (c == '{') {
    // The file contains a struct definion
    // Load the struct
    return JSONLoadStruct(that, stream);
  // Else if the file starts with a '['
  } else if (c == '[') {
    // The file contains an array 
    // Load the array
    return JSONLoadArr(that, stream, "");
  // Else, the file doesn't start with '{' or '['
  } else {
    // It's not a valid file, stop here
    JSONErr->_type = PBErrTypeInvalidData;
    char ctx[2 * PBJSON_CONTEXTSIZE + 1];
    JSONGetContextStream(stream, ctx);
    sprintf(JSONErr->_msg, 
      "JSONLoad: Expected '{' or '[' but found '%c' near ...%s...", 
      c, ctx);
    return false;
  }
  // Return the success code
  return true;
}

// Load the JSON 'that' from the string 'str' seen as a stream
// Return true if it could load, false else
bool JSONLoadFromStr(JSONNode* const that, const char* const str) {
#if BUILDMODE == 0
  if (that == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'that' is null");
    PBErrCatch(JSONErr);
  }
  if (str == NULL) {
    JSONErr->_type = PBErrTypeNullPointer;
    sprintf(JSONErr->_msg, "'str' is null");
    PBErrCatch(JSONErr);
  }
#endif
  // Open the string as a stream
  FILE* stream = fmemopen((void*)str, strlen(str), "r");
  
  // Load the JSON as with a normal stream
  bool ret = JSONLoad(that, stream);
  
  // Close the stream
  fclose(stream);
  
  // Return the success code
  return ret;
}

// Return the JSONNode of the property with label 'lbl' of the 
// JSON 'that'
// If the property doesn't exist return NULL
JSONNode* JSONProperty(const JSONNode* const that, 
  const char* const lbl) {
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
  // If the JSONNode has properties
  if (JSONGetNbValue(that) > 0) {
    // Declare an iterator on properties of the JSONNode
    GSetIterForward iter = 
      GSetIterForwardCreateStatic(JSONProperties(that));
    // Loop on properties
    do {
      // Get the property
      JSONNode* prop = GSetIterGet(&iter);
      // Skip the eventual '[]'
      char* propLbl = JSONLabel(prop);
      if (propLbl[0] == '[' && propLbl[1] == ']')
        propLbl += 2;
      // If the label of the property is the same as the searched
      // property
      if (strcmp(propLbl, lbl) == 0) {
        // Return the property
        return prop;
      }
    } while (GSetIterStep(&iter));
  }
  // If we reach here it means the searched property doesn't exist
  return NULL;
}

