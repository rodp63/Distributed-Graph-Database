#include "DGDB.h"



using namespace std;

int main(int argc, char *argv[])
{
    DGDB db;
    if (argv[1][0] == 'C'){
        db.setMode('C');
        db.setClient();
        //db.setNode(argv[2]);
        db.setRelation(argv[2],argv[3]);
    }
    else if (argv[1][0] == 'S'){
        db.setMode('S');
        db.setServer();
        db.runServer();
    }
    else if (argv[1][0] == 'R'){
        db.setPort(atoi(argv[2]));
        db.setIp("127.0.0.1");
        db.setMainIp("127.0.0.1");
        db.setMainPort(50000);
        db.setMode('R');
        db.setRepository();
        db.runServer();

    }

}


/*
int action; // 1B CRUD                  C
int name_node_size;// 3B                3
char name_node[255];// VB               15151515 (DNI)
int number_of_attributes; // 2B         1
int number_of_relations;  // 3B         0

// attributes are optional  
int name_attribute_size;  //  3B        13
char name_attribute[255]; //  VB        Primer Nombre
int value_attribute_size; //  3B        5
char value_attribute[255]; // VB        Julio

// relations are optional
int node_relation_size;  // 3B
char node_relations[255];// VB

insert
tabla de Nodos
Julio
Fabiola

tabla de Relaciones 
Julio  |  Fabiola 

tabla atributos
Julio  | edad | 48
Julio  | grad Aca | Phd 
Julio  | Est. Civ. | Casado



9B  VB ?A  6B VB ?R 3B VB 
int action; // 1B CRUD                  C
int name_node_size;// 3B                3
int number_of_attributes; // 2B         1
int number_of_relations;  // 3B         0
char name_node[255];// VB               15151515 (DNI)

// attributes are optional  
int name_attribute_size;  //  3B        13
int value_attribute_size; //  3B        5
char name_attribute[255]; //  VB        Primer Nombre
char value_attribute[255]; // VB        Julio

// relations are optional
int node_relation_size;  // 3B
char node_relations[255];// VB

////////////////////////////////////////////

int action; //       1B CRUD                  R 
int query_node_size ; //  2B                  0
char query_node[99];  //  VB                  0
int  deep; //        1B                       3
int  leaf; // on/off 1B                       off  
int attributes; // on/off 1B                  1       //  se quiere extraer los atributos

int number_of_conditions;  // 2B              3

int  query_name_attribute_size;  //  3B       15
char query_name_attribute[255]; //  VB        Grado academico
int  operador; // 1=, 2>, 3<, ..... 4like     4
int  query_value_attribute_size; //  3B       2
char query_value_attribute[255]; // VB        MSc*
int  is_and; on/off // 1B                     AND

int  query_name_attribute_size;  //  3B       15
char query_name_attribute[255]; //  VB        Grado academico
int  operador; // 1=, 2>, 3<, ..... 4like     4
int  query_value_attribute_size; //  3B       2
char query_value_attribute[255]; // VB        Dr
int  is_and; on/off // 1B                     OR

int  query_name_attribute_size;  //  3B       15
char query_name_attribute[255]; //  VB        Grado academico
int  operador; // 1=, 2>, 3<, ..... 4like     4
int  query_value_attribute_size; //  3B       2
char query_value_attribute[255]; // VB        Dra
int  is_and; on/off // 1B



////////////////////////////////////////////
int action; //       1B CRUD                      U
int node_or_attribute; // on:node off:attibute    1
int  query_node_size ; //  2B                     5
char query_value_node[99];  //  VB                      Julio

int  set_node_size ; //  2B                       4
char set_value_node[99];  //  VB                        Omar

int query_attribute_size;  //          3B        15
int query_value_attribute_size; //  3B           Grado academico
int set_attribute_size;  //  3B        13        2 
int set_value_attribute_size; //  3B             Dr

////////////////////////////////////////////
int action; //       1B CRUD                      D
int node_or_attribute; // 1:node 2:attibute 3:R   1
int  query_node_size ; //  2B                     5
char query_value_node[99];  //  VB                      Julio

int query_attribute_o_R_size;  //          3B        15
int query_value_attribute_o_R_size; //  3B           Grado academico
*/