#include "shape_drawer.hpp"
#include <typeinfo>

using namespace std;

void draw_shapes( const vector< tr1::shared_ptr<shape> >& shapes)
{
#if __cplusplus >= 199711L
    for ( const auto& shape : shapes ) shape->draw();
#else
    for ( size_t i = 0; i < shapes.size(); i++ )
        shapes[ i ]->draw();
#endif
}
