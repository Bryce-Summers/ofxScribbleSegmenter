#ifndef FACEFINDER_H
#define FACEFINDER_H

/*
* Transforms a set of input polylines into a planar graph embedding.
*
* Written by Bryce Summers.
* 6/26/2015: Wrote Original.
* 8/16/2016: Rewrote as PlanarGraphEmbedder in order to output full graph embeddings using a half edge structure.
*            This class remains as a very focused class.
* 8/18/2016: Post proccessing operations have been moved to PolylineGraphEmbedder
*
* Written for the STUDIO for Creative Inquiry at Carnegie Mellon University.
*/

#include <vector>
#include "ofMain.h"
#include "Line.h"
#include "Intersector.h"
#include "PolylineGraphPostProcessor.h" // point_info definition.

namespace scrib {


    class FaceFinder
    {
    public:

        // A User can explicitly pass false to force the intersection points to be found using a brute force algorithm that
        // may potentially be more robust and reliable than the optimized intersection algorithm,
        // but it kills the performance.
        FaceFinder(bool useFastAlgo = true)
        {
            bUseFastAlgo = useFastAlgo;
            closed_loop = false;
        };
        virtual ~FaceFinder() {};


        /* -- Here is the interface for calling the built in algorithms for the Scribble segmenter.
        * These algorithms include:
        * Preprocessing:
        * [EMPTY]
        *
        * Main Algorithm:
        * 1. The main algorithm for embedding a set of polylines in space and determining the set of non chordal cycles in the
        *    associated embedded planar graph.
        *
        * - A polygon is closed if it has identical starting and ending points and open otherwise.
        *   The algorithm may be configured to output either open or closed polygons based on the closed_loop mode state.
        *
        * FIXME: If a user draws a second line completely around an original line, then their will be faces defined by both an external
        *        face on the original polyline embedding and an internal face on the new enclosing embedding.
        *        This may invalidate some users' assumptions of a global planar graph embedding without any holes.
        *
        * Post Processing:
        * 1. Determine internal and external faces. (Initial Release)
        * 2. Determine trivial and non trivial area faces according to a constant area threshold value. (8/11/2016)
        *    (If you can't think of any good constant values, you might want to look at the field of
        *     Topological Data Analysis and their 'barcode' concept: https://en.wikipedia.org/wiki/Topological_data_analysis.
        * 3. Clipping off tails, i.e. portions of faces that enclose 0 area. (8/11/2016)
        *    This could potentially be put into the getCycle function, but I think that it is best to make this a dedicated post processing step instead
        *    in order to preserve the simplicity of the main algorithm.
        *    This algorithm properly handles faces with either duplicated or non-duplicated starting and ending points.
        *    i.e. Those produced in open and closed mode.
        */

        // Derive faces from a single polyline input.
        // No guarantee is made about the order of the polygons.
        // The Output is a list of sub polygons.
        // All of the points given as inputs to this algorithm will be treated as if they were distinct.
        // The points will also be randomly offset by a 'small' amount to prevent the existence of vertical lines.
        /*std::vector< // List of Polygons.
            std::vector< // Each polygon is a list of points.
            point_info // Information about the point.
            > *> *
         */  
        Face_Vector_Format * FindFaces(std::vector<ofPoint> * inputs);

        // Derive faces from a set list of vertex disjoint polyline inputs.
        /*std::vector< // List of Polygons.
            std::vector< // Each polygon is a list of points.
            point_info // Information about the Point.
            > *> *
         */
        Face_Vector_Format * FindFaces(std::vector< std::vector<ofPoint> *> * inputs);

        // Tells this face finder to interpret the input curve as a line if open and a closed loop if closed.
        // If close, it will consider endpoints as attached to each other.
        void setClosed(bool isClosed);


    protected:
    private:

        // The trivial function constructs the proper output for input polylines of size 1 or 0.
        inline Face_Vector_Format * trivial(std::vector<ofPoint> * inputs);
        inline Face_Vector_Format * do_the_rest();

        bool bUseFastAlgo;
        bool closed_loop;

        // Initializes the original lines from the input points.
        // Starts up the indexed collection of points.
        void loadInput(std::vector<ofPoint> * inputs);

        // Intersects the original lines and splits them.
        void splitIntersectionPoints();

        // Convert the set of lines into a directed graph.
        void convert_to_directedGraph();

        void addDirectedEdge(int p1, int p2);

        // Sort the graph by the cartesian angle of the edges.
        void sort_graph_by_edge_angle();

        // Sorts the outgoing_indices by the angles of the lines from the center
        // point to the points cooresponding to the outgoing indices.
        void sort_vertice_by_edge_angle(int center_point_index, std::vector<int> * outgoing_indices);

        // Uses the computed data structures to construct the set of all cycle lists.
        Face_Vector_Format * deriveFaces();

        /* Outputs the cycle containing the directed edge p1 --> p2, where p2 = dg[p1][p2_index].
        * Traces cycles by always consistently following the rightmost edges.
        * (It could be leftmost and would still work as long as it is consistently left or consistently right, but not mixed...)
        * All edges are traced twice, once in each direction. For planar directed graphs, this is guaranteed to produce every cycle.
        * The "output_predicate" structure is used to keep track of which edges have been outputted.
        * Every edge direction is guaranteed to be in exactly one cycle. Each undirected edge can be though of as being in two cycles.
        */
        Point_Vector_Format * getCycle(int p1, int p2, int p2_index);

        /* INPUT : the indices of the directed edge p1 --> p2.
        *
        * OUTPUT : the indice pointers will point to the new directed edge p1' --> p2'
        * NOTE : Indices are integers corresponding to the location in the global points array for each point.
        */
        void getNextEdge(int * p1_index, int * p2_index, int * outgoing_index);

        // Sets the predicate associated with the given directed edge.
        void setPredicate(int p1, int p2);

        // Returns the index i, such that dg[p1][i] = p2;
        // Returns -1 if index not found. (This should be an error for this project.)
        int find_outgoing_index_of_edge(int p1, int p2);

        // Free all of the data structures.
        void cleanup();

        // -- Data Structures.

        // The canonical collection of points at their proper indices.
        std::vector<ofPoint> points;

        // The original input lines.
        std::vector<scrib::Line> lines_initial;
        // Split version of original input lines, where lines only intersect at vertices.
        std::vector<scrib::Line> lines_split;

        // The directed graph that represents edges between points.
        // Each of the integers represents an index into the points array.
        // The output for a Y = lookup(x) is the set of all outgoing edges x --> y \in Y.
        std::map<int, std::vector<int> *> directed_graph;

        // A structure that keeps track of which edges have been added to an output cycle yet.
        // true  --> do not process this edge again, it is already in an output cycle.
        // false --> proccess this edge, it is part of a cycle that has not yet been output.
        std::map<int, std::vector<bool> *> output_predicate;

    };

}

#endif // FACEFINDER_H
