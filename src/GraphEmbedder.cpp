#include "GraphEmbedder.h"

namespace scrib {

    void GraphEmbedder::setClosed(bool isClosed)
    {
        closed_loop = isClosed;
    }

    Graph * GraphEmbedder::embedPolylineSet(std::vector< std::vector<ofPoint> *> * inputs)
    {
        // Make sure that the previous data is cleared.
        int len = inputs -> size();
        for (int i = 0; i < len; i++)
        {
            loadInput(inputs->at(i));
        }

        return do_the_rest();
    }

    Graph * GraphEmbedder::embedPolyline(std::vector<ofPoint> * inputs)
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

    inline Graph * GraphEmbedder::trivial(std::vector<ofPoint> * inputs)
    {
        Graph * output = new Graph();

        if (inputs->size() < 1)
        {
            return output; // Trivial empty Graph.
        }

        // 1 point Graph.
        
        // We construct one of each element for the singleton graph.
        // NOTE: This allocation is a wrapper on top of the Graph allocation function, which allocates its Vertex_Data object.
        //       the other functions this->new[____] work in the same way.
        Vertex * vertex = newVertex();
		Vertex_Data * vertex_data = vertex -> data;
        Edge * edge     = newEdge();

        Face * interior = newFace();
        Face * exterior = newFace();
		Face_Data * interior_data = interior -> data;
		Face_Data * exterior_data = exterior -> data;

        Halfedge * halfedge = newHalfedge();
        Halfedge * twin     = newHalfedge();// Somewhat fake, since singleton graphs are degenerate.


        vertex_data -> point = inputs -> at(0);

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
    }

    void GraphEmbedder::loadInput(std::vector<ofPoint> * inputs)
    {
        // Populate the original points.
        int len = inputs -> size();

        // The offset is the initial index of the first input point.
        // We can therefore load multiple input lines and keep the indices distinct.
        int offset = points.size();

        for (int i = 0; i < len; i++)
        {
            ofPoint input_point = inputs->at(i) + ofPoint(ofRandomf(), ofRandomf());

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

    inline Graph * GraphEmbedder::do_the_rest()
    {
        splitIntersectionPoints();
        allocate_graph_from_input();
        associate_halfedge_cycles();
        sort_graph_by_edge_angle();

        Graph * output = deriveFaces();

        cleanup();

        return output;
    }

    void GraphEmbedder::splitIntersectionPoints()
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

    void GraphEmbedder::allocate_graph_from_input()
    {
        graph = new Graph();

        // -- Allocate all Vertices.
        int len = points.size();
        for (int i = 0; i < len; i++)
        {
            Vertex * vert = newVertex();
			Vertex_Data * vert_data = vert -> data;
            
            vert -> halfedge = NULL;
			vert_data -> point = points[i];
        }

        // -- Allocate 2 halfedges and 1 full edge for ever line in the split input.
        std::vector<scrib::Line> lines_split;
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
		// Naively associate halfedges <-> next and previous halfedges, they will be fixed in the intersection proccessing step.
		Halfedge * last_forwards_halfedge  = NULL;
		Halfedge * last_backwards_halfedge = NULL;
        int last_index = len*2 - 1;
        for (int i = 0; i < len; i++)
        {
            Line     & line = lines_split[i];
            int vertex_ID      = line.p1_index;
            int vertex_twin_ID = line.p2_index;
            int edge_ID = i;
            int halfedge_ID = i;         // Forwards halfedges with regards to the polyline.
            int twin_ID = last_index - i;// Backwards halfedges.

            Edge     * edge      = graph -> getEdge(edge_ID);
            Halfedge * halfedge  = graph -> getHalfedge(halfedge_ID); // Forwards facing.
            Halfedge * twin      = graph -> getHalfedge(twin_ID);     // Backwards facing.
            Vertex   * vert      = graph -> getVertex(vertex_ID);
            Vertex   * vert_twin = graph -> getVertex(vertex_twin_ID);

			Vertex_Data * vert_data      = vert      -> data;
			Vertex_Data * vert_twin_data = vert_twin -> data;

            // Edge <--> Halfedge.
            edge     -> halfedge = halfedge;
            halfedge -> edge     = edge;
            twin     -> edge     = edge;

            // Halfedge <--> twin Halfedges.
            halfedge -> twin = twin;
            twin -> twin     = halfedge;

            // Halfedge <--> Vertex.

            halfedge  -> vertex = vert;
            twin      -> vertex = vert_twin;

            // Here we guranteed that Halfedge h->vertex->halfedge = h iff
            // the halfedge is the earliest halfedge originating from the vertex in the order.
            // We are also able to detect and mark all ending tail points.
            if (vert -> halfedge == NULL)
            {
                vert -> halfedge = halfedge;
                vert_data -> ending_tail_point = false;
            }

            if (vert_twin -> halfedge == NULL)
            {
                vert_twin -> halfedge = twin;
                vert_twin_data -> ending_tail_point = true;
            }

			// Naive halfedges <-> next and previous halfedges.

			// Handle starting pair in the polyline.
			// This is a bad conditional branch, and it may be a bit confusing. The good news is that it allows the code to run correctly
			// with no else statement which simplifies reosoning about this code.
			if (last_forwards_halfedge == NULL)
			{
				last_forwards_halfedge  = twin;
				last_backwards_halfedge = halfedge;
			}

			// Current iteration --> previous.
			halfedge -> prev = last_forwards_halfedge;
			twin -> next = last_backwards_halfedge;

			// Last iteration --> current.
			last_forwards_halfedge  -> next = halfedge;
			last_backwards_halfedge -> prev = twin;

			// Update the last forwards and backwards halfedges from the last iteration.
			last_forwards_halfedge  = halfedge;
			last_backwards_halfedge = twin;
        }
    }

    void GraphEmbedder::associate_halfedge_cycles()
    {

        // IDEA: If all intersections contain at most 2 lines, and therefore an earlier and later pair of incoming and outgoing edges,
        // then if the vertex has its canonical halfedge pointer facing the newer one, then we simply need to compute the correct orientation and
        // set the next pointers. We can actually set the next and previous pointers for the majority of the elements naively first and then just fix the pointers at the intersections.

        // I also want to have references for each vertex to their next major vertex (End point or intersection point.)
        // Food for thought, can this be done without doing arcCos or touching the ofPoints? That would be miraculous.
        // I think that right now they are being atan2'd, which is a bit ugly.

        // FIXME: Goodnight.

        int numPoints = points.size();
        for (int i = 0; i < numPoints; i++)
        {
            // Note: map associations may safely use index notation with no prior association.
            directed_graph[i] = new std::vector<Halfedge *>();
        }

        // Add all of the lines to the directed graph structure.
        int numLines = lines_split.size();
        for (int i = 0; i < numLines; i++)
        {
            scrib::Line * line = &(lines_split[i]);

            int index_a = line->p1_index;
            int index_b = line->p2_index;

            // Add both directions.
            addDirectedEdge(index_a, index_b);
            addDirectedEdge(index_b, index_a);

        }

    }

    // Add the directed edge from the point with index_a to the point with index_b;
    void FaceFinder::addDirectedEdge(int index_a, int index_b)
    {
        // Edge specification.
        std::vector<int> * outgoing_indices = directed_graph[index_a];
        outgoing_indices->push_back(index_b);

        // Output inclusion predicate initialization.
        std::vector<bool> * outgoing_predicates = output_predicate[index_a];
        outgoing_predicates->push_back(false);
    }

    void FaceFinder::sort_graph_by_edge_angle()
    {
        // Sort each outgoing edges list.
        int numPoints = points.size();
        for (int i = 0; i < numPoints; i++)
        {

            std::vector<int> * outgoing_indices = directed_graph[i];
            sort_vertice_by_edge_angle(i, outgoing_indices);

        }
    }

    void FaceFinder::sort_vertice_by_edge_angle(int center_point_index, std::vector<int> * outgoing_indices)
    {
        // Initialize useful information.
        int len = outgoing_indices->size();
        std::vector<float> angles;
        ofPoint center = points[center_point_index];

        // Populate the angles array with absolute relative angles.
        for (int i = 0; i < len; i++)
        {
            int point_index = outgoing_indices->at(i);
            ofPoint point = points[point_index];

            float angle = atan2(point.y - center.y, point.x - center.x);
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
                // Keep the angle measurements synchronized with the
                float temp_f = angles.at(i2);
                angles[i2] = angles[i1];
                angles[i1] = temp_f;

                int temp_i = outgoing_indices->at(i2);
                outgoing_indices->at(i2) = outgoing_indices->at(i1);
                outgoing_indices->at(i1) = temp_i;
            }
        }

    }

    std::vector< std::vector<point_info> *> * FaceFinder::deriveFaces()
    {
        // -- Initialize Output Structures.
        std::vector< std::vector<point_info> *> * output = new std::vector< std::vector<point_info> *>();

        // For all edges, output their cycle once.

        int numPoints = points.size();

        // Iterate through all originating points.
        for (int point_index = 0; point_index < numPoints; point_index++)
        {
            std::vector<int>  * outgoing_vertices = directed_graph[point_index];
            std::vector<bool> * outgoing_predicates = output_predicate[point_index];

            int numEdges = outgoing_vertices->size();

            // Iterate through all outgoing edges at each point.
            for (int edge_index = 0; edge_index < numEdges; edge_index++)
            {
                // Skip Edges that have already been processed.
                if (outgoing_predicates->at(edge_index))
                {
                    continue;
                }

                int p2 = outgoing_vertices->at(edge_index);
                output->push_back(getCycle(point_index, p2, edge_index));
            }
        }

        return output;
    }

    std::vector<point_info> * FaceFinder::getCycle(int p1_original, int p2_original, int p2_original_outgoing_index)
    {
        std::vector<point_info> * output = new std::vector<point_info>();

        int i1 = p1_original;
        int i2 = p2_original;

        // The outgoing index is always the index such that p2 = dg[p1][outgoing_index].
        int outgoing_index = p2_original_outgoing_index;

        // Iterate until we have come back to the beginning of the cycle.
        // Push points on the cycle at each point in time.
        do
        {
            setPredicate(i1, outgoing_index);// Mark the half edge to avoid redundant processing.
            output->push_back(point_info(points[i2], i2));// i2 is the Global index.
            getNextEdge(&i1, &i2, &outgoing_index);
        } while (i1 != p1_original || i2 != p2_original);

        return output;
    }

    void FaceFinder::getNextEdge(int * p1_index, int * p2_index, int * outgoing_index)
    {
        int p1 = *p1_index;
        int p2 = *p2_index;

        // The outgoing vertices for edges originating at p2;
        std::vector<int> * p2_edges = directed_graph[p2];

        // Search for the reverse edge.
        int index = find_outgoing_index_of_edge(p2, p1);

        // Compute the next edge in sorted order.
        int len = p2_edges->size();

        int p2_outgoing_index = (index + 1) % len;
        int p3 = p2_edges->at(p2_outgoing_index);

        *p1_index = p2;
        *p2_index = p3;
        *outgoing_index = p2_outgoing_index;

    }

    void FaceFinder::setPredicate(int p1, int p2_index)
    {
        std::vector<bool> * predicates = output_predicate[p1];
        predicates->at(p2_index) = true;
    }

    int FaceFinder::find_outgoing_index_of_edge(int p1, int p2)
    {
        std::vector<int> * edges = directed_graph[p1];

        int len = edges->size();

        // Search for the index.
        for (int i = 0; i < len; i++)
        {
            if (edges->at(i) == p2)
            {
                return i;
            }
        }

        cout << "Error: Reverse edge not found." << endl;
        exit(-7);

        // Not found.
        return -1;
    }

    void FaceFinder::cleanup()
    {
        // Remove the previous data.

        // Erase the stack class allocated memory.
        points.clear();
        lines_initial.clear();
        lines_split.clear();

        // -- Deallocate the memory heap allocated in this class.

        //std::map<int, std::vector<int> *> directed_graph

        for (std::map<int, std::vector<int> *>::iterator iter = directed_graph.begin(); iter != directed_graph.end(); ++iter)
        {
            //iter->first;
            delete iter->second;

        }

        directed_graph.clear();

        //std::map<int, std::vector<bool> *> output_predicate;

        for (std::map<int, std::vector<bool> *>::iterator iter = output_predicate.begin(); iter != output_predicate.end(); ++iter)
        {
            //iter->first;
            delete iter->second;
        }

        output_predicate.clear();

    }

    void FaceFinder::determineExternalFaces(std::vector<std::vector<point_info> *> * input, std::vector<int> * output)
    {
        int len = input->size();

        for (int index = 0; index < len; index++)
        {
            float area = computeAreaOfPolygon(input->at(index));

            if (area > 0)
            {
                output->push_back(index);
            }
        }
    }

    void FaceFinder::determineNonTrivialAreaFaces(std::vector<std::vector<point_info> *> * input, std::vector<int> * output, float min_area)
    {
        int len = input->size();

        for (int index = 0; index < len; index++)
        {
            float area = computeAreaOfPolygon(input->at(index));

            // Absolute value to account for external faces.
            area = area >= 0 ? (area) : (-area);

            if (area >= min_area)
            {
                output->push_back(index);
            }
        }
    }

    void FaceFinder::determineTrivialAreaFaces(std::vector<std::vector<point_info> *> * input, std::vector<int> * output, float min_area)
    {
        int len = input->size();

        for (int index = 0; index < len; index++)
        {
            float area = computeAreaOfPolygon(input->at(index));

            // Absolute value to account for external faces.
            area = area >= 0 ? (area) : (-area);

            if (area < min_area)
            {
                output->push_back(index);
            }
        }
    }

    std::vector<std::vector<point_info> *> * FaceFinder::clipTails(std::vector<std::vector<point_info> *> * input)
    {
        std::vector<std::vector<point_info> *> * output = new std::vector<std::vector<point_info> *>();

        int len = input->size();

        for (int index = 0; index < len; index++)
        {
            std::vector<point_info> * unclipped_face = input->at(index);
            std::vector<point_info> * clipped_face = clipTails(unclipped_face);

            // Append only non trivial faces to the output.
            if (clipped_face->size() > 0)
            {
                output->push_back(clipped_face);
            }
        }

        return output;
    }

    // Note: May return 0 point polygons if given trivial polygons.
    // < 3 point inputs --> 0 point outputs!
    std::vector<point_info> * FaceFinder::clipTails(std::vector<point_info> * input)
    {
        std::vector<point_info> * output = new std::vector<point_info>();

        int len = input->size();

        // Faces cannot enclose area if they have less than 3 vertices.
        // This pre check rules out trivial input without a start and end point.
        if (len < 3)
        {
            return output; // EMPTY.
        }

        int p_start = (input->at(0)).ID;
        int p_end = (input->at(len - 1)).ID;

        // A polygon is closed if it has identical starting and ending points.
        bool closed = (p_start == p_end);

        // For now, we will pretend that the duplicated point does not exist by pruning it.
        // Don't worry, we will add it back later.
        if (closed)
        {
            len -= 1;
            p_end = (input->at(len - 1)).ID;
        }

        // We again remove input that cannot possibly enclose an area.
        // We do this again to rule out stuff like single line segments input in closed mode.
        if (len < 3)
        {
            return output; // EMPTY.
        }


        // The main idea behind tail clipping is to transform regions of the form ABA --> A,
        // in other words removing any consecutive pairs of half edges cooresponding to the same full edge.
        // We therefore

        // Stores whether the last iteration of the loop clipped a tail ending.
        // We can use this to ensure that the loop goes around the starting point to properly clip all tails.
        bool clipped_previous = false;
        bool non_empty_output = false;

        // NOTE: we will be using the len variable in this routine to gradually cull the back of the list as needed which
        //       wraps around the arbitrary list starting point location.

        for (int i = 0; i < len; i++)
        {
            // Determine the nearest previous unpruned point, which will be pruned if it is mirrored by the next point.
            int p_previous;
            non_empty_output = output->size() > 0;

            // A non pruned point exists in the output.
            if (non_empty_output)
            {
                p_previous = (output->back()).ID;
            }
            else //Otherwise use the unpruned point at the tail of the unpruned list prefix sublist.
            {
                p_previous = (input->at(len - 1)).ID;
            }

            int p_next = (input->at((i + 1) % len)).ID;

            // If haven't locally detected a tail, then we simply push the point onto the output.
            if (p_previous != p_next)
            {
                output->push_back(input->at(i%len));
                clipped_previous = false;
                continue;
            }

            // Actually prune the current point and the previous point.
            clipped_previous = true;

            if (non_empty_output)
            {
                // Prune output point.
                output->pop_back();
            }
            else
            {
                // Prune tail point.
                len -= 1;
            }

            // Don't add the current point, because we prune it as well.
            // If p_next ends up now being a non tail point, it will be successfully added during the next iteration.
            // We don't add it now, because we want to give it the opportunity to pruned by its next neighbor.
            continue;

        }

        // Due to the wrap around nature of face loops, the beginning may be in the middle of a tail,
        // so, if necessary, we now need to prune the beginning of the output

        // We will now prune the beginning of the output.
        int prune_num = 0;
        while (clipped_previous)// Essentially a while true loop if clipped_previous is true at the end of the first pass.
        {
            int len = output->size();

            // Entire face has been pruned.
            if (len < 3)
            {
                return output;
            }

            int p_previous = (output->at(len - 1)).ID;
            int p_next = (output->at(prune_num + 1)).ID;

            if (p_previous != p_next)
            {
                break;
            }

            // Prune the head and tail of the output.
            prune_num += 1;
            output->pop_back();
            continue;
        }

        // If the wrap pruning pass was done, we need to downsize the array to exclude the no longer relevant data.
        if (clipped_previous)
        {
            output->erase(output->begin(), output->begin() + prune_num);
        }

        // Here we add the duplicate point if the input was closed.
        if (closed && non_empty_output)
        {
            point_info start = output->at(0);
            output->push_back(start);
        }

        return output;
    }

}