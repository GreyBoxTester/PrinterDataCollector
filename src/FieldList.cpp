#include "FieldList.h"
#include <sstream>


class ValuePrinter
{
public:
    ValuePrinter(std::ostream& ostr) : ostr_(ostr) {}
    void operator()(const std::string& str) { ostr_ << '\"' << str << '\"'; }
    void operator()(int64_t num) { ostr_ << num; }
    void operator()(uint64_t num) { ostr_ << num; }
    void operator()(float num) { ostr_ << num; }
    void operator()(bool num) { ostr_ << (num ? "true" : "false"); }
private:
    std::ostream& ostr_;
};


void FieldList::add(std::string name, ValueType value) 
{
    data_.emplace_back(std::move(name), std::move(value));
}

std::string FieldList::toJson() const
{
    std::ostringstream json;
    json << R"({"version":1,"data":{)";  
    for (auto& [name, value] : data_)
    {
        json << '\"' << name << "\":";
        std::visit(ValuePrinter(json), value);
        json << ',';
    }
    json << "}}";
    return json.str();
}