// Copyright 2021 <The Three Musketeers UCSP>
#ifndef DMP_NODE_HPP_
#define DMP_NODE_HPP_

#include <memory>
#include <string>
#include <vector>
#include "Attribute.hpp"

struct Node {
  std::string name;
  std::vector<Attribute> attributes;
  std::vector<std::shared_ptr<Node>> relations;
};

#endif  // NODE_HPP_
