# PBJson
PBJson is a C library providing structures and functions to encode and decode structure data into JSON format.

An example is given below to show how the user can use PBJson to implement encoding and decoding functions of his/her data structures. Structures can include sub-structures recursively. Values can be atomic values (converted into string), array of atomic values, sub-structures, and array of sub-structures. The encoding can be done in a compact form (no indentation and no line return), or a readable form (indentation and line return). The decoding supports both compact and readable form. Keys and values are delimited by double quote (") and values can include double quote by escaping them with an anti-slash (\textbackslash). The library has the two folllowing limitations: key's label cannot starts with "[]", and the value must be less than 500 characters long.

```
// Declare two structures for example
struct structB {
  int _intVal;
};

struct structA {
  int _intVal;
  int _intArr[3];
  struct structB _structVal;
  struct structB _structArr[2];
};

// Function which return the JSON encoding of the structB 'that' 
JSONNode* StructBEncodeAsJSON(struct structB* that) {

  // Create the JSON structure
  JSONNode* json = JSONCreate();

  // Declare a buffer to convert value into string
  char val[100];

  // Convert the value into string
  sprintf(val, "%d", that->_intVal);

  // Add a property to the JSON
  JSONAddProp(json, "_intVal", val);

  // Return the JSON
  return json;
}

// Function which return the JSON encoding of the structA 'that' 
JSONNode* StructAEncodeAsJSON(struct structA* that) {

  // Create the JSON structure
  JSONNode* json = JSONCreate();

  // Declare a buffer to convert value into string
  char val[100];

  // Convert a int value into string
  sprintf(val, "%d", that->_intVal);
  // Add the property to the JSON
  JSONAddProp(json, "_intVal", val);

  // Declare an array of values converted to string
  JSONArrayVal setVal = JSONArrayValCreateStatic();
  // For each int value in the array
  for (int i = 0; i < 3; ++i) {
    // Convert the int value into string
    sprintf(val, "%d", that->_intArr[i]);
    // Add the string to the array
    JSONArrayValAdd(&setVal, val);
  }
  // Add the array of values to the JSON
  JSONAddProp(json, "_intArr", &setVal);

  // Add a key with an encoded structure as value
  JSONAddProp(json, "_structVal", 
    StructBEncodeAsJSON(&(that->_structVal)));
  
  // Declare an array of structures converted to string
  JSONArrayStruct setStruct = JSONArrayStructCreateStatic();
  // Add the two encoded structures to the array
  JSONArrayStructAdd(&setStruct, StructBEncodeAsJSON(that->_structArr));
  JSONArrayStructAdd(&setStruct, 
    StructBEncodeAsJSON(that->_structArr + 1));
  // Add a key with the array of structures
  JSONAddProp(json, "_structArr", &setStruct);

  // Free memory
  JSONArrayStructFlush(&setStruct);
  JSONArrayValFlush(&setVal);
  
  // Return the created JSON 
  return json;
}

// Function which save the structA 'that' on the stream 'stream'
// If 'compact' equals true it saves in compact form, else it saves in 
// readable form
void StructASave(struct structA* that, FILE* stream, bool compact) {

  // Get the JSON encoding of 'that'
  JSONNode* json = StructAEncodeAsJSON(that);
  
  // Save the JSON
  if (JSONSave(json, stream, compact) == false) {
    // ... manage the error
  }

  // Free memory
  JSONFree(&json);
}

// Function which decode from JSON encoding 'json' to the structB 'that'
bool StructBDecodeAsJSON(struct structB* that, JSONNode* json) {

  // Get the property _intVal from the JSON
  JSONNode* prop = JSONProperty(json, "_intVal");
  if (prop == NULL) {
    // ... manage the error
  }

  // Set the value of _intVal
  JSONNode* val = JSONValue(prop, 0);
  that->_intVal = atoi(JSONLabel(val));

  // Return the success code
  return true;
}

// Function which decode from JSON encoding 'json' to the structA 'that'
bool StructADecodeAsJSON(struct structA* that, JSONNode* json) {

  // Get the property _intVal from the JSON
  JSONNode* prop = JSONProperty(json, "_intVal");
  if (prop == NULL) {
    // ... manage the error
  }

  // Set the value of _intVal
  JSONNode* val = JSONValue(prop, 0);
  that->_intVal = atoi(JSONLabel(val));

  // Get the property _intArr from the JSON
  prop = JSONProperty(json, "_intArr");
  if (prop == NULL) {
    // ... manage the error
  }

  // Set the values of _intArr
  for (int i = 0; i < JSONGetNbValue(prop); ++i) {
    JSONNode* val = JSONValue(prop, i);
    that->_intArr[i] = atoi(JSONLabel(val));
  }

  // Get the property _structVal from the JSON
  prop = JSONProperty(json, "_structVal");
  if (prop == NULL) {
    // ... manage the error
  }

  // Decode the values of the sub struct
  if (StructBDecodeAsJSON(&(that->_structVal), prop) == false) {
    // ... manage the error
  }

  // Get the property _structArr from the JSON
  prop = JSONProperty(json, "_structArr");
  if (prop == NULL) {
    // ... manage the error
  }

  // Decode the values of _structArr
  for (int i = 0; i < JSONGetNbValue(prop); ++i) {
    JSONNode* val = JSONValue(prop, i);
    if (StructBDecodeAsJSON(that->_structArr + i, val) == false) {
      // ... manage the error
    }
  }

  // Return the success code
  return true;
}

// Function which load from the stream 'stream' containing the JSON 
// encoding of the structA 'that'
void StructALoad(struct structA* that, FILE* stream) {

  // Declare a json to load the encoded data
  JSONNode* json = JSONCreate();

  // Load the whole encoded data
  if (JSONLoad(json, stream) == false) {
    // ... manage the error
  }

  // Decode the data from the JSON to the structA
  if (!StructADecodeAsJSON(that, json)) {
    // ... manage the error
  }

  // Free the memory used by the JSON
  JSONFree(&json);
}

// Create an instance of structA for example
struct structA myStruct;
myStruct._intVal = 1;
myStruct._intArr[0] = 2;
myStruct._intArr[1] = 3;
myStruct._intArr[2] = 4;
myStruct._structVal._intVal = 5;
myStruct._structArr[0]._intVal = 6;
myStruct._structArr[1]._intVal = 7;

// Save the structure in JSON encoding on the standard output stream
// in readable form
bool compact = false;
StructASave(&myStruct, stdout, compact);

// Load the structure in JSON encoding from the standard input stream
StructALoad(&myStruct, stdin);

// Result:

{
  "_intVal":"1",
  "_intArr":["2","3","4"],
  "_structVal":{
    "_intVal":"5"
  },
  "_structArr":[
    {
      "_intVal":"6"
    },
    {
      "_intVal":"7"
    }
  ]
}
```
