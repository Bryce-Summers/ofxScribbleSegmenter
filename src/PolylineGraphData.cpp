#include "PolylineGraphData.h"

namespace scrib
{

    void Graph_Data::clearFaceMarks()
    {
        for (Face_Iter iter = graph->facesBegin(); iter != graph->facesEnd(); iter++)
        {
            (*iter) -> data -> marked = false;
        }
    }

    void Graph_Data::clearVertexMarks()
    {
        for (Vertex_Iter iter = graph->verticesBegin(); iter != graph->verticesEnd(); iter++)
        {
            (*iter) -> data -> marked = false;
        }
    }

    void Graph_Data::clearEdgeMarks()
    {
        for (Edge_Iter iter = graph->edgesBegin(); iter != graph->edgesEnd(); iter++)
        {
            (*iter) -> data -> marked = false;
        }
    }

    void Graph_Data::clearHalfedgeMarks()
    {
        for (Halfedge_Iter iter = graph->halfedgesBegin(); iter != graph->halfedgesEnd(); iter++)
        {
            (*iter) -> data -> marked = false;
        }
    }

    void Graph_Data::clearMarks()
    {
        clearFaceMarks();
        clearVertexMarks();
        clearEdgeMarks();
        clearHalfedgeMarks();
    }
}