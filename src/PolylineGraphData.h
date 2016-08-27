#pragma once

#include "ofMain.h"
#include "HalfedgeGraph.h"

namespace scrib
{
    // Forward Declaration of Associated Data Objects.
    class Graph_Data;
    class Vertex_Data;
    class Face_Data;
    class Halfedge_Data;
    class Edge_Data;

    class Graph_Data
    {
    private: Graph * graph;

    public:

        Graph_Data(Graph * graph)
        {
            this -> graph = graph;
        }

        void clearFaceMarks();
        void clearVertexMarks();
        void clearEdgeMarks();
        void clearHalfedgeMarks();

        void clearMarks();

    };

    class Face_Data
    {
    private:
        Face * face;

    public:

        Face_Data(Face * face)
        {
            this -> face = face;
        }

        bool marked = false;

        std::vector<Face *> hole_representatives;

        void addHole(Face * hole)
        {
            hole_representatives.push_back(hole);
        }

        // The area of the face is determined by the intersection this face with all of the hole faces,
        // which will be specified by exterior facing edge that enlose an infinite complemented area.
    };

    class Vertex_Data
    {
    private:

        Vertex * vertex;

    public:

        Vertex_Data(Vertex * vertex)
        {
            this->vertex = vertex;
        }

        ofPoint point;
        bool marked = false;

        bool tail_point = false;

        // Labels Vertices that have more than two outgoing edges.
        bool intersection_point = false;

        // FIXME: Remove this if it is just taking up unneeded space.
        bool singleton_point = false;

        bool isExtraordinary()
        {
            return tail_point || intersection_point;
        }

        // Used as a temporary structure for graph construction, but it is also may be relevant to users.
        // I don't know whether I will maintain this structure outside of graph construction.
        // FIXME: I might switch this to being a pointer to allow for me to null it out when no longer needed.
        std::vector<Halfedge *> outgoing_edges;
    };

    class Edge_Data
    {
    private:

        // Halfedge Data members have pointers to their connectivity element.
        Edge * edge;

    public:

        Edge_Data(Edge * edge)
        {
            this->edge = edge;
        }

        bool marked = false;
    };

    class Halfedge_Data
    {

    private:

        // Halfedge Data members have pointers to their connectivity element.
        Halfedge * halfedge;

    public:

        Halfedge_Data(Halfedge * halfedge)
        {
            this->halfedge = halfedge;
        }


        bool marked = false;
        Halfedge * next_extraordinary = NULL;

        // A Halfedge will be labeled as extraordinary iff its vertex is an intersection point or a tail_point.
        bool isExtraordinary()
        {
            return halfedge->vertex->data->isExtraordinary();
        }
    };
}