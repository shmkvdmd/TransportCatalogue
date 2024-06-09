#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() = default;
    Node(std::nullptr_t)
        : value_(nullptr) {
    }
    Node(int val)
        : value_(val) {
    }
    Node(double val)
        : value_(val) {
    }
    Node(std::string val)
        : value_(std::move(val)) {
    }
    Node(Array val)
        : value_(std::move(val)) {
    }
    Node(Dict val)
        : value_(std::move(val)) {
    }
    Node(bool val)
        : value_(val) {
    }

    const Value& GetValue() const {
        return value_;
    }

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    const std::string& AsString() const;
    bool AsBool() const;
    double AsDouble() const;

    bool operator==(const Node& rhs);
    bool operator !=(const Node& rhs);

private:
    Value value_;
};

bool operator==(const Node& lhs, const Node& rhs);
bool operator !=(const Node& lhs, const Node& rhs);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& rhs);
    bool operator !=(const Document& rhs);

private:
    Node root_;
};

Document Load(std::istream& input);

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

void Print(const Document& doc, std::ostream& out);
void PrintNode(const Node& node, PrintContext& context);
template <typename Value>
void PrintValue(const Value& value, PrintContext& context);
void PrintValue(std::nullptr_t, PrintContext& context);
void PrintValue(Array arr, PrintContext& context);
void PrintValue(Dict dict, PrintContext& context);
void PrintValue(std::string str, PrintContext& context);
void PrintValue(bool value, PrintContext& context);

}  // namespace json