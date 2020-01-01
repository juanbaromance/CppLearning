#include <vector>
#include <tr1/memory>
using namespace std;
#include "shapes.hpp"
#include "shape_drawer.hpp"

int main()
{
    std::vector<tr1::shared_ptr<shape> > shapes;
    shapes.push_back( tr1::shared_ptr<circle>( new circle ));
    shapes.push_back( tr1::shared_ptr<square>( new square ));
	draw_shapes(shapes);
}
