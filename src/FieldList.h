#pragma once
#include <vector>
#include <string>
#include <variant>
#include <cstdint>


class FieldList
{
public:
    using ValueType = std::variant<std::string, int64_t, uint64_t, float, bool>;
public:
    void add(std::string name, ValueType value);
    std::string toJson() const;
private:
    std::vector<std::pair<std::string, ValueType>> data_;
};
