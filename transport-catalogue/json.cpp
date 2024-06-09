#include "json.h"

namespace json {

namespace {

Node LoadNode(std::istream& input);

Node LoadNumber(std::istream& input){
    using namespace std::literals;

    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream");
        }
    };

    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected");
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }

    if (input.peek() == '0') {
        read_char();
    } else {
        read_digits();
    }

    bool is_int = true;

    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            try {
                return std::stoi(parsed_num);
            } catch (...) {
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert " + parsed_num + " to number");
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\" + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line");
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadNull(std::istream& input) {
    std::string value;
    char c;
    while (std::isalpha(input.peek())) {
        input >> c;
        value += c;
    }
    if (value == "null"){
        return Node(nullptr);
    }
    throw ParsingError("Null parsing error");
}

Node LoadBool(std::istream& input) {
    std::string value;
    char c;
    while (std::isalpha(input.peek())) {
        input >> c;
        value += c;
    }
    if (value == "true"){
        return Node{true};
    }
    else if (value == "false"){
        return Node{false};
    }
    throw ParsingError("Bool parsing error");
}

Node LoadArray(std::istream& input) {
    Array result;
    if (input.peek() == -1){
        throw ParsingError("Array parsing error");
    }
    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(move(result));
}

Node LoadDict(std::istream& input) {
    Dict result;
    if (input.peek() == -1){
        throw ParsingError("Dictionary parsing error");
    }
    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        std::string key = LoadString(input).AsString();
        input >> c;
        result.insert({std::move(key), LoadNode(input)});
    }

    return Node(move(result));
}

Node LoadNode(std::istream& input) {
    char c;
    if (!(input >> c)){
        throw ParsingError("Parsing error");
    }
    switch(c){
        case '[':
            return LoadArray(input);
            break;
        case '{':
            return LoadDict(input);
            break;
        case '"':
            return LoadString(input);
            break;
        case 'n':
            input.putback(c);
            return LoadNull(input);
            break;
        case 't': case 'f':
            input.putback(c);
            return LoadBool(input);
            break;
        default:
            input.putback(c);
            return LoadNumber(input);
            break;
    }
}

}  // namespace


bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
    return (std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_));
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

const Array& Node::AsArray() const {
    if (IsArray()){
        return std::get<Array>(value_);
    }
    throw std::logic_error("Wrong type");
}

const Dict& Node::AsMap() const {
    if (IsMap()){
        return std::get<Dict>(value_);
    }
    throw std::logic_error("Wrong type");
}

int Node::AsInt() const {
    if (IsInt()){
        return std::get<int>(value_);
    }
    throw std::logic_error("Wrong type");
}

const std::string& Node::AsString() const {
    if (IsString()){
        return std::get<std::string>(value_);
    }
    throw std::logic_error("Wrong type");
}

bool Node::AsBool() const{
    if (IsBool()){
        return std::get<bool>(value_);
    }
    throw std::logic_error("Wrong type");
}

double Node::AsDouble() const{
    if (!IsDouble()){
        throw std::logic_error("Wrong type");
    }
    if(IsInt()) return static_cast<double>(std::get<int>(value_));
    return std::get<double>(value_);
}

bool Node::operator==(const Node& rhs){
    return value_ == rhs.GetValue();
}

bool Node::operator !=(const Node& rhs){
    return !(value_ == rhs.GetValue());
}

bool operator==(const Node& lhs, const Node& rhs){
    return lhs.GetValue() == rhs.GetValue();
}
bool operator !=(const Node& lhs, const Node& rhs){
    return !(lhs == rhs);
}

bool Document::operator==(const Document& rhs){
    return root_ == rhs.GetRoot(); 
}
bool Document::operator !=(const Document& rhs){
    return !(root_ == rhs.GetRoot());
}

template <typename Value>
void PrintValue(const Value& value, PrintContext& context) {
    context.out << value;
}

void PrintValue(std::nullptr_t, PrintContext& context) {
    context.out << "null";
}

void PrintValue(bool value, PrintContext& context) {
    context.out << std::boolalpha << value;
}

void PrintValue(Array arr, PrintContext& context) {
    context.out << "[\n";
    auto indent = context.Indented();
    for (auto it = arr.begin(); it != arr.end(); ++it){
        indent.PrintIndent();
        PrintNode(*it, indent);
        if (it != std::prev(arr.end())){
            context.out << ",\n";
        }
    }
    context.out << "\n";
    context.PrintIndent();
    context.out << ']';
}

void PrintValue(std::string str, PrintContext& context){
    context.out << "\"";
    for (const char& ch : str){
        switch(ch){
            case '\n':
                context.out << "\\n";
                break;
            case '\r':
                context.out << "\\r";
                break;
            case '\"':
                context.out << R"(\")";
                break;
            case '\t':
                context.out << "\\t";
                break;
            case '\\':
                context.out << R"(\\)";
                break;
            default:
                context.out << ch;
                break;
        }
    }
    context.out << "\"";
}

void PrintValue(Dict dict, PrintContext& context){
    context.out << "{\n";
    bool is_first = true;
    for (const auto& [key, value] : dict){
        if (is_first){
            is_first = false;
        }
        else{
            context.out << ",\n";
        }
        context.Indented().PrintIndent();
        PrintValue(key, context);
        context.out << ": ";
        PrintNode(value, context);
    }
    context.out << "\n";
    context.PrintIndent();
    context.out << "}";
}

void PrintNode(const Node& node, PrintContext& context) {
    std::visit(
        [&context](const auto& value){ PrintValue(value, context); },
        node.GetValue());
} 

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintContext context{output};
    PrintNode(doc.GetRoot(), context);
}

}  // namespace json