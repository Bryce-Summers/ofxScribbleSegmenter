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
			this->graph = graph;
		}

		// This may be useful to legacy code and people who want a simpler more genaric c++ output format.
		std::vector<std::vector<point_info>> faces;
	};

	class Face_Data
	{
	private:
		Face * face;

	public:

		Face_Data(Face * face)
		{
			this->face = face;
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
			this -> vertex = vertex;
		}

		ofPoint point;
		bool marked = false;

		bool tail_point = false;

		// Labels Vertices that have more than two outgoing edges.
		bool intersection_point = false;

		bool isExtraordinary()
		{
			return tail_point || intersection_point;
		}

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
			this -> halfedge = halfedge;
		}


		bool marked = false;
		Halfedge * next_extraordinary = NULL;

		// A Halfedge will be labeled as extraordinary iff its vertex is an intersection point or a tail_point.
		bool isExtraordinary()
		{
			return halfedge -> vertex -> data -> isExtraordinary();
		}
	};

	class point_info
	{
	public:

		// -- Constructor.
		point_info(ofPoint p, int id)
		{
			point = p;
			ID = id;
		}

		ofPoint point;
		int ID;
	};
}