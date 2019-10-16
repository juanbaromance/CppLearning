#include <iostream>
#include <vector>

template<typename Child>
struct shape {
    void draw()const {
	std::cout << "Preparing screen\n";
	static_cast<const Child*>(this)->draw_implementation();
    }

private:
    shape(){}
    friend Child;
};

struct square : shape<square> {
    void draw_implementation() const { std::cout << "square::draw\n"; }
};

struct circle : shape<circle> {
    void draw_implementation()const { std::cout << "circle::draw\n"; }
};

struct triangle : shape<triangle>
{
    void draw_implementation() const { std::cout << __PRETTY_FUNCTION__ << "::draw\n"; }
};


/* client part */
template<typename Shape>
void draw(const Shape& shape)
{
    shape.draw();
}


int main() {
    circle c;
    square s;
    triangle t;

    draw(c);
    draw(s);
    draw(t);
}
