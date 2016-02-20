#include "CurveFunctions.h"

namespace scrib{

float computeAreaOfPolygon(std::vector<point_info> * closed_polygon)
{
    int len = closed_polygon -> size();
    ofPoint * p1 = &(closed_polygon -> at(len - 1).point);

    float area = 0.0;

    // Compute based on Green's Theorem.
    for(int i = 0; i < len; i++)
    {
        ofPoint * p2 = &(closed_polygon->at(i).point);
        area += (p2 -> x + p1 -> x)*(p2->y - p1->y);
        p1 = p2;// Shift p2 to p1.
    }

    return area/2.0;
}

float computeAreaOfPolygon(std::vector<ofPoint> * closed_polygon)
{
    int len = closed_polygon -> size();
    ofPoint * p1 = &(closed_polygon -> at(len - 1));

    float area = 0.0;

    // Compute based on Green's Theorem.
    for(int i = 0; i < len; i++)
    {
        ofPoint * p2 = &(closed_polygon->at(i));
        area += (p2 -> x + p1 -> x)*(p2->y - p1->y);
        p1 = p2;// Shift p2 to p1.
    }

    return area/2.0;
}
}
