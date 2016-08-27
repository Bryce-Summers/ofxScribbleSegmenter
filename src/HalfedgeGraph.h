#pragma once

#include "ofMain.h"

/*
* Halfedge Graph / Mesh class.
*
* Written by Bryce Summers.
*
* 8/17/2016: Finished purely connectivist version with association.
*
* Usage:
*
* The Application programmer / New Media Artist specifies the following:
* 1. The data format they have availible for graph construction (e.g. vectors of openframeworks points.)
* 2. The Algorithms they need.
* 3. The data format that they wish to receive the results in.
*
* The algorithm designer / computational geometer specifies the following:
* 1. Definitions for the associated data.
* 2. The interface for constructing Graphs from application data
* 3. The interface for running algorithms on the Graphs.
* 4. The interface for allowing the application programmer to retrieve the results.
*
* If done elegantly, the New Media Artist should never need to touch the halfedge mesh, go on pointer journeys, and
* they should be able to treat the internal implementation as a black box.
*/

namespace scrib
{
    // == Forward Declaration of classes.

    // -- Connectivity Elements.

    // Represents an entire planar graph embedding.
    class Graph;
    class Vertex;
    class Face;
    class Halfedge;
    class Edge;

    // Iterators for accessing the elements from the Graph object.
    // FIXME: I may wish to use lists instead of vectors if I ever wish to delete objects.
    // For now they are great because they allow random access to the elements by ID.
    typedef   vector<Vertex *>::iterator   Vertex_Iter;
    typedef   vector<Edge *>::iterator     Edge_Iter;
    typedef   vector<Face *>::iterator     Face_Iter;
    typedef   vector<Halfedge *>::iterator Halfedge_Iter;

    // -- Associated Data.
    // The classes will be defined in application specific files so that this halfedge mesh header file may be reused.
    class Graph_Data;
    class Vertex_Data;
    class Face_Data;
    class Halfedge_Data;
    class Edge_Data;

    // FIXME: Clean up this prose.

    // -- Structural definition of classes.
    // Every class is specified by its connectivity information and a pointer to associated user data.

    // All elements may be marked and unmarked by algorithms and users to specific sets of elements that meet various criteria.

    // The Graph class represents an entire graph embedding defined by points in space.
    // For the purposes of the facefinder, the output graph will be planar.
    // connected via edges that intersect only at vertices.
    // The FaceFinder class may be used to derive a Graph from a set of potentially intersecting input polylines.
    class Graph
    {

        // Graph classes are where all of the actual data will be stored, so it contains vectors of valued data,
        // rather than pointers.
        // All ID's contained within these vectors will reference tha index of the object within these vectors.

        // Ideally, vertices, edges, and halfedges will be ordered logically according to the order they were input into the facefinder,
        // but I will need to do some more thinking on how to formally specify these things.

    private:

        // Note: We use pointers, because then the location on the heap for the elements is permanant.
        std::vector<Face *>    faces;
        std::vector<Vertex *>  vertices;
        std::vector<Edge *>    edges;
        std::vector<Halfedge*> halfedges;

    public:

        // Extra Application specific information.
        Graph_Data * data;

        // -- Public Interface.

        // Allocation functions.
        Face     * newFace();
        Vertex   * newVertex();
        Edge     * newEdge();
        Halfedge * newHalfedge();

        // Accessing functions. We keep this interface, because then we only have to guranteed that the this.get(element.ID) = element.
        // We could even change the internal structure to a non contiguous lookup and the interface would be preserved.

        Face * getFace(int ID)
        {
            return faces[ID];
        }

        Vertex * getVertex(int ID)
        {
            return vertices[ID];
        }

        Edge * getEdge(int ID)
        {
            return edges[ID];
        }

        Halfedge * getHalfedge(int ID)
        {
            return halfedges[ID];
        }

        size_t numFaces()
        {
            return faces.size();
        }

        size_t numVertices()
        {
            return vertices.size();
        }

        size_t numEdges()
        {
            return edges.size();
        }

        // Should theoretically be numEdges * 2.
        size_t numHalfedges()
        {
            return halfedges.size();
        }

        // -- Iteration functions.

        Face_Iter facesBegin() { return faces.begin(); }
        Face_Iter facesEnd() { return faces.end(); }

        Vertex_Iter verticesBegin() { return vertices.begin(); }
        Vertex_Iter verticesEnd() { return vertices.end(); }

        Edge_Iter edgesBegin() { return edges.begin(); }
        Edge_Iter edgesEnd() { return edges.end(); }

        Halfedge_Iter halfedgesBegin() { return halfedges.begin(); }
        Halfedge_Iter halfedgesEnd()   { return halfedges.end();   }

    };

    class Face
    {
    public:

        // Representative from the interior loop of halfedges defining the boundary of the face.
        Halfedge * halfedge;

        Face_Data * data;
        int ID;
    };

    class Vertex
    {
    public:

        // A representative halfedge that is traveling away from this Vertex.
        // this -> halfedge -> vertex = this.
        Halfedge * halfedge;

        Vertex_Data * data;
        int ID;
    };

    // Non directed edges, very useful for getting consecutive ID's within input polylines.
    class Edge
    {
    public:

        Halfedge * halfedge;

        Edge_Data * data;
        int ID;
    };


    class Halfedge
    {
    public:

        Halfedge * twin;
        Halfedge * next;
        Halfedge * prev;

        Face * face;
        Edge * edge;
        Vertex * vertex;

        Halfedge_Data * data;
        int ID;
    };
}