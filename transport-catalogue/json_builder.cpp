#include "json_builder.h"
#include <exception>

using namespace std::literals;

namespace json {

Builder::Builder() : root_(), nodes_stack_{&root_} {}

Node Builder::Build(){
    if(root_.IsNull() || nodes_stack_.size() > 1){
        throw std::logic_error("Attempt to build JSON which isn't initialized");
    }
    return std::move(root_);
}

KeyItemContext Builder::Key(std::string key){
    if(!current_key_.has_value() && nodes_stack_.back()->IsDict()){
        current_key_ = std::move(key);
    }
    else{
        throw std::logic_error("Error setting Key");
    }
    return *this;
}

Builder& Builder::Value(Node::Value value){
    if(root_.IsNull()){
        root_.GetValue() = std::move(value);
    }
    else if(nodes_stack_.back()->IsDict()){
        if(!current_key_.has_value()){
            throw std::logic_error("No key for Dict");
        }
        std::get<json::Dict>(nodes_stack_.back()->GetValue()).emplace(current_key_.value(), value);
        current_key_.reset();
    }
    else if(nodes_stack_.back()->IsArray()){
        std::get<json::Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(value));
    }
    else{
        throw std::logic_error("No containers for Value");
    }
    return *this;
}

DictItemContext Builder::StartDict(){
    if(nodes_stack_.back()->IsNull()){
        nodes_stack_.back()->GetValue() = Dict{};
    }
    else if(nodes_stack_.back()->IsDict()){
        if(!current_key_.has_value()){
            throw std::logic_error("No key for Dict");
        }
        std::get<json::Dict>(nodes_stack_.back()->GetValue()).emplace(current_key_.value(), Dict{});
        const json::Node* node = &nodes_stack_.back()->AsDict().at(current_key_.value());
        nodes_stack_.push_back(const_cast<Node*>(node));
        current_key_.reset();
    }
    else if(nodes_stack_.back()->IsArray()){
        auto& temp_arr = std::get<json::Array>(nodes_stack_.back()->GetValue());
        temp_arr.emplace_back(Dict{});
        nodes_stack_.emplace_back(&temp_arr.back());
    }
    else{
        throw std::logic_error("Cant start Dict");
    }
    return *this;
}

ArrayItemContext Builder::StartArray(){
    if(nodes_stack_.back()->IsNull()){
        nodes_stack_.back()->GetValue() = Array{};
    }
    else if(nodes_stack_.back()->IsDict()){
        if(!current_key_.has_value()){
            throw std::logic_error("No key for Dict");
        }
        std::get<json::Dict>(nodes_stack_.back()->GetValue()).emplace(current_key_.value(), Array{});
        const json::Node* node = &nodes_stack_.back()->AsDict().at(current_key_.value());
        nodes_stack_.push_back(const_cast<Node*>(node));
        current_key_.reset();
    }
    else if(nodes_stack_.back()->IsArray()){
        auto& temp_arr = std::get<json::Array>(nodes_stack_.back()->GetValue());
        temp_arr.emplace_back(Array{});
        nodes_stack_.emplace_back(&temp_arr.back());
    }
    else{
        throw std::logic_error("Cant start Array");
    }
    return *this;
}

Builder& Builder::EndDict(){
    if(!nodes_stack_.back()->IsDict()){
        throw std::logic_error("Node not a Dict");
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray(){
    if(!nodes_stack_.back()->IsArray()){
        throw std::logic_error("Node not an Array");
    }
    nodes_stack_.pop_back();
    return *this;
}
// --------- Context ----------
DictItemContext::DictItemContext(Builder& builder) : builder_(builder) {}

ArrayItemContext::ArrayItemContext(Builder& builder) : builder_(builder) {}

KeyItemContext::KeyItemContext(Builder& builder) : builder_(builder) {}

KeyItemContext DictItemContext::Key(std::string key){
    return builder_.Key(key);
}

Builder& DictItemContext::EndDict(){
    return builder_.EndDict();
}

ArrayItemContext ArrayItemContext::Value(Node::Value value){
    return ArrayItemContext{builder_.Value(value)};
}

DictItemContext ArrayItemContext::StartDict(){
    return builder_.StartDict();
}

Builder& ArrayItemContext::EndArray(){
    return builder_.EndArray();
}

ArrayItemContext ArrayItemContext::StartArray(){
    return builder_.StartArray();
}

DictItemContext KeyItemContext::Value(Node::Value value){
    return DictItemContext{builder_.Value(value)};
}

DictItemContext KeyItemContext::StartDict(){
    return builder_.StartDict();
}

ArrayItemContext KeyItemContext::StartArray(){
    return builder_.StartArray();
}
}