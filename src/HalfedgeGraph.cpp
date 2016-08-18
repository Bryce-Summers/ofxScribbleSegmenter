#include "HalfedgeGraph.h"

namespace scrib
{

    Face * Graph::newFace()
    {
        int ID = faces.size();
        faces.push_back(Face());
        Face * output = &faces[ID];
        output->ID = ID;
        return output;
    }
    Vertex * Graph::newVertex()
    {
        int ID = vertices.size();
        vertices.push_back(Vertex());
        Vertex * output = &vertices[ID];
        output->ID = ID;
        return output;
    }

    Edge * Graph::newEdge()
    {
        int ID = edges.size();
        edges.push_back(Edge());
        Edge * output = &edges[ID];
        output->ID = ID;
        return output;
    }

    Halfedge * Graph::newHalfedge()
    {
        int ID = halfedges.size();
        halfedges.push_back(Halfedge());
        Halfedge * output = &halfedges[ID];
        output->ID = ID;
        return output;
    }

}