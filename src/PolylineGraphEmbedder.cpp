#include "PolylineGraphEmbedder.h"

namespace scrib {

    void PolylineGraphEmbedder::setClosed(bool isClosed)
    {
        closed_loop = isClosed;
    }

    // FIXME: Allow for the embedding of multiple trivial subgraphs.

    Graph * PolylineGraphEmbedder::embedPolylineSet(std::vector< std::vector<ofPoint> *> * inputs)
    {
        // Make sure that the previous data is cleared.
        int len = inputs->size();
        for (int i = 0; i < len; i++)
        {
            loadInput(inputs->at(i));
        }

        return do_the_rest();
    }

    Graph * PolylineGraphEmbedder::embedPolyline(std::vector<ofPoint> * inputs)
    {
        // Handle Trivial Input.
        if (inputs->size() <= 1)
        {
            return trivial(inputs);
        }

        // Make sure that the previous data is cleared.
        loadInput(inputs);

        return do_the_rest();
    }

    inline Graph * PolylineGraphEmbedder::trivial(std::vector<ofPoint> * inputs)
    {
        graph = new Graph();

        if (inputs->size() < 1)
        {
            return graph; // Trivial empty Graph.
        }

        // 1 point Graph.

        // We construct one of each element for the singleton graph.
        // NOTE: This allocation is a wrapper on top of the Graph allocation function, which allocates its Vertex_Data object.
        //       the other functions this->new[ ____ ] work in the same way.
        Vertex * vertex = newVertex();
        Vertex_Data * vertex_data = vertex -> data;
        Edge * edge = newEdge();

        Face * interior = newFace();
        Face * exterior = newFace();
        Face_Data * interior_data = interior -> data;
        Face_Data * exterior_data = exterior -> data;

        Halfedge * halfedge = newHalfedge();
        Halfedge * twin     = newHalfedge();// Somewhat fake, since singleton graphs are degenerate.


        vertex_data->point = inputs -> at(0);

        vertex -> halfedge = halfedge;
        edge   -> halfedge = halfedge;

        // The interior is trivial and is defined by a trivial internal and external null area point boundary.
        interior -> halfedge = halfedge;
        interior_data -> addHole(exterior);

        exterior -> halfedge = halfedge;

        // Self referential exterior loop.
        halfedge -> edge   = edge;
        halfedge -> face   = exterior;
        halfedge -> next   = halfedge;
        halfedge -> prev   = halfedge;
        halfedge -> twin   = twin;
        halfedge -> vertex = vertex;

        // Self referential interior loop.
        twin -> edge   = edge;
        twin -> face   = interior;
        twin -> next   = twin;
        twin -> prev   = twin;
        twin -> twin   = halfedge;
        twin -> vertex = vertex;

        return graph;
    }

    void PolylineGraphEmbedder::loadInput(std::vector<ofPoint> * inputs)
    {
        // Populate the original points.
        int len = inputs -> size();

        // The offset is the initial index of the first input point.
        // We can therefore load multiple input lines and keep the indices distinct.
        int offset = points.size();

        for (int i = 0; i < len; i++)
        {
            ofPoint input_point = inputs -> at(i) + ofPoint(ofRandomf(), ofRandomf());

            // A Paranoid vertical line prevention technique.
            if ((offset > 0 || i > 0) && points[offset + i - 1].x == input_point.x)
            {
                input_point.x += .001;
            }

            points.push_back(input_point);
        }

        // Populate the original lines.
        for (int i = 0; i < len - 1; i++)
        {
            lines_initial.push_back(scrib::Line(i + offset, i + offset + 1, &points));
        }

        // Add a line connecting the first and last points on the original set of input points if
        // the face finder is in closed loop mode.
        // In other words put a duplicate copy of the initial point.
        if (closed_loop)
        {
            // connects last point at index (len - 1 + offset) to the first point, located at index (0 + offset).
            lines_initial.push_back(scrib::Line(len - 1 + offset, 0 + offset, &points));
        }

    }

    inline Graph * PolylineGraphEmbedder::do_the_rest()
    {
        // ASSUMPTION: Step 1. Input Loading has been acomplished.
        // We should have a list of indexed points and index associated edges.

        splitIntersectionPoints();
        allocate_graph_from_input();
        sort_outgoing_edges_by_angle();
        associate_halfedge_cycles();

        Graph * output = deriveFaces();

        cleanup();

        return output;
    }

    void PolylineGraphEmbedder::splitIntersectionPoints()
    {
        scrib::Intersector intersector;

        // Use a custom made O(maximum vertical overlap * log(maximum vertical overlap).
        // Very small constant factors, cache friendly.
        if (bUseFastAlgo)
        {
            intersector.intersect(&lines_initial);
        }
        else
        {
            // Naive brute force algo.
            // N^2. Small constants. As robust as it gets.
            intersector.intersect_brute_force(&lines_initial);
        }

        // Populate the split sequence of lines.
        lines_split.clear();

        int numLines = lines_initial.size();

        // Populates the list of edge disjoint lines that only intersect at vertices.
        // puts the edge in consecutive order following the input polylines.
        for (int i = 0; i < numLines; i++)
        {
            scrib::Line line = lines_initial[i];
            line.getSplitLines(&lines_split);
        }
    }

    void PolylineGraphEmbedder::allocate_graph_from_input()
    {
        graph = newGraph();

        // -- Allocate all Vertices and their outgoing halfedge temporary structure.
        int len = points.size();
        for (int i = 0; i < len; i++)
        {
            Vertex * vert           = newVertex();
            Vertex_Data * vert_data = vert -> data;

            vert -> halfedge = NULL;
            vert_data -> point = points[i];
        }

        // -- Allocate 2 halfedges and 1 full edge for ever line in the split input.
        len = lines_split.size();
        for (int i = 0; i < len; i++)
        {
            newHalfedge();
            newHalfedge();
            newEdge();
        }

        // Associate edges <-> halfedges.
        //           halfedges <-> twin halfedges.
        //           halfedges <-> vertices.
        Halfedge * last_forwards_halfedge = NULL;
        Halfedge * last_backwards_halfedge = NULL;
        int last_index = len * 2 - 1;
        for (int i = 0; i < len; i++)
        {
            Line & line        = lines_split[i];
            int vertex_ID      = line.p1_index;
            int vertex_twin_ID = line.p2_index;
            int edge_ID        = i;
            int halfedge_ID    = i;             // Forwards halfedges with regards to the polyline.
            int twin_ID        = last_index - i;// Backwards halfedges.

            Edge     * edge      = graph -> getEdge(edge_ID);
            Halfedge * halfedge  = graph -> getHalfedge(halfedge_ID); // Forwards facing.
            Halfedge * twin      = graph -> getHalfedge(twin_ID);     // Backwards facing.
            Vertex   * vert      = graph -> getVertex(vertex_ID);
            Vertex   * vert_twin = graph -> getVertex(vertex_twin_ID);

            Vertex_Data * vert_data      = vert -> data;
            Vertex_Data * vert_twin_data = vert_twin -> data;

            // Edge <--> Halfedge.
            edge -> halfedge = halfedge;
            halfedge -> edge = edge;
            twin     -> edge = edge;

            // Halfedge <--> twin Halfedges.
            halfedge -> twin = twin;
            twin     -> twin = halfedge;

            // Halfedge <--> Vertex.

            halfedge -> vertex = vert;
            twin     -> vertex = vert_twin;

            // Here we guranteed that Halfedge h->vertex->halfedge = h iff
            // the halfedge is the earliest halfedge originating from the vertex in the order.

            // FIXME: This no longer seems necessary, because of the outgoing edge structure.
            // Desired properties may be maintained at a later step.

            if (vert -> halfedge == NULL)
            {
                vert -> halfedge = halfedge;
            }

            if (vert_twin -> halfedge == NULL)
            {
                vert_twin -> halfedge = twin;
            }

            // -- We store outgoing halfedges for each vertex in a temporary outgoing edges structure.
            vert_data -> outgoing_edges.push_back(halfedge);
            vert_twin_data -> outgoing_edges.push_back(twin);
        }
    }

    void PolylineGraphEmbedder::sort_outgoing_edges_by_angle()
    {
        // Sort each outgoing edges list.
        Vertex_Iter start = graph -> verticesBegin();
        Vertex_Iter end   = graph -> verticesEnd();
        for (Vertex_Iter iter = start; iter != end; iter++)
        {
            Vertex_Data * vert_data = (*iter) -> data;
            std::vector<Halfedge *> & outgoing_edges = vert_data -> outgoing_edges;
            sort_outgoing_edges(outgoing_edges);
        }
    }

    void PolylineGraphEmbedder::sort_outgoing_edges(std::vector<Halfedge * > & outgoing_edges)
    {
        // Initialize useful information.
        int len = outgoing_edges.size();

        // Less than 2 are already sorted, regardless of orientation.
        if (len <= 2)
        {
            return;
        }

        // Note: len == 3 is sorted, but possibly of the wrong orientation.

        std::vector<float> angles;

        // Extract central information.
        Halfedge    * outgoing_halfedge_representative = outgoing_edges[0];
        Vertex      * center_vert  = outgoing_halfedge_representative -> vertex;
        Vertex_Data * center_data  = center_vert -> data;
        ofPoint       center_point = center_data -> point;

        // Populate the angles array with absolute relative angles.
        for (auto iter = outgoing_edges.begin(); iter != outgoing_edges.end(); iter++)
        {
            Halfedge * out = *iter;
            Halfedge *  in = out -> twin;
            Vertex * outer_vert = in -> vertex;
            Vertex_Data * outer_data = outer_vert -> data;
            ofPoint outer_point      = outer_data -> point;

            float angle = atan2(outer_point.y - center_point.y,
                outer_point.x - center_point.x);
            angles.push_back(angle);
        }

        // Insertion sort based on the angles.
        for (int i = 1; i < len; i++)
        {
            for (int i2 = i - 1; i2 >= 0; i2--)
            {
                int i1 = i2 + 1;

                if (angles[i2] <= angles[i1])
                {
                    break;
                }

                // -- Swap at indices i2 and i2 + 1.
                // Keep the angle measurements synchronized with the halfedges.
                float temp_f = angles.at(i2);
                angles[i2] = angles[i1];
                angles[i1] = temp_f;

                Halfedge * temp_he = outgoing_edges[i2];
                outgoing_edges[i2] = outgoing_edges[i1];
                outgoing_edges[i1] = temp_he;
            }
        }

    }

    void PolylineGraphEmbedder::associate_halfedge_cycles()
    {
        Vertex_Iter start = graph -> verticesBegin();
        Vertex_Iter end   = graph -> verticesEnd();
        for (Vertex_Iter vert = start; vert != end; vert++)
        {
            Vertex_Data * vert_data           = (*vert) -> data;
            vector<Halfedge *> outgoing_edges = vert_data -> outgoing_edges;
            int degree = outgoing_edges.size();

            // Singleton point.
            if (degree == 0)
            {
                vert_data -> singleton_point = true;

                Halfedge * halfedge = (*vert) -> halfedge;
                // ASSERTION: halfedge != null. If construction the user inputs a graph with singleton points.
                // FIXME: Perhaps I should allocate the half edge here for the trivial case. Maybe I should combine the
                // places in my code where I define the singleton state.

                halfedge -> next = halfedge;
                halfedge -> prev = halfedge;
                continue;
            }

            // Tail vertex.
            if (degree == 1)
            {
                vert_data -> tail_point = true;

                Halfedge * out = (*vert) -> halfedge;
                Halfedge * in  = out  -> twin;

                out -> prev = in;
                in  -> next = out;
                continue;
            }

            // Mark junction points.
            if (degree > 2)
            {
                vert_data -> intersection_point = true;
            }

            // Link the halfedge neighborhood.
            for (int i = 0; i < degree; i++)
            {
                Halfedge * out = outgoing_edges[i];
                Halfedge * in  = out -> twin;

                // This combined with the sort order determines the consistent orientation.
                // I think that it defines a clockwise orientation, but I could be wrong.
                
                // FIXME: There is something wrong about this ordering.

                in  -> next  = outgoing_edges[(i + 1) % degree];
                out -> prev  = outgoing_edges[(i + degree - 1) % degree] -> twin;
            }

            continue;
        }
    }

    Graph * PolylineGraphEmbedder::deriveFaces()
    {

        // For each halfedge, output its cycle once.

        Halfedge_Iter start = graph -> halfedgesBegin();
        Halfedge_Iter end   = graph -> halfedgesEnd();

        // Iterate through all originating points.
        for (Halfedge_Iter halfedge = start; halfedge != end; halfedge++)
        {
            Halfedge_Data * halfedge_data = (*halfedge) -> data;

            // Avoid previously traced cycles.
            if (halfedge_data -> marked)
            {
                continue;
            }

            Face * face = newFace(); // GraphEmbedder::newFace() ...

            face -> halfedge = *halfedge;
            trace_face(face);
        }

        // Clear the marks.
        graph -> data -> clearHalfedgeMarks();

        return this -> graph;
    }

    // Isn't this nice and conscise?
    void PolylineGraphEmbedder::trace_face(Face * face)
    {
        Halfedge * start   = face -> halfedge;
        Halfedge * current = start;

        do
        {
            current -> face = face;
            current -> data -> marked = true;
            current = current -> next;
        } while (current != start);
    }

    void PolylineGraphEmbedder::cleanup()
    {
        // Remove the previous data.

        // Erase the stack class allocated memory.
        points.clear();
        lines_initial.clear();
        lines_split.clear();

        // No dynamic allocated temporary structures! Wahoo!
    }

}