#ifndef MESSBASE_UTILS_H
#define MESSBASE_UTILS_H

#include "fwd.h"
#include <sstream>
MESSBASE_NAMESPACE_START

template <typename T>
struct Vec2 {
    T x{0};
    T y{0};

#define DEINFE_OP_ASS(op) \
    template <typename D> \
    Vec2 &operator op (const Vec2<D>& a) { \
        x op a.x; \
        y op a.y; \
        return *this; \
    }
    DEINFE_OP_ASS(+=)
    DEINFE_OP_ASS(-=)
#undef DEINFE_OP_ASS

#define DEINFE_OP_ASS(op) \
    template <typename D> \
    Vec2 &operator op (D a) { \
        x op a; \
        y op a; \
        return *this; \
    }
    DEINFE_OP_ASS(*=)
    DEINFE_OP_ASS(/=)
#undef DEINFE_OP_ASS
};

template <typename T>
using Point = Vec2<T>;

template <typename T>
struct Rect {
    Point<T> top_left{};
    T width{0};
    T height{0};

    Point<T> center() const {
        return Point<T>{top_left} += Point<T>{width / 2, height / 2};
    }

    template <typename V>
    bool inner(const Rect<V> &other) const {
        if (top_left.x < other.top_left.x) return false;
        if (top_left.y < other.top_left.y) return false;
        if (top_left.x + width > other.top_left.x + other.width) return false;
        if (top_left.y + height > other.top_left.y + other.height) return false;
        return true;
    }

    template <typename V>
    bool surrounded(const Rect<V> &other) const {
        return other.inner(*this);
    }

    template <typename V>
    bool intersect(const Rect<V> &other) const {
        if (top_left.y > other.top_left.y + other.height) return false;
        if (top_left.x > other.top_left.x + other.width) return false;
        if (other.top_left.y > top_left.y + height) return false;
        if (other.top_left.x > top_left.x + width) return false;
        return true;
    }
};

template <typename T>
struct AABB { // Oriented Bounding Box
    Rect<T> area_{};
    T theta_{0}; // with posi-x, only 0, 90, 180, 270

    Rect<T> to_rect() const {
        int i = (int)theta_;
        Rect<T> res = area_;
        if (i == 0 || i == 180) {
            res.top_left.x += (area_.width - area_.height) / 2;
            res.top_left.y -= (area_.width - area_.height) / 2;
            std::swap(res.width, res.height);
        } else if (i == 90 || i == 270) {
        } else {
            MESS_LOG("ERR: i={0}", i);
            PGZXB_DEBUG_ASSERT(false);
        }

        return res;
    }

    template <typename V>
    bool intersect(const AABB<V> &other) const {
        return to_rect().intersect(other.to_rect());
    }
};

template <typename T>
inline Vec2<T> axis_theta_to_unit_vec2(double theta) {
    int i = (int)theta;
    if (i == 0) return {1, 0};
    if (i == 90) return {0, 1};
    if (i == 180) return {-1, 0};
    if (i == 270) return {0, -1};
    PGZXB_DEBUG_ASSERT(false);
    return {};
}

template <typename F, typename R, typename ...Args>
class FuncRef {
public:
    using Func = R(Args...);

    FuncRef(F *f) : f_(f) {

    }

    FuncRef(const FuncRef &o) = default;
    FuncRef &operator= (const FuncRef &o) = default;

    R operator() (Args ... args) {
        return (*f_)(std::forward<Args>(args)...);
    }
private:
    F *f_{nullptr};
};

struct Op {
    std::string auth_token;
    std::string op;
    std::vector<std::string> args;

    static std::optional<Op> from_string(const std::string &str) { // tk op arg0 arg1 ...
        std::istringstream iss(str);
        Op op;
        if (!(iss >> op.auth_token)) return std::nullopt;
        if (!(iss >> op.op)) return std::nullopt;
        
        std::string temp;
        while (iss >> temp) {
            op.args.push_back(std::move(temp));
        }

        return op;
    }
};

MESSBASE_NAMESPACE_END
#endif
