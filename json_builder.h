#include "json.h"

namespace json{

struct NodeVisitor{
    Node operator()(std::nullptr_t value);
    Node operator()(const Array& value);
    Node operator()(const Dict& value);
    Node operator()(bool value);
    Node operator()(int value);
    Node operator()(double value);
    Node operator()(const std::string& value);
};

class Builder {
public:
    class BaseContext;
    class KeyBaseContext;
    class DictBaseContext;
    class ArrayBaseContext;
    
    Builder() = default;
    KeyBaseContext Key(std::string key);
    Builder& Value(Node::Value value, bool is_beginning = false);
    DictBaseContext StartDict();
    ArrayBaseContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();
private:
    Node node_;
    std::vector<Node*> nodes_stack_;
    bool is_empty_ = true;
};

class Builder::BaseContext {
public:
    BaseContext(Builder& builder)
    :builder_(builder){}

protected:
    KeyBaseContext Key(std::string);
    DictBaseContext StartDict();
    ArrayBaseContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Builder& builder_;
};

class Builder::KeyBaseContext : BaseContext {
public:
    using BaseContext::BaseContext;
    using BaseContext::StartDict;
    using BaseContext::StartArray;

    DictBaseContext Value(Node::Value value);
};

class Builder::DictBaseContext : BaseContext {
public:
    using BaseContext::BaseContext;
    using BaseContext::Key;
    using BaseContext::EndDict;
};

class Builder::ArrayBaseContext : BaseContext {
public:
    using BaseContext::BaseContext;
    using BaseContext::StartDict;
    using BaseContext::StartArray;
    using BaseContext::EndArray;

    ArrayBaseContext Value(Node::Value value);

};


} // namespace json