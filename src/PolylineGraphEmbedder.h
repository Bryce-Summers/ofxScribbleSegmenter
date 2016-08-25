#pragma once

/*
* Transforms a set of input polylines into a planar graph embedding.
*
* Written by Bryce Summers.
*
* 8/16/2016: Written as a more fully advanced version of FaceFinder.h,
*            which outputs sophisticated graph structures oozing with useful connectivity information.
*
* Written for the STUDIO for Creative Inquiry at Carnegie Mellon University.
*/

// FIXME: I am refactoring post proccess operations into their own class.

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



// FIXME: Write a list of all of the relevant interesting properties of the my planar embedding implementation.

// Edges alwas point to the forward faceing halfedge.
// forward facing half edges are consecutively ordered in the first half.
// backwards facing half edges are consecutively ordered in the reverse order of the first half.




#include <vector>
#include "ofMain.h"
#include "Line.h"
#include "Intersector.h"
#include "HalfedgeGraph.h"
#include "PolylineGraphData.h"

namespace scrib {


    class PolylineGraphEmbedder
    {
    public:

        // A User can explicitly pass false to force the intersection points to be found using a brute force algorithm that
        // may potentially be more robust and reliable than the optimized intersection algorithm,
        // but it kills the performance.
        PolylineGraphEmbedder(bool useFastAlgo = true)
        {
            bUseFastAlgo = useFastAlgo;
            closed_loop = false;
        };
        virtual ~PolylineGraphEmbedder() {};

        // Tells this face finder to interpret the input curve as a line if open and a closed loop if closed.
        // If close, it will consider endpoints as attached to each other.
        void setClosed(bool isClosed);

        // Derives a planar graph embedding from the given input polyline.
        // The input will be interpretted as open or closed depending on the value of this.closed_loop;
        // Assumes all points are distinct.
        // Offsets all input points by a small random amount to prevent degeneracies from vertical edges.
        Graph * embedPolyline(std::vector<ofPoint> * inputs);

        // Derive faces from a set list of vertex disjoint polyline inputs.
        Graph * embedPolylineSet(std::vector< std::vector<ofPoint> *> * inputs);

    protected:
    private:

        // The trivial function constructs the proper output for input polylines of size 1 or 0.
        // ASSUMES input is of size 0 or 1.
        inline Graph * trivial(std::vector<ofPoint> * inputs);
        inline Graph * do_the_rest();

        bool bUseFastAlgo;
        bool closed_loop;

        // -- Step 1. Compute canonical input structures.

        // The embedding is broken down into seperate phases. Here I have listed each operation,
        // followed by the data structures that they have built.

        // Appends the given input points to the collated single input point array.
        // Performs point fudging to avoid degenerate behavior.
        // Starts up the indexed collection of points.
        void loadInput(std::vector<ofPoint> * inputs);

        // The canonical collection of points at their proper indices.
        std::vector<ofPoint> points;
        // The original input lines.
        std::vector<scrib::Line> lines_initial;

        // -- Step 2. Find intersections in the input and compute the embedded polyline structure.

        // Intersects the input lines, then splits them and connects them appropiatly.
        // Populates the list of edge disjoint lines that only intersect at vertices.
        // puts the edge in consecutive order following the input polylines.
        // results put into this.lines_split
        void splitIntersectionPoints();

        // Split version of original input lines, where lines only intersect at vertices.
        std::vector<scrib::Line> lines_split;

        // Allocates the output graph object and allocates vertices, edges, and halfedges for the input data.
        // Vertices are Indexed as follows [original points 1 for input polyline 1, then 2, ...,
        // new intersection points for polyline 1, then 2, etc, ...]
        // Halfedges are indexed in polyline input order, then in backwards input order.
        // -- Step 3. Proccess the embedded input and initialize the Planar Graph vertices, edges, and halfedges.
        void allocate_graph_from_input();

        // The graph that is being built.
        // Once it is returned, the responsibility for this memory transfers to the user and the pointer is forgotten from this class.
        // FIXME: Shared_ptr or some other supposedly better pointer type?
        Graph * graph;

        // -- Step 4. Sort all outgoing edge lists for intersection vertices by the cartesian angle of the edges.
        void sort_outgoing_edges_by_angle();

        // Step 4 helper function.
        // Sorts the outgoing_indies by the angles of the lines from the center
        // point to the points cooresponding to the outgoing indices.
        void sort_outgoing_edges(std::vector<Halfedge * > & outgoing_indices);

        // -- Step 5.
        // Determines the next and previous pointers for the halfedges in the Graph.
        // This is done almost entirely using the sets of outgoing edges for each vertex.
        // vertices of degree 2 associate their 2 pairs of neighbors.
        // vertices of degree are on a tail and associate their one pair of neighbors.
        // vertices of degree >2 are intersection points and they first sort their neighbors, then associate their star.
        // This function sets the Vertex_Data objects classification data.
        void associate_halfedge_cycles();


        // Step 6.
        // Uses the vertex and edge complete halfedge mesh to add face data.
        // Also produces simpler cycle structures along that serve as an alternate representation of the faces.
        Graph * deriveFaces();

        /*
        * REQUIRES: 1. face -> halfedge well defined already.
        *			 2. halfedge next pointer well defined already.
        * ENSURES:  links every halfedge in the loop starting and ending at face -> halfedge
        *           with the face.
        */
        void trace_face(Face * face);

        // Free all of the intermediary data structures.
        // Clear input structures.
        // Unmark the output.
        void cleanup();


        // Helper functions.
    private:

        // Application Specific allocation functions.
        // REQUIRE: All allocation function need the graph to be already instantiated.

        Graph * newGraph()
        {
            Graph * output  = new Graph();
            output -> data  = new Graph_Data(output);
            return output;
        }

        Face * newFace()
        {
            Face * output  = graph -> newFace();
            output -> data = new Face_Data(output);
            return output;
        };

        Edge * newEdge()
        {
            Edge * output  = graph -> newEdge();
            output -> data = new Edge_Data(output);
            return output;
        }

        Halfedge * newHalfedge()
        {
            Halfedge * output = graph -> newHalfedge();
            output -> data    = new Halfedge_Data(output);
            return output;
        }

        Vertex * newVertex()
        {
            Vertex * output = graph -> newVertex();
            output -> data  = new Vertex_Data(output);
            return output;
        }
    };
}