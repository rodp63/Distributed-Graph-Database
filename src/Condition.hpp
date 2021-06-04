#ifndef DISTRIBUTED_GRAPH_DATABASE_CONDITION_HPP_
#define DISTRIBUTED_GRAPH_DATABASE_CONDITION_HPP_

#include <iostream>

struct Conditional {

  int  query_name_attribute_size;
  std::string query_name_attribute;
  int  operador;
  int  query_value_attribute_size;
  std::string query_value_attribute;
  int  is_and;

};

#endif // DISTRIBUTED_GRAPH_DATABASE_CONDITION_HPP_