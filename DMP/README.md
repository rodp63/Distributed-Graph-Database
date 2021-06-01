# CRUD PROTOCOL
---
### Create
```
int action;               // 1B         C
int name_node_size;       // 3B         3
char name_node[255];      // VB         15151515 (DNI)
int number_of_attributes; // 2B         1
int number_of_relations;  // 3B         0
```
_Attributes are optional_
```
int name_attribute_size;  //  3B        13
char name_attribute[255]; //  VB        Primer Nombre
int value_attribute_size; //  3B        5
char value_attribute[255];//  VB        Julio
```
_Relations are optional_
```
int node_relation_size;  // 3B
char node_relations[255];// VB
```
---
### Read
```
int action;                // 1B              R 
int query_node_size ;      // 2B              0
char query_node[99];       //  VB             0
int  deep;                 //  1B             3
int  leaf;                 // on/off 1B       off
int attributes;            // on/off 1B       on
int number_of_conditions;  // 2B              3
```
_Conditions are optional_
```
int  query_name_attribute_size;  // 3B        15
char query_name_attribute[255];  // VB        Grado academico
int  operador;                   // 1B        4
int  query_value_attribute_size; // 3B        2
char query_value_attribute[255]; // VB        MSc*
int  is_and; on/off              // 1B        AND
```
```
int  query_name_attribute_size;  // 3B        15
char query_name_attribute[255];  // VB        Grado academico
int  operador;                   // 1B        4
int  query_value_attribute_size; // 3B        2
char query_value_attribute[255]; // VB        Dr*
int  is_and; on/off              // 1B
```
---
### Update
```
int action;                // 1B                      U
int node_or_attribute;     // on:node off:attibute    1
int  query_node_size ;     // 2B                      5
char query_value_node[99]; // VB                      Julio
```
_If node_
```
int  set_node_size ;       // 2B              4
char set_value_node[99];   // VB              Omar
```
_If attribute_
```
int query_attribute_size;       // 3B         15
int query_value_attribute_size; // 3B         Grado academico
int set_attribute_size;         // 3B         2
int set_value_attribute_size;   // 3B         Dr
```
---
### Delete
```
int action;                 // 1B                      D
int node_or_attribute;      // 1:node 2:attibute 3:R   1
int  query_node_size ;      // 2B                      5
char query_value_node[99];  // VB                      Julio
```
_If attribute_
```
int query_attribute_o_R_size;        // 3B     15
int query_value_attribute_o_R_size;  // 3B     Grado academico
```
