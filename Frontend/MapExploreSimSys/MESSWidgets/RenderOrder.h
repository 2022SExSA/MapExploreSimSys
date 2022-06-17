#ifndef RENDERORDER_H
#define RENDERORDER_H

#include <vector>
#include <string>

class QJsonValue;

struct RenderOrder {
    enum class Code : int {
        NOP, CLEAR, DRAW,
    };

    static constexpr const char *CODE2STRING[] = {
            "N", "C", "D", // nop, clear, draw
    };

    Code code{Code::NOP};
    std::vector<int> args;

    std::string stringify() const {
        std::string res(CODE2STRING[(int)code]);
        for (auto arg : args) {
            res.append(" ").append(std::to_string(arg));
        }
        return res;
    }

    void fromJson(const QJsonValue &json);
};

#endif // RENDERORDER_H
