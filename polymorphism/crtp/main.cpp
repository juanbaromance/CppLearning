#include <vector>
#include <iostream>

template <typename T>
struct crtp {
    T& underly(){ return static_cast<T&>(*this); }
    T const& underly() const { return static_cast<T const&>(*this); }
};

template<typename Child>
struct shape : crtp<Child> {
    void draw()const
    {
        std::cout << "Preparing screen :: " << this->underly().draw_impl() << std::endl;
    }

    void reduce() const
    {
        std::cout << "Preparing screen :: " << this->underly().reduce_impl() << std::endl;
    }
#if __cplusplus >= 199711L

private:

    shape(){}
    friend Child;

#endif

};


struct reductor : shape<reductor> {
    std::string reduce_impl() const { return __PRETTY_FUNCTION__ ; }
    std::string draw_impl() const { return __PRETTY_FUNCTION__ ; }
};

struct square : shape<square> {
    std::string draw_impl() const { return __PRETTY_FUNCTION__ ; }
};

struct circle : shape<circle> {
    std::string draw_impl()const { return __PRETTY_FUNCTION__ ; }
};

struct triangle : shape<triangle>
{
    std::string draw_impl() const { return __PRETTY_FUNCTION__ ; }
};


/* client part */
template<typename Shape>
void draw(const Shape& shape)
{
    shape.draw();
}

int main()
{
    triangle t;
    circle c;

    draw( t );
    draw( c );

    reductor r;
    r.draw();
    r.reduce();
    return 0;
}
