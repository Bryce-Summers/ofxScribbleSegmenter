#include "PolylineGraphPostProcessor.h"

namespace scrib
{

    /*
    PolylineGraphPostProcessor::PolylineGraphPostProcessor()
    {
    }


    PolylineGraphPostProcessor::~PolylineGraphPostProcessor()
    {
    }
    */

    float computeAreaOfPolygon(Point_Vector_Format * closed_polygon)
    {
        int len = closed_polygon->size();
        ofPoint * p1 = &(closed_polygon->at(len - 1).point);

        float area = 0.0;

        // Compute based on Green's Theorem.
        for (int i = 0; i < len; i++)
        {
            ofPoint * p2 = &(closed_polygon->at(i).point);
            area += (p2->x + p1->x)*(p2->y - p1->y);
            p1 = p2;// Shift p2 to p1.
        }

        return area / 2.0;
    }

    float computeAreaOfPolygon(OF_Point_Vector_Format * closed_polygon)
    {
        int len = closed_polygon->size();
        ofPoint * p1 = &(closed_polygon->at(len - 1));

        float area = 0.0;

        // Compute based on Green's Theorem.
        for (int i = 0; i < len; i++)
        {
            ofPoint * p2 = &(closed_polygon->at(i));
            area += (p2->x + p1->x)*(p2->y - p1->y);
            p1 = p2;// Shift p2 to p1.
        }

        return area / 2.0;
    }

    bool  isComplemented(Point_Vector_Format * closed_polygon)
    {
        return computeAreaOfPolygon(closed_polygon) > 0;
    }

    bool  isComplemented(OF_Point_Vector_Format * closed_polygon)
    {
        return computeAreaOfPolygon(closed_polygon) > 0;
    }

    Face_Vector_Format * PolylineGraphPostProcessor::convert_to_face_vectors()
    {
        Face_Vector_Format * output = new Face_Vector_Format();

        Face_Iter start = graph->facesBegin();
        Face_Iter end = graph->facesEnd();

        for (Face_Iter face = start; face != end; face++)
        {
            Point_Vector_Format * face_output = new Point_Vector_Format();

            Halfedge * starting_half_edge = (*face) -> halfedge;
            Halfedge * current = starting_half_edge;

            // Convert the entire face into point info objects.
            do
            {
                Vertex * vert           = current -> vertex;
                Vertex_Data * vert_data = vert    -> data;

                ofPoint point = vert_data->point;
                int ID = vert -> ID;

                face_output -> push_back(point_info(point, ID, current));

                // Iterate.
                current = current->next;
            } while (starting_half_edge != current);

            output -> push_back(face_output);
        }

        return output;
    }

    void PolylineGraphPostProcessor::determineComplementedFaces(std::vector<int> * output)
    {
        Face_Vector_Format * input = face_vector;

        int len = input -> size();

        for (int index = 0; index < len; index++)
        {
            float area = computeAreaOfPolygon(input -> at(index));

            if (area > 0)
            {
                output -> push_back(index);
            }
        }
    }

    void PolylineGraphPostProcessor::determineNonTrivialAreaFaces(Int_Vector_Format * output, float min_area)
    {
        Face_Vector_Format * input = face_vector;

        int len = input -> size();

        for (int index = 0; index < len; index++)
        {
            float area = computeAreaOfPolygon(input -> at(index));

            // Absolute value to account for external faces.
            area = area >= 0 ? (area) : (-area);

            if (area >= min_area)
            {
                output -> push_back(index);
            }
        }
    }

    void PolylineGraphPostProcessor::determineTrivialAreaFaces(Int_Vector_Format * output, float min_area)
    {
        Face_Vector_Format * input = face_vector;

        int len = input -> size();

        for (int index = 0; index < len; index++)
        {
            float area = computeAreaOfPolygon(input -> at(index));

            // Absolute value to account for external faces.
            area = area >= 0 ? (area) : (-area);

            if (area < min_area)
            {
                output -> push_back(index);
            }
        }
    }

    Face_Vector_Format * PolylineGraphPostProcessor::clipTails()
    {
        Face_Vector_Format * input  = face_vector;
        Face_Vector_Format * output = new Face_Vector_Format();

        int len = input -> size();

        for (int index = 0; index < len; index++)
        {
            Point_Vector_Format * unclipped_face = input -> at(index);
            Point_Vector_Format * clipped_face   = clipTails(unclipped_face);

            // Append only non trivial faces to the output.
            if (clipped_face -> size() > 0)
            {
                output -> push_back(clipped_face);
            }
        }

        return output;
    }

    // Note: May return 0 point polygons if given trivial polygons.
    // < 3 point inputs --> 0 point outputs!
    Point_Vector_Format * PolylineGraphPostProcessor::clipTails(Point_Vector_Format * input)
    {
        Point_Vector_Format * output = new Point_Vector_Format();

        int len = input -> size();

        // Faces cannot enclose area if they have less than 3 vertices.
        // This pre check rules out trivial input without a start and end point.
        if (len < 3)
        {
            return output; // EMPTY.
        }

        int p_start = (input -> at(0)).ID;
        int p_end   = (input -> at(len - 1)).ID;

        // A polygon is closed if it has identical starting and ending points.
        bool closed = (p_start == p_end);

        // For now, we will pretend that the duplicated point does not exist by pruning it.
        // Don't worry, we will add it back later.
        if (closed)
        {
            len -= 1;
            p_end = (input -> at(len - 1)).ID;
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
            non_empty_output = output -> size() > 0;

            // A non pruned point exists in the output.
            if (non_empty_output)
            {
                p_previous = (output -> back()).ID;
            }
            else //Otherwise use the unpruned point at the tail of the unpruned list prefix sublist.
            {
                p_previous = (input -> at(len - 1)).ID;
            }

            int p_next = (input -> at((i + 1) % len)).ID;

            // If haven't locally detected a tail, then we simply push the point onto the output.
            if (p_previous != p_next)
            {
                output -> push_back(input -> at(i % len));
                clipped_previous = false;
                continue;
            }

            // Actually prune the current point and the previous point.
            clipped_previous = true;

            if (non_empty_output)
            {
                // Prune output point.
                output -> pop_back();
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
            int len = output -> size();

            // Entire face has been pruned.
            if (len < 3)
            {
                return output;
            }

            int p_previous = (output -> at(len - 1)).ID;
            int p_next = (output -> at(prune_num + 1)).ID;

            if (p_previous != p_next)
            {
                break;
            }

            // Prune the head and tail of the output.
            prune_num += 1;
            output -> pop_back();
            continue;
        }

        // If the wrap pruning pass was done, we need to downsize the array to exclude the no longer relevant data.
        if (clipped_previous)
        {
            output -> erase(output -> begin(), output -> begin() + prune_num);
        }

        // Here we add the duplicate point if the input was closed.
        if (closed && non_empty_output)
        {
            point_info start = output -> at(0);
            output -> push_back(start);
        }

        return output;
    }

    

    std::vector<face_info *> * PolylineGraphPostProcessor::mergeFaces(ID_Set * face_ID_set)
    {
        // Temporarily store the raw list of face_info's.
        std::vector < face_info *> faces_uncomplemented;
        std::vector < face_info *> faces_complemented;

        // Go through all halfedges in all relevant faces and trace any representational union faces one time each.
        for (auto iter = face_ID_set -> begin(); iter != face_ID_set -> end(); iter++)
        {
            Face     * face    = graph -> getFace(*iter);
            Halfedge * start   = face  -> halfedge;
            Halfedge * current = face  -> halfedge;
            do
            {
                if (current -> data -> marked == false && _halfedgeInUnion(face_ID_set, current))
                {
                    face_info * face = _traceUnionFace(face_ID_set, current);
                    if (!isComplemented(&(face -> points)))
                    {
                        face -> complemented = false;
                        faces_uncomplemented.push_back(face);
                    }
                    else
                    {
                        face -> complemented = true;
                        faces_complemented.push_back(face);
                    }
                }

                // Try the next edge.
                current = current -> next;
            } while (current != start);
        }

        // Clear markings.
        graph -> data -> clearHalfedgeMarks();

        // Now we associate face_info objects with their internal complemented hole objects.
        std::vector<face_info *> * output = new std::vector<face_info *>();
        
        std::map<int, face_info *> map;

        // On the first pass we add all exterior faces to the output and the map.
        for (auto iter = faces_uncomplemented.begin(); iter != faces_uncomplemented.end(); iter++)
        {
            face_info * face = *iter;

            output -> push_back(face);
            ID_Set & set = face -> faces_ID_set;
            for (auto id = set.begin(); id != set.end(); id++)
            {
                map[*id] = face;
            }

        }

        // On the second pass we add all complemented faces to the proper uncomplemented face hole set.
        // FIXME: Think about what will happen if their is a complemented face that should be by itself.
        //        What about merging a complemented and uncomplemented face.
        for (auto iter = faces_complemented.begin(); iter != faces_complemented.end(); iter++)
        {
            face_info * face = *iter;
            int ID = *(face -> faces_ID_set.begin());
            std::map<int, face_info *>::const_iterator it = map.find(ID);

            // If the index is not associated with an uncomplemented face, then this uncomplemented face must be singleton,
            // instead of a hole. We therefore add it to the direct output.
            if (it == map.end())
            {
                output -> push_back(face);
                continue;
            }

            // Otherwise we add it as a hole to the relevant face.
            face_info * uncomplemented_face = it -> second;
            uncomplemented_face -> holes.push_back(face);
        }

        return output;
    }

    bool PolylineGraphPostProcessor::_halfedgeInUnion(ID_Set * face_ID_set, Halfedge * start)
    {
        Face * face = start -> face;
        int face_ID = face  -> ID;
        ID_Set::const_iterator face_iter = face_ID_set -> find(face_ID);

        Face * twin_face = start -> twin -> face;
        int twin_ID = twin_face -> ID;
        ID_Set::const_iterator twin_iter = face_ID_set -> find(twin_ID);

        // true iff Twin face not in the set of faces in the union.
        auto NOT_FOUND = face_ID_set -> end();

        // NOTE: Tail edges with the same Face on both sides are treated as not in the union,

        return face_iter != NOT_FOUND && twin_iter == NOT_FOUND;
    }

    face_info * PolylineGraphPostProcessor::_traceUnionFace(ID_Set * face_ID_set, Halfedge * start)
    {
        face_info * output = new face_info();
        ID_Set & output_ID_set = output -> faces_ID_set;

        // We only need to worry about tracing output points,
        // because the holes will associated later on from faces tracede with this function.
        Point_Vector_Format & output_points = output -> points;

        Halfedge * current = start;
        do
        {
            current -> data -> marked = true;

            // Output current point (with halfedge).
            point_info current_point = halfedgeToPointInfo(current);
            output_points.push_back(current_point);
            int current_face_ID = current -> face -> ID;
            output_ID_set.insert(current_face_ID);

            // Transition to the next halfedge along this union face.
            current = nextUnionFace(face_ID_set, current);

        } while (current != start);

        return output;
    }

    Halfedge * PolylineGraphPostProcessor::nextUnionFace(ID_Set * face_ID_Set, Halfedge * current)
    {
        // Go around the star backwards.

        // Transition from the incoming current edge to the backmost candidate outgoing edge.
        current = current -> twin -> prev -> twin;
        
        // NOTE: WE could theoretically put in an infinite loop check here, because this code will fail if the graph is malformed.

        // Keep trying out candidate outgoing faces, until we find the first one that works.
        while(!_halfedgeInUnion(face_ID_Set, current))
        {
            // The cycling operations come in two forms, since we flip our orientation after each path change attempt.
            current = current -> prev -> twin;   
        }

        return current;
    }

    point_info PolylineGraphPostProcessor::halfedgeToPointInfo(Halfedge * halfedge)
    {
        Vertex * vertex           = halfedge -> vertex;
        Vertex_Data * vertex_data = vertex   -> data;
        return point_info(vertex_data -> point, vertex -> ID, halfedge);
    }
}