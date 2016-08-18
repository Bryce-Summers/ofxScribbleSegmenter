#include "Line.h"

namespace scrib {

    Line::Line(int start_point, int end_point, std::vector<ofPoint> * points_global)
    {
        p1_index = start_point;
        p2_index = end_point;

        // The indices point to points within this global array.
        points = points_global;

        p1 = points->at(p1_index);
        p2 = points->at(p2_index);

        offset = p2 - p1;
    }

    Line::~Line()
    {
        //dtor
    }

    // Returns true iff the lines intersect, if they are not already connected at endpoints a split point is created.
    // Intersections at end points --> a false return;
    bool Line::intersect(Line * other)
    {

        // Already Previously Connected.
        // Connected at a joint in the input polyline.
        if (p1_index == other->p1_index || p1_index == other->p2_index ||
            p2_index == other->p1_index || p2_index == other->p2_index)
        {
            return false;
        }

        // No intersection.
        if (!detect_intersection(other))
        {
            return false;
        }

        report_intersection(other);
        return true;

    }

    // Returns -1 on one side of the line.
    // Returns 0 if the point is on the line.
    // Returns 1 if the point is on the other side of the line.
    float Line::line_side_test(ofPoint c)
    {
        return ((p2.x - p1.x)*(c.y - p1.y) - (p2.y - p1.y)*(c.x - p1.x));
    }

    inline bool Line::detect_intersection(Line * other)
    {
        float a1 = line_side_test(other->p1);
        float a2 = line_side_test(other->p2);

        float b1 = other->line_side_test(p1);
        float b2 = other->line_side_test(p2);

        /* The product of two point based line side tests will be negative iff
        * the points are not on strictly opposite sides of the line.
        * If the product is 0, then at least one of the points is on the line not containing the points.
        */
        return a1*a2 <= 0 && b1*b2 <= 0;
    }

    inline void Line::report_intersection(Line * other)
    {

        // Find the intersection point.

        /*
        u = ((bs.y - as.y) * bd.x - (bs.x - as.x) * bd.y) / (bd.x * ad.y - bd.y * ad.x)
        v = ((bs.y - as.y) * ad.x - (bs.x - as.x) * ad.y) / (bd.x * ad.y - bd.y * ad.x)
        Factoring out the common terms, this comes to:

        dx = bs.x - as.x
        dy = bs.y - as.y
        det = bd.x * ad.y - bd.y * ad.x
        u = (dy * bd.x - dx * bd.y) / det
        v = (dy * ad.x - dx * ad.y) / det
        */

        // Extract the relevant points.
        ofPoint as = p1;
        ofPoint bs = other->p1;
        ofPoint ad = offset;
        ofPoint bd = other->offset;

        float dx = bs.x - as.x;
        float dy = bs.y - as.y;
        float det = bd.x * ad.y - bd.y * ad.x;
        float u = (dy * bd.x - dx * bd.y) / det;
        float v = (dy * ad.x - dx * ad.y) / det;

        // The intersection is at time coordinates u and v.
        // Note: Time is relative to the offsets, so p1 = time 0 and p2 is time 1.

        // u is the time coordinate for this line.
        split_points_per.push_back(u);

        // v is the time coordinate for the other line.
        other->split_points_per.push_back(v);

        ofPoint intersection_point = as + ad*u;

        // Get the next index that will be used to store the newly created point.
        int index = points->size();
        points->push_back(intersection_point);

        split_points_indices.push_back(index);
        other->split_points_indices.push_back(index);
    }

    /*
    bool isLeft(Point a, Point b, Point c){
    return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
    }
    */


    // Appends all of the lines that are used to subdivide this one,
    // complete with proper and consistent indices into the global array.
    void Line::getSplitLines(std::vector<Line> * lines_collector)
    {
        // Number of split points.
        int len = split_points_per.size();

        // Not split points.
        if (len == 0)
        {
            // Saves work.
            lines_collector->push_back(*this);
            return;
        }

        // First sort points.
        sort_sub_points();

        // Make sure the last line is pushed.
        // This ensures that that initial line will be pushed if this line has no intersections.
        split_points_indices.push_back(p2_index);

        // Append all of the line's segments to the inputted array.
        int last_indice = split_points_indices[0];

        // The initial line.
        lines_collector->push_back(scrib::Line(p1_index, last_indice, points));

        for (int i = 1; i < len; i++)
        {
            int next_indice = split_points_indices[i];
            lines_collector->push_back(scrib::Line(last_indice, next_indice, points));
            last_indice = next_indice;
        }

        // The last line.
        lines_collector->push_back(scrib::Line(last_indice, p2_index, points));

        // Done.
        return;

    }

    // Sorts all of the subpoints by percentage.
    void Line::sort_sub_points()
    {

        int len = split_points_per.size();

        // Insertion sort.
        for (int i = 1; i < len; i++)
            for (int i2 = i - 1; i2 >= 0; i2--)
            {
                int i1 = i2 + 1;

                if (split_points_per[i2] <= split_points_per[i1])
                {
                    break;
                }


                // -- Swap at indices i2 and i2 + 1.
                // Keep the percentage measuremtents consistent with the indices.
                float temp_f = split_points_per[i2];
                split_points_per[i2] = split_points_per[i1];
                split_points_per[i1] = temp_f;

                int temp_i = split_points_indices[i2];
                split_points_indices[i2] = split_points_indices[i1];
                split_points_indices[i1] = temp_i;
            }

        return;

    }

    ofPoint Line::getLatestIntersectionPoint()
    {
        return points->at(points->size() - 1);
    }

}
