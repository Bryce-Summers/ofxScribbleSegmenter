#include "../include/FaceFinder.h"

/*
FaceFinder::FaceFinder()
{

}

FaceFinder::~FaceFinder()
{
    //dtor
}*/

namespace scrib{

std::vector< std::vector<point_info> *> * FaceFinder::FindFaces(std::vector< std::vector<ofPoint> *> * inputs)
{
    // Make sure that the previous data is cleared.
    int len = inputs->size();
    for(int i = 0; i < len; i++)
    {
        loadInput(inputs -> at(i));
    }

    return do_the_rest();
}

std::vector< std::vector<point_info> *> * FaceFinder::FindFaces(std::vector<ofPoint> * inputs)
{
    // Make sure that the previous data is cleared.
    loadInput(inputs);

    return do_the_rest();
}

inline std::vector<std::vector<point_info> *> * FaceFinder::do_the_rest()
{
    splitIntersectionPoints();
    convert_to_directedGraph();
    sort_graph_by_edge_angle();

    std::vector< std::vector<point_info> *> * output = deriveFaces();

    cleanup();

    return output;
}

void FaceFinder::loadInput(std::vector<ofPoint> * inputs)
{
    // Populate the original points.
    int len = inputs -> size();
    int offset = points.size();

    for(int i = 0; i < len; i++)
    {
        ofPoint input_point = inputs -> at(i) + ofPoint(ofRandomf(), ofRandomf());

        // A Paranoid vertical line prevention technique.
        if((offset > 0 || i > 0) && points[offset + i - 1].x == input_point.x)
        {
            input_point.x += .001;
        }

        points.push_back(input_point);
    }

    // Populate the original lines.
    for(int i = 0; i < len - 1; i++)
    {
        lines_initial.push_back(scrib::Line(i + offset, i + offset + 1, &points));
    }

}

void FaceFinder::splitIntersectionPoints()
{

    scrib::Intersector intersector;

    // Use a custom made O(maximum vertical overlap * log(maximum vertical overlap).
    // Very small constant factors, cache friendly.
    if(bUseFastAlgo)
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

    for(int i = 0; i < numLines; i++)
    {
        scrib::Line line = lines_initial[i];
        line.getSplitLines(&lines_split);
    }
}

void FaceFinder::convert_to_directedGraph()
{
    // Initialize the map based directed graph structure.
    //directed_graph   = new std::map<int, std::vector<int> *> ();
    //output_predicate = new std::map<int, std::vector<bool> *>();

    int numPoints = points.size();
    for(int i = 0; i < numPoints; i++)
    {
        directed_graph[i]   = new std::vector<int>();
        output_predicate[i] = new std::vector<bool>();
    }

    // Add all of the lines to the directed graph structure.
    int numLines = lines_split.size();
    for(int i = 0; i < numLines; i++)
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
    for(int i = 0; i < numPoints; i++)
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
    for(int i = 0; i < len; i++)
    {
        int point_index = outgoing_indices->at(i);
        ofPoint point   = points[point_index];

        float angle     = atan2(point.y - center.y, point.x - center.x);
        angles.push_back(angle);
    }

    // Insertion sort based on the angles.
    for(int i = 1; i < len; i++)
    {
        for(int i2 = i - 1; i2 >= 0; i2--)
        {
            int i1 = i2 + 1;

            if(angles[i2] <= angles[i1])
            {
                break;
            }


            // -- Swap at indices i2 and i2 + 1.
            // Keep the angle measurements synchronized with the
            float temp_f = angles.at(i2);
            angles[i2]   = angles[i1];
            angles[i1]   = temp_f;

            int temp_i   = outgoing_indices -> at(i2);
            outgoing_indices -> at(i2) = outgoing_indices -> at(i1);
            outgoing_indices -> at(i1) = temp_i;
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
    for(int point_index = 0; point_index < numPoints; point_index++)
    {
        std::vector<int>  * outgoing_vertices   = directed_graph[point_index];
        std::vector<bool> * outgoing_predicates = output_predicate[point_index];

        int numEdges = outgoing_vertices -> size();

        // Iterate through all outgoing edges at each point.
        for(int edge_index = 0; edge_index < numEdges; edge_index++)
        {
            // Skip Edges that have already been processed.
            if(outgoing_predicates -> at(edge_index))
            {
                continue;
            }

            int p2 = outgoing_vertices -> at(edge_index);
            output -> push_back(getCycle(point_index, p2, edge_index));
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
        setPredicate(i1, outgoing_index);
        output->push_back(point_info(points[i2], i2));// i2 is the Global index.
        getNextEdge(&i1, &i2, &outgoing_index);
    }
    while(i1 != p1_original || i2 != p2_original);

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
    int len = p2_edges -> size();

    int p2_outgoing_index = (index + 1) % len;
    int p3 = p2_edges->at(p2_outgoing_index);

    *p1_index = p2;
    *p2_index = p3;
    *outgoing_index = p2_outgoing_index;

}

void FaceFinder::setPredicate(int p1, int p2_index)
{
    std::vector<bool> * predicates = output_predicate[p1];
    predicates -> at(p2_index) = true;
}

int FaceFinder::find_outgoing_index_of_edge(int p1, int p2)
{
    std::vector<int> * edges = directed_graph[p1];

    int len = edges->size();

    // Search for the index.
    for(int i = 0; i < len; i++)
    {
        if(edges -> at(i) == p2)
        {
            return i;
        }
    }

    cout<< "Error: Reverse edge not found."<<endl;
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

    for(std::map<int, std::vector<int> *>::iterator iter = directed_graph.begin(); iter != directed_graph.end(); ++iter)
    {
        //iter->first;
        delete iter->second;

    }

    directed_graph.clear();

    //std::map<int, std::vector<bool> *> output_predicate;

    for(std::map<int, std::vector<bool> *>::iterator iter = output_predicate.begin(); iter != output_predicate.end(); ++iter)
    {
        //iter->first;
        delete iter->second;
    }

    output_predicate.clear();

}

void FaceFinder::determineExternalFaces(std::vector<std::vector<point_info> *> * input, std::vector<int> * output)
{
    int len = input -> size();

    for(int index = 0; index < len; index++)
    {
        float area = computeAreaOfPolygon(input->at(index));

        if(area > 0)
        {
            output->push_back(index);
        }
    }
}


float FaceFinder::computeAreaOfPolygon(std::vector<point_info> * closed_polygon)
{
    int len = closed_polygon -> size();
    ofPoint * p1 = &(closed_polygon -> at(len - 1).point);

    float area = 0.0;

    // Compute based on Green's Theorem.
    for(int i = 0; i < len; i++)
    {
        ofPoint * p2 = &(closed_polygon->at(i).point);
        area += (p2 -> x + p1 -> x)*(p2->y - p1->y);
        p1 = p2;// Shift p2 to p1.
    }

    return area/2.0;
}

}
