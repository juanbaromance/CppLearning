#include <iostream>
#include <variant>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int main() 
{
    auto PrintVisitor = [](const auto& t) { std::cout << t << '\n'; };
    
    float p(4.32);
    std::variant<int, float, std::string> intFloatString { p };
    std::visit(overloaded{
        [](int& i) { i*= 2; },
        [](float& f) { f*= 2.0f; },
        [](std::string& s) { s = s + s; }
    }, intFloatString);
    
    std::visit(PrintVisitor, intFloatString);
    
    // "custom" print:
    std::visit(overloaded{
        [](const int& i) { std::cout << "int: " << i << '\n'; },
        [](const float& f) { std::cout << "float: " << f << '\n'; },
        [](const std::string& s) { std::cout << "string: " << s << '\n';}
    }, intFloatString);
    
    return 0;
}