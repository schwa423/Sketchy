//
//  Arc.cpp
//  Sketchy
//
//  Created by Josh Gargus on 8/5/12.
//
//

#include "Arc.h"

namespace Sketchy {
namespace Geom {

std::ostream& operator <<(std::ostream& os, const Sketchy::Geom::Arc& arc) {
    return os
        << "Arc{start:" << arc.start_radians / M_PI << "pi"
        << ", end:" << arc.end_radians / M_PI << "pi"
        << ", radius:" << arc.radius
        << ", length:" << arc.length()
        << ", center:" << arc.center.x() << "," << arc.center.y()
        << endl;
}

}   // namespace Geom
}  // namespace Sketchy
