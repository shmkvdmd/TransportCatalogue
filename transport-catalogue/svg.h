#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <fstream>

namespace svg {

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap);

std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join);

struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

struct Rgb{
    Rgb() = default;

    Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {};

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba : public Rgb{
    Rgba() = default;

    Rgba(uint8_t r, uint8_t g, uint8_t b, double op) : Rgb(r,g,b), opacity(op) {};

    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{"none"};

void PrintColor(std::ostream& out, std::monostate);

void PrintColor(std::ostream& out, std::string& color);

void PrintColor(std::ostream& out, Rgb rgb);

void PrintColor(std::ostream& out, Rgba rgba);

std::ostream& operator<<(std::ostream& out, const Color& color);

template <typename Owner>
class PathProps{
public:
    Owner& SetFillColor(Color color){
        fill_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeColor(Color color){
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width){
        width_ = width;
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap){
        line_cap_ = line_cap;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
        line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        fill_color_ != std::nullopt ? out << " fill=\""sv << fill_color_.value() << "\""sv : out << "";
        stroke_color_ != std::nullopt ? out << " stroke=\""sv << stroke_color_.value() << "\""sv : out << "";
        width_ != std::nullopt ? out << " stroke-width=\""sv << width_.value() << "\""sv : out << "";
        line_cap_ != std::nullopt ? out << " stroke-linecap=\""sv << line_cap_.value() << "\""sv : out << "";
        line_join_ != std::nullopt ? out << " stroke-linejoin=\""sv << line_join_.value() << "\""sv : out << "";
    }
private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    
    std::vector<Point> points_;
};

class Text final : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos);

    Text& SetOffset(Point offset);

    Text& SetFontSize(uint32_t size);

    Text& SetFontFamily(std::string font_family);

    Text& SetFontWeight(std::string font_weight);

    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;
    std::string ParseData(const std::string& data) const;
    Point pos_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

class ObjectContainer{
public:
    virtual ~ObjectContainer() = default;

    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
};

class Drawable {
public:
    virtual ~Drawable() = default;

    virtual void Draw(ObjectContainer& container) const = 0;
};

class Document : public ObjectContainer {
public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg