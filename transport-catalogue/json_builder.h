#pragma once

#include "json.h"

#include <optional>

namespace json {

class KeyItemContext;
class ArrayItemContext;
class DictItemContext;

class Builder {
public:
    Builder();
    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();
    Node GetNode(Node::Value value);

private:
    Node root_ = nullptr;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> current_key_;
};

class DictItemContext{
public:
    DictItemContext(Builder& builder);
    KeyItemContext Key(std::string key);
    Builder& EndDict();
private:
    Builder& builder_;
};

class ArrayItemContext{
public:
    ArrayItemContext(Builder& builder);
    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    Builder& EndArray();
    ArrayItemContext StartArray();
private:
    Builder& builder_;
};

class KeyItemContext{
public:
    KeyItemContext(Builder& builder);
    DictItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
private:
    Builder& builder_;
};

}