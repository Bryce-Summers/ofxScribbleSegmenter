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

    // Forward declare types.
    class point_info;
    class face_info;


    // Convenience Typedefs.
    // Same old fashion mainstream c++ vectors, just an alis name.
    typedef std::vector<point_info> Point_Vector_Format;
    typedef std::vector<ofPoint> OF_Point_Vector_Format;
    typedef std::vector<Point_Vector_Format *> Face_Vector_Format;
    typedef std::vector<int> Int_Vector_Format;
    typedef std::set<int> ID_Set;

    class point_info
    {
    public:

        // -- Constructor.
        
        // Used in Facefinder for non halfedgemesh based embeddings.
        point_info(ofPoint p, int id)
        {
            point = p;
            ID = id;
        }

        point_info(ofPoint p, int id, Halfedge * hedge)
        {
            point = p;
            ID = id;
            this -> halfedge = hedge;
        }

        ofPoint point;
        int ID;

        // The halfedge that this point represents when this point is collected in a Point_Vector to represent a face.
        // This may be used to easily extract local connectivity information and attributes for this point and its neighbors.
        // WARNING: This always points to the original embedding's connectivity information,
        // which means that things like next pointers may no longer be valid after tails are clipped or other algorithms.
        // Faces and twin pointers should still be valid though...
        // Undefined for output from scrib::FaceFinder.
        Halfedge * halfedge = NULL;
    };

    class face_info
    {
    public:
        //int color;
        std::vector<face_info *> holes;
        Point_Vector_Format points;

        // Contains a list of all faces contributing to this unioned face.
        ID_Set faces_ID_set;

        bool complemented;
    };

    /*
    * http://math.blogoverflow.com/2014/06/04/greens-theorem-and-area-of-polygons/
    * Computes the area of a 2D polygon directly from the polygon's coordinates.
    * The area will be positive or negative depending on the
    * clockwise / counter clockwise orientation of the points.
    * Also see: https://brycesummers.wordpress.com/2015/08/24/a-proof-of-simple-polygonal-area-via-greens-theorem/
    */
    float computeAreaOfPolygon(Point_Vector_Format * closed_polygon);
    bool  isComplemented      (Point_Vector_Format * closed_polygon);

    /*
    * http://math.blogoverflow.com/2014/06/04/greens-theorem-and-area-of-polygons/
    * Computes the area of a 2D polygon directly from the polygon's coordinates.
    * The area will be positive or negative depending on the
    * clockwise / counter clockwise orientation of the points.
    * Also see: https://brycesummers.wordpress.com/2015/08/24/a-proof-of-simple-polygonal-area-via-greens-theorem/
    */
    float computeAreaOfPolygon(OF_Point_Vector_Format * closed_polygon);
    bool  isComplemented      (OF_Point_Vector_Format * closed_polygon);

    class PolylineGraphPostProcessor
    {

    public:
        PolylineGraphPostProcessor() {}
        virtual ~PolylineGraphPostProcessor() {}

    private:
        // Data.
        Graph * graph = NULL;

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
            this -> face_vector = face_vector_format;
        }

        void free_face_vector()
        {
            if (this -> face_vector != NULL)
            {
                delete this -> face_vector;
                this -> face_vector = NULL;
            }
        }

        void load_graph(Graph * graph)
        {
            this -> graph = graph;
        }

        void freeGraph()
        {
            if(this -> graph != NULL)
            {
                delete this -> graph;
                this -> graph = NULL;
            }
        }


        // -- Post processing algorithms.

        // Appends the indices of any external faces amongst the input list of faces to the output vector.
        // NOTE : The input type is equivelant to the output type of the face finding functions,
        // so using this function may be a natural extension of using the original functions.
        void determineComplementedFaces(std::vector<int> * output);

        // Appends to output the indices of the faces of **NonTrivial** Area (area >= min_area)
        void determineNonTrivialAreaFaces(Int_Vector_Format * output, float min_area);
        // Appends to output the indices of the faces of **Trivial** Area (area < min_area)
        void determineTrivialAreaFaces(Int_Vector_Format * output, float min_area);

        // Input: a set of faces, Output: a new set of faces that have no trivial contiguous subfaces.
        // ENSURES: Polygons will be output either open or closed in the manner that they are passed in.
        // ENSURES: Omits faces consisting of only a single long tail.
        // The user is still responsible to deallocating the original vector.
        Face_Vector_Format * clipTails();

        // Returns a copy of the single input face without any trivial area contiguous subfaces. (Tails)
        // May return a 0 point polyline if the input line is non-intersecting.
        Point_Vector_Format * clipTails(Point_Vector_Format * input);

        // Uses the currently loaded this->graph object as Input.
        // Takes in a vector containing the integer IDs of the faces to be merged.
        // Outputs the result of unioning all of the faces.
        std::vector<face_info *> * mergeFaces(ID_Set * face_ID_set);

    private:
        
        // Returns true iff the given hafedge is included in the output of the union of the given faces.
        // I.E. returns true iff the given half edge -> face is within the set of unioned faces and half_edge->twin -> face is not.
        // Tail edges, where the halfedge and its twin are on the same face are not considered to be in a halfedgeUnion face.
        bool _halfedgeInUnion(ID_Set * face_ID_set, Halfedge * start);

        // Given an In Union halfege, traces its face_info union face information.
        // Properly sets the output's: points and face_IDs fields.
        // Marks halfedges, therefore calling functions are responsible for unmarking halfedges.
        face_info * _traceUnionFace(ID_Set * face_ID_set, Halfedge * start);

        // Given a halfedge inside of a unionface, returns the next halfedge within that face.
        Halfedge * nextUnionFace(ID_Set * face_ID_Set, Halfedge * current);

        point_info halfedgeToPointInfo(Halfedge * halfedge);

    };
}