#ifndef FACEFINDER_H
#define FACEFINDER_H

/* Finds Face regions from a given contiguous polyline.
 *
 * Written by Bryce Summers.
 * 6/26/2015.
 */

#include <vector>
#include "ofMain.h"
#include "Line.h"
#include "Intersector.h"
#include "CurveFunctions.h"

namespace scrib{


class FaceFinder
{
    public:

        FaceFinder(bool useFastAlgo = true)
        {
            bUseFastAlgo = useFastAlgo;
            closed_loop = false;
        };
        virtual ~FaceFinder(){};

        // Derive faces from a single polyline input.
        // No gurrantee is made about the order of the polygons.
        // The Output is a list of sub polygons.
        // All of the points given as inputs to this algorithm will be treated as if they were distinct.
        // The points will also be randomly offset by a 'small' amount to prevent the existence of vertical lines.
        std::vector< // List of Polygons.
        std::vector< // Each polygon is a list of points.
        point_info // Information about the point.
        > *> * FindFaces(std::vector<ofPoint> * inputs);

        // Derive faces from a set list of vertice disjoint polyline inputs.
        std::vector< // List of Polygons.
        std::vector< // Each polygon is a list of points.
        point_info // Information about the Point.
        > *> * FindFaces(std::vector< std::vector<ofPoint> *> * inputs);

        // Appends the indices of any external faces amongst the input list of faces to the output vector.
        // NOTE : The input type is equivelant to the output type of the face finding functions,
        // so using this function may be a natural extension of using the original functions.
        void determineExternalFaces(std::vector<std::vector<point_info> *> * input, std::vector<int> * output);

        // Tells this face finder to interpret the input curve as a line if open and a closed loop if closed.
        // If close, it will consider endpoints as attached to each other.
        void setClosed(bool isClosed);


    protected:
    private:

        inline std::vector<std::vector<point_info> *> * trivial(std::vector<ofPoint> * inputs);
        inline std::vector<std::vector<point_info> *> * do_the_rest();

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
        std::vector< std::vector<point_info> *> * deriveFaces();

        /* Outputs the cycle containing the directed edge p1 --> p2, where p2 = dg[p1][p2_index].
         * Traces cycles by always consistently following the rightmost edges.
         * (It could be leftmost and would still work as long as it is consistently left or consistently right, but not mixed...)
         * All edges are traced twice, once in each direction. For planar directed graphs, this is guaranteed to produce every cycle.
         * The "output_predicate" structure is used to keep track of which edges have been outputted.
         * Every edge direction is guaranteed to be in exactly one cycle. Each undirected edge can be though of as being in two cycles.
         */
        std::vector<point_info> * getCycle(int p1, int p2, int p2_index);

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
