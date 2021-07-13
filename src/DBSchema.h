#ifndef DGDB_DBSCHEMA_H_
#define DGDB_DBSCHEMA_H_

#include "thirdparty/sqlite_orm/sqlite_orm.h"

struct Condition {
  enum Operator {EQUAL, LESS, GREATER, LIKE};

  std::string key;
  std::string value;
  Operator op;
  bool is_or;
  
  Condition() {}
  Condition(const std::string& _key, const std::string& _value, char _op, char _lo)
      : key(_key), value(_value)  {
        is_or = _lo == '|' ? true : false;
        if (_op == '=') op = EQUAL;
        if (_op == '<') op = LESS;
        if (_op == '>') op = GREATER;
        if (_op == '%') op = LIKE;
      }
  std::string op_to_string() {
    if (op == EQUAL) return "=";
    if (op == LESS) return "<";
    if (op == GREATER) return ">";
    if (op == LIKE) return "%";
    return "~";
  }
  std::string is_or_to_string() {
    return is_or ? "|" : "&";
  }
};

struct Node {
  int id;
  std::string name;

  Node() {}
  explicit Node(const std::string& name) : name(name) {}
  explicit Node(int id, const std::string& name) : id(id), name(name) {}
};

struct Attribute {
  std::string node_name;
  std::string key;
  std::string value;

  Attribute() {}
  Attribute(const std::string& node_name, const std::string& key,
            const std::string& value) :
    node_name(node_name), key(key), value(value) {}
};

struct Relation {
  std::string node_name1;
  std::string node_name2;

  Relation() {}
  Relation(const std::string& node1, const std::string& node2)
    : node_name1(node1), node_name2(node2) {}
};

inline auto InitStorage(const std::string& path) {
  using namespace sqlite_orm;
  auto storage = make_storage(path,
                              make_table("Nodes",
                                         make_column("id",
                                             &Node::id,
                                             primary_key()),
                                         make_column("name",
                                             &Node::name,
                                             unique())),
                              make_table("Attributes",
                                         make_column("node_name",
                                             &Attribute::node_name),
                                         make_column("key",
                                             &Attribute::key),
                                         make_column("value",
                                             &Attribute::value)),
                              make_table("Relations",
                                         make_column("node_name1",
                                             &Relation::node_name1),
                                         make_column("node_name2",
                                             &Relation::node_name2)));

  storage.sync_schema();

  return storage;
}

#endif  // DGDB_DBSCHEMA_H_