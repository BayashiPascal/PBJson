#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "pberr.h"
#include "pbjson.h"

#define RANDOMSEED 0

void UnitTestJSONCreateFree() {
  JSONNode* json = JSONCreate();
  if (json == NULL) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONCreate failed");
    PBErrCatch(JSONErr);
  }
  JSONFree(&json);
  if (json != NULL) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONFree failed");
    PBErrCatch(JSONErr);
  }
  printf("UnitTestJSONCreateFree OK\n");
}

void UnitTestJSONSetGet() {
  JSONNode* json = JSONCreate();
  char* lbl = "testlabel";
  JSONSetLabel(json, lbl);
  if (strcmp(lbl, JSONLabel(json)) != 0) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONSetLabel failed");
    PBErrCatch(JSONErr);
  }
  char* key = "key";
  char* val = "val";
  JSONAddProp(json, key, val);
  if (strcmp(key, JSONLabel(GenTreeSubtree(json, 0))) != 0 ||
    strcmp(val, 
      JSONLabel(GenTreeSubtree(GenTreeSubtree(json, 0), 0))) != 0) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONAddProp failed");
    PBErrCatch(JSONErr);
  }
  JSONNode* prop = JSONCreate();
  JSONAddProp(prop, key, val);
  char* propkey = "propkey";
  JSONAddProp(json, propkey, prop);
  if (strcmp(propkey, JSONLabel(prop)) != 0 ||
    GenTreeSubtree(json, 1) != prop) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONAddProp failed");
    PBErrCatch(JSONErr);
  }
  JSONFree(&json);
  printf("UnitTestJSONSetGet OK\n");
}

struct structB {
  int _intVal;
  float _floatVal;
};

struct structA {
  int _intVal;
  int _intArr[3];
  struct structB _structVal;
  struct structB _structArr[2];
};

JSONNode* StructBEncodeAsJSON(struct structB* that) {
  // Create the JSON structure
  JSONNode* json = JSONCreate();
  // Declare a buffer to convert value into string
  char val[100];
  // Convert the value into string
  sprintf(val, "%d", that->_intVal);
  // Add a key/value to the JSON
  JSONAddProp(json, "_intVal", val);
  // Convert the value into string
  sprintf(val, "%f", that->_floatVal);
  // Add a key/value to the JSON
  JSONAddProp(json, "_floatVal", val);
  // Return the JSON
  return json;
}

JSONNode* StructAEncodeAsJSON(struct structA* that) {
  // Create the JSON structure
  JSONNode* json = JSONCreate();
  // Declare a buffer to convert value into string
  char val[100];

  // Add a property with empty value to the JSON
  val[0] = '\0';
  JSONAddProp(json, "_emptyVal", val);

  // Convert a int value into string
  sprintf(val, "%d", that->_intVal);
  // Add the property to the JSON
  JSONAddProp(json, "_intVal", val);

  // Add a property with a value containing a double quote to the JSON
  sprintf(val, "\\\"double quoted\\\"");
  JSONAddProp(json, "_escapeVal", val);

  // Declare an array of values converted to string
  JSONArrayVal setVal = JSONArrayValCreateStatic();
  // Create buffer for the conversion of int values into strinng
  char valInt[100];
  // For each int value in the array
  for (int i = 0; i < 3; ++i) {
    // Convert the int value into string
    sprintf(valInt, "%d", that->_intArr[i]);
    // Add the string to the array
    JSONArrayValAdd(&setVal, valInt);
  }
  // Add a property with the array of values to the JSON
  JSONAddProp(json, "_intArr", &setVal);

  // Empty the array
  JSONArrayValFlush(&setVal);
  // Add a property with an empty array of value
  JSONAddProp(json, "_emptyArr", &setVal);

  // Put back one value in the array
  JSONArrayValAdd(&setVal, valInt);
  // Add a property with an array of only one value
  JSONAddProp(json, "_oneIntArr", &setVal);

  // Add a property with an encoded structure as value
  JSONAddProp(json, "_structVal", 
    StructBEncodeAsJSON(&(that->_structVal)));
  
  // Declare an array of structures converted to string
  JSONArrayStruct setStruct = JSONArrayStructCreateStatic();
  // Add the two encoded structures to the array
  JSONArrayStructAdd(&setStruct, StructBEncodeAsJSON(that->_structArr));
  JSONArrayStructAdd(&setStruct, 
    StructBEncodeAsJSON(that->_structArr + 1));
  // Add a property with the array of structures
  JSONAddProp(json, "_structArr", &setStruct);

  // Free memory
  JSONArrayStructFlush(&setStruct);
  JSONArrayValFlush(&setVal);
  
  // Return the created JSON 
  return json;
}

void StructASave(struct structA* that, FILE* stream, bool compact) {
  // Get the JSON encoding of 'that'
  JSONNode* json = StructAEncodeAsJSON(that);
  
  // Save the JSON
  if (JSONSave(json, stream, compact) == false) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONSave failed");
    PBErrCatch(JSONErr);
  }

  // Free memory
  JSONFree(&json);
}

bool StructBDecodeAsJSON(struct structB* that, JSONNode* json) {
  // Get the property _intVal from the JSON
  JSONNode* prop = JSONProperty(json, "_intVal");
  if (prop == NULL) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "StructBDecodeAsJSON failed");
    PBErrCatch(JSONErr);
  }
  // Set the value of _intVal
  JSONNode* val = JSONValue(prop, 0);
  that->_intVal = atoi(JSONLabel(val));
  // Get the property _floatVal from the JSON
  prop = JSONProperty(json, "_floatVal");
  if (prop == NULL) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "StructBDecodeAsJSON failed");
    PBErrCatch(JSONErr);
  }
  // Set the value of _floatVal
  val = JSONValue(prop, 0);
  that->_floatVal = atof(JSONLabel(val));
  // Return the success code
  return true;
}

bool StructADecodeAsJSON(struct structA* that, JSONNode* json) {
  // Get the property _intVal from the JSON
  JSONNode* prop = JSONProperty(json, "_intVal");
  if (prop == NULL) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "StructADecodeAsJSON failed");
    PBErrCatch(JSONErr);
  }
  // Set the value of _intVal
  JSONNode* val = JSONValue(prop, 0);
  that->_intVal = atoi(JSONLabel(val));

  // Get the property _intArr from the JSON
  prop = JSONProperty(json, "_intArr");
  if (prop == NULL) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "StructADecodeAsJSON failed");
    PBErrCatch(JSONErr);
  }
  // Set the values of _intArr
  for (int i = 0; i < JSONGetNbValue(prop); ++i) {
    JSONNode* val = JSONValue(prop, i);
    that->_intArr[i] = atoi(JSONLabel(val));
  }

  // Get the property _structVal from the JSON
  prop = JSONProperty(json, "_structVal");
  if (prop == NULL) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "StructADecodeAsJSON failed");
    PBErrCatch(JSONErr);
  }
  // Decode the values of the sub struct
  if (StructBDecodeAsJSON(&(that->_structVal), prop) == false) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "StructBDecodeAsJSON failed");
    PBErrCatch(JSONErr);
  }

  // Get the property _structArr from the JSON
  prop = JSONProperty(json, "_structArr");
  if (prop == NULL) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "StructADecodeAsJSON failed");
    PBErrCatch(JSONErr);
  }
  // Decode the values of _structArr
  for (int i = 0; i < JSONGetNbValue(prop); ++i) {
    JSONNode* val = JSONValue(prop, i);
    if (StructBDecodeAsJSON(that->_structArr + i, val) == false) {
      JSONErr->_type = PBErrTypeUnitTestFailed;
      sprintf(JSONErr->_msg, "StructBDecodeAsJSON failed");
      PBErrCatch(JSONErr);
    }
  }

  // Return the success code
  return true;
}

void StructALoad(struct structA* that, FILE* stream) {
  // Declare a json to load the encoded data
  JSONNode* json = JSONCreate();
  // Load the whole encoded data
  if (JSONLoad(json, stream) == false) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONLoad failed");
    PBErrCatch(JSONErr);
  }
  // Decode the data from the JSON to the structA
  if (!StructADecodeAsJSON(that, json)) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "StructADecodeAsJSON failed");
    PBErrCatch(JSONErr);
  }
  // Free the memory used by the JSON
  JSONFree(&json);
}

void UnitTestJSONLoadSave() {
  struct structA myStruct;
  myStruct._intVal = 1;
  myStruct._intArr[0] = 2;
  myStruct._intArr[1] = 3;
  myStruct._intArr[2] = 4;
  myStruct._structVal._intVal = 5;
  myStruct._structVal._floatVal = 6.0;
  myStruct._structArr[0]._intVal = 7;
  myStruct._structArr[0]._floatVal = 8.0;
  myStruct._structArr[1]._intVal = 9;
  myStruct._structArr[1]._floatVal = 10.0;
  bool compact = false;
  printf("myStruct:\n");
  StructASave(&myStruct, stdout, compact);
  FILE* fd = fopen("./testJsonReadable.txt", "w");
  StructASave(&myStruct, fd, compact);
  fclose(fd);
  compact = true;
  fd = fopen("./testJsonCompact.txt", "w");
  StructASave(&myStruct, fd, compact);
  fclose(fd);

  struct structA myStructLoad;
  fd = fopen("./testJsonReadable.txt", "r");
  StructALoad(&myStructLoad, fd);
  fclose(fd);
  if (memcmp(&myStructLoad, &myStruct, sizeof(struct structA)) != 0) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONLoad failed");
    PBErrCatch(JSONErr);
  }

  char* array[3] = {"8","9","10"};
  JSONNode* json = JSONCreate();
  JSONArrayVal set = JSONArrayValCreateStatic();
  for (int i = 0; i < 3; ++i)
    JSONArrayValAdd(&set, array[i]);
  JSONAddProp(json, "", &set);
  JSONArrayValFlush(&set);
  printf("array:\n");
  if (!JSONSave(json, stdout, true)) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONSave failed");
    PBErrCatch(JSONErr);
  }
  fd = fopen("./testJsonArray.txt", "w");
  if (!JSONSave(json, fd, false)) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONSave failed");
    PBErrCatch(JSONErr);
  }
  fclose(fd);
  fd = fopen("./testJsonArray.txt", "r");
  JSONNode* jsonLoaded = JSONCreate();
  if (JSONLoad(jsonLoaded, fd) == false) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONLoad failed");
    PBErrCatch(JSONErr);
  }
  if (strcmp(JSONLabel(JSONValue(JSONValue(jsonLoaded, 0), 0)), 
    array[0]) != 0 ||
    strcmp(JSONLabel(JSONValue(JSONValue(jsonLoaded, 0), 1)), 
    array[1]) != 0 ||
    strcmp(JSONLabel(JSONValue(JSONValue(jsonLoaded, 0), 2)), 
    array[2]) != 0) {
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONLoad failed");
    PBErrCatch(JSONErr);
  }
  fclose(fd);
  JSONFree(&json);
  JSONFree(&jsonLoaded);
  JSONNode* jsonStr = JSONCreate();
  char* str = "{\"v\":\"1\"}";
  if (JSONLoadFromStr(jsonStr, str) == false){
    JSONErr->_type = PBErrTypeUnitTestFailed;
    sprintf(JSONErr->_msg, "JSONLoadFromStr failed");
    PBErrCatch(JSONErr);
  }
  JSONFree(&jsonStr);
  printf("UnitTestJSONLoadSave OK\n");
}

void UnitTestJSON() {
  UnitTestJSONCreateFree();
  UnitTestJSONSetGet();
  UnitTestJSONLoadSave();
  printf("UnitTestJSON OK\n");
}

void UnitTestAll() {
  UnitTestJSON();
  printf("UnitTestAll OK\n");
}

int main() {
  UnitTestAll();
  // Return success code
  return 0;
}

