#ifndef LINE_H
#define LINE_H

#include <vector>
#include "ofMain.h"

/**
* Line Representation and methods class.
* First Draft completed by Bryce Summers on 7/14/2015.
*/

namespace scrib
{

    class Line
    {
    public:

        // Constructor.
        Line(int start_point_index, int end_point_index, std::vector<ofPoint> * points_global);
        virtual ~Line();

        /** Intersects the given line with this line.
        *  Adds a split point if they do intersect.
        *  Any created split points are added to the referenced global collection of points.
        */
        bool intersect(Line * other);

        // Returns a signed floating point number indicating which direction the given point is relative to this line.
        float line_side_test(ofPoint p);

        // Appends all of the split set of lines in order to the output vector.
        // Adds itself if it does not contain any split lines.
        // Line pts are oriented along the polyline, such that p1 comes before p2 in the polyline + intersection point ordering.
        void getSplitLines(std::vector<Line> * lines_collector);

        // This function should only be called after a call to intersect has returned true.
        ofPoint getLatestIntersectionPoint();

        // Point indices.
        int p1_index;
        int p2_index;

        // Actual points.
        ofPoint p1;
        ofPoint p2;

        // The offset between the two points.
        ofPoint offset;


    protected:
    private:

        // The canonical array of points.
        std::vector<ofPoint> * points;
        // Collection of doubles representing the percentage a point is between p1 and p2.
        std::vector<float> split_points_per;
        // The indices of the points.
        std::vector<int> split_points_indices;

        void sort_sub_points();

        // Returns true iff this line segment intersects with the other line segment.
        inline bool detect_intersection(Line * other);
        inline void report_intersection(Line * other);

    };

}

#endif // LINE_H
