#pragma once

#include "ofMain.h"
#include "HalfedgeGraph.h"
#include "PolylineGraphData.h"

/* The PolylineGraphPostProcessor class.
*
* Written and adapted from FaceFinder on 8/18/2016.
*
* Purpose:
*
* Allows users to convert Planar Polyline Embedded Graphs into mainstream C++ data structures.
* This class then provides some useful processing algorithms on these output structures.
*
* I may also put information extraction algorithms here.
*
* The key idea is that the post processor doesn't mutate the Graph object in any way.
* FIXME: Use the Graph Mutator Proccessor instead (Currenly non existant.) if you wish to modify the graph.
*
* Maybe I will make a modification mode.
*/

namespace scrib
{

    class point_info
    {
    public:

        // -- Constructor.
        point_info(ofPoint p, int id)
        {
            point = p;
            ID = id;
        }

        ofPoint point;
        int ID;
    };

    // Convenience Typedefs.
    // Same old fashion mainstream c++ vectors, just an alis name.
    typedef std::vector<point_info> Point_Vector_Format;
    typedef std::vector<ofPoint> OF_Point_Vector_Format;
    typedef std::vector<Point_Vector_Format *> Face_Vector_Format;


    /*
    * http://math.blogoverflow.com/2014/06/04/greens-theorem-and-area-of-polygons/
    * Computes the area of a 2D polygon directly from the polygon's coordinates.
    * The area will be positive or negative depending on the
    * clockwise / counter clockwise orientation of the points.
    * Also see: https://brycesummers.wordpress.com/2015/08/24/a-proof-of-simple-polygonal-area-via-greens-theorem/
    */
    float computeAreaOfPolygon(Point_Vector_Format * closed_polygon);

    /*
    * http://math.blogoverflow.com/2014/06/04/greens-theorem-and-area-of-polygons/
    * Computes the area of a 2D polygon directly from the polygon's coordinates.
    * The area will be positive or negative depending on the
    * clockwise / counter clockwise orientation of the points.
    * Also see: https://brycesummers.wordpress.com/2015/08/24/a-proof-of-simple-polygonal-area-via-greens-theorem/
    */
    float computeAreaOfPolygon(OF_Point_Vector_Format * closed_polygon);


    class PolylineGraphPostProcessor
    {

    public:
        PolylineGraphPostProcessor(Graph * graph = NULL)
        {
            this->graph = graph;
        }

        virtual ~PolylineGraphPostProcessor() {}

    private:
        // Data.
        Graph * graph;

        // Face point vector format. // FIXME: Consider using smart pointers.
        Face_Vector_Format * face_vector = NULL;

    public:

        // -- Data Structure Conversion.
        Face_Vector_Format * convert_to_face_vectors();


        // -- This class performs operations on face vectors, but it only uses the current face vector as an input.
        // The class never changes the loaded face vector internally.
        // It is up to the user to load the proper face vector when they need a change.
        void load_face_vector(Face_Vector_Format * face_vector_format)
        {
            this->face_vector = face_vector_format;
        }

        void load_graph(Graph * graph)
        {
            this->graph = graph;
        }


        // -- Post processing algorithms.

        // Appends the indices of any external faces amongst the input list of faces to the output vector.
        // NOTE : The input type is equivelant to the output type of the face finding functions,
        // so using this function may be a natural extension of using the original functions.
        void determineExternalFaces(std::vector<int> * output);

        // Appends to output the indices of the faces of **NonTrivial** Area (area >= min_area)
        void determineNonTrivialAreaFaces(std::vector<int> * output, float min_area);
        // Appends to output the indices of the faces of **Trivial** Area (area < min_area)
        void determineTrivialAreaFaces(std::vector<int> * output, float min_area);

        // Input: a set of faces, Output: a new set of faces that have no trivial contiguous subfaces.
        // ENSURES: Polygons will be output either open or closed in the manner that they are passed in.
        // ENSURES: Omits faces consisting of only a single long tail.
        // The user is still responsible to deallocating the original vector.
        Face_Vector_Format * clipTails();

        // Returns a copy of the single input face without any trivial area contiguous subfaces. (Tails)
        // May return a 0 point polyline if the input line is non-intersecting.
        std::vector<point_info> * clipTails(Point_Vector_Format * input);

    };
}