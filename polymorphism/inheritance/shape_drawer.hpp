#pragma once
#include "shapes_interface.hpp"
#include <tr1/memory>
#include <vector>

void draw_shapes(const std::vector<std::tr1::shared_ptr<shape> > &shapes);
