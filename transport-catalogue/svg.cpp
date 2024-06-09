#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap){
    switch (stroke_line_cap){
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join){
    switch(stroke_line_join){
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
    }
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    if (!points_.empty()){
        for (auto it = points_.begin(); it != points_.end(); ++it){
            if (it != std::prev(points_.end())){
                out << it->x << ","sv << it->y << " "sv;
            }
            else{
                out << it->x << ","sv << it->y << "\""sv;
            }
        }
    }
    else{
        out << "\"";
    }
    RenderAttrs(context.out);
    out << " />"sv;
}
// ---------- Text ------------------

Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data){
    data_ = std::move(data);
    return *this;
}

std::string Text::ParseData(const std::string& data) const {
    std::string result;
    for (char ch : data){
        switch(ch) {
            case '"':
                result += "&quot;";
                break;
            case '\'':
                result += "&apos;";
                break;
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            case '&':
                result += "&amp;";
                break;
            default:
                result += ch;
                break;
        }
    }
    return result;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv
        << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv
        << size_ << "\""sv;
    font_family_.empty() ? out << "" : out << " font-family=\"" << font_family_ << "\"";
    font_weight_.empty() ? out << "" : out << " font-weight=\"" << font_weight_ << "\"";
    std::string temp_str = ParseData(data_);
    if (!temp_str.empty()){
        temp_str = temp_str.substr(temp_str.find_first_not_of(' '), temp_str.find_last_not_of(' ') - temp_str.find_first_not_of(' ') + 1);
    }
    RenderAttrs(context.out);
    out << ">"sv << temp_str << "</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext render_context{out, 2, 2};
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv
        << "\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    for (const auto& object : objects_){
        object->Render(render_context);
    }
    out << "</svg>"sv;
}
// ---------- Colors ------------------

void PrintColor(std::ostream& out, std::monostate){
    out << "none";
}

void PrintColor(std::ostream& out, std::string& color){
    out << color;
}

void PrintColor(std::ostream& out, Rgb rgb){
    out << "rgb("sv << static_cast<short>(rgb.red) 
    << "," << static_cast<short>(rgb.green) 
    << "," << static_cast<short>(rgb.blue) << ")";
}

void PrintColor(std::ostream& out, Rgba rgba){
    out << "rgba("sv << static_cast<short>(rgba.red) 
    << "," << static_cast<short>(rgba.green) 
    << "," << static_cast<short>(rgba.blue) 
    << "," << rgba.opacity << ")";
}

std::ostream& operator<<(std::ostream& out, const Color& color){
    std::visit([&out](auto value){
        PrintColor(out, value);
    }, color);
    return out;
}
}  // namespace svg