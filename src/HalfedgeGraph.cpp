#include "HalfedgeGraph.h"

namespace scrib
{

    Face * Graph::newFace()
    {
        int ID = faces.size();
        Face * output = new Face();
        faces.push_back(output);        
        output -> ID = ID;
        return output;
    }
    Vertex * Graph::newVertex()
    {
        int ID = vertices.size();
        Vertex * output = new Vertex();
        vertices.push_back(output);
        output -> ID = ID;
        return output;
    }

    Edge * Graph::newEdge()
    {
        int ID = edges.size();
        Edge * output = new Edge();
        edges.push_back(output);
        output -> ID = ID;
        return output;
    }

    Halfedge * Graph::newHalfedge()
    {
        int ID = halfedges.size();
        Halfedge * output = new Halfedge();
        halfedges.push_back(output);
        output -> ID = ID;
        return output;
    }

}