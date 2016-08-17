#ifndef CURVEFUNCTIONS_H
#define CURVEFUNCTIONS_H

// Standard functions and classes in the scrib namespace.
#include "ofMain.h"

namespace scrib
{
    /*
     * http://math.blogoverflow.com/2014/06/04/greens-theorem-and-area-of-polygons/
     * Computes the area of a 2D polygon directly from the polygon's coordinates.
     * The area will be positive or negative depending on the
     * clockwise / counter clockwise orientation of the points.
     * Also see: https://brycesummers.wordpress.com/2015/08/24/a-proof-of-simple-polygonal-area-via-greens-theorem/
     */
    float computeAreaOfPolygon(std::vector<point_info> * closed_polygon);

    /*
     * http://math.blogoverflow.com/2014/06/04/greens-theorem-and-area-of-polygons/
     * Computes the area of a 2D polygon directly from the polygon's coordinates.
     * The area will be positive or negative depending on the
     * clockwise / counter clockwise orientation of the points.
     * Also see: https://brycesummers.wordpress.com/2015/08/24/a-proof-of-simple-polygonal-area-via-greens-theorem/
     */
    float computeAreaOfPolygon(std::vector<ofPoint> * closed_polygon);
}

#endif // CURVEFUNCTIONS_H
