#pragma once

#include "ofMain.h"
#include "HalfedgeGraph.h"
#include "PolylineGraphData.h"

/* The PolylineGraphPostProcessor class.
 *
 * Written and adapted from FaceFinder on 8/18/2016.
 *
 * Purpose:
 *
 * Allows users to convert Planar Polyline Embedded Graphs into mainstream C++ data structures.
 * This class then provides some useful processing algorithms on these output structures.
 */

namespace scrib
{

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

	// Convenience Typedefs.
	// Same old fashion mainstream c++ vectors, just an alis name.
	typedef std::vector<point_info> Point_Vector_Format;
	typedef std::vector<ofPoint> OF_Point_Vector_Format;
	typedef std::vector<Point_Vector_Format *> Face_Vector_Format;


	/*
	* http://math.blogoverflow.com/2014/06/04/greens-theorem-and-area-of-polygons/
	* Computes the area of a 2D polygon directly from the polygon's coordinates.
	* The area will be positive or negative depending on the
	* clockwise / counter clockwise orientation of the points.
	* Also see: https://brycesummers.wordpress.com/2015/08/24/a-proof-of-simple-polygonal-area-via-greens-theorem/
	*/
	float computeAreaOfPolygon(Point_Vector_Format * closed_polygon);

	/*
	* http://math.blogoverflow.com/2014/06/04/greens-theorem-and-area-of-polygons/
	* Computes the area of a 2D polygon directly from the polygon's coordinates.
	* The area will be positive or negative depending on the
	* clockwise / counter clockwise orientation of the points.
	* Also see: https://brycesummers.wordpress.com/2015/08/24/a-proof-of-simple-polygonal-area-via-greens-theorem/
	*/
	float computeAreaOfPolygon(OF_Point_Vector_Format * closed_polygon);


	class PolylineGraphPostProcessor
	{

	public:
		PolylineGraphPostProcessor(Graph * graph)
		{
			format_graph = graph;
		}

		virtual ~PolylineGraphPostProcessor() {}

	private:
		// Data.
		Graph * format_graph;

		// Face point vector format. // FIXME: Consider using smart pointers.
		Face_Vector_Format * format_face_vectors;

	public:

		// -- Data Structure Conversion.
		Face_Vector_Format * convert_to_face_vectors();

		// -- Post processing algorithms.


		// Appends the indices of any external faces amongst the input list of faces to the output vector.
		// NOTE : The input type is equivelant to the output type of the face finding functions,
		// so using this function may be a natural extension of using the original functions.
		void determineExternalFaces(std::vector<int> * output);

		// Appends to output the indices of the faces of **NonTrivial** Area (area >= min_area)
		void determineNonTrivialAreaFaces(std::vector<int> * output, float min_area);
		// Appends to output the indices of the faces of **Trivial** Area (area < min_area)
		void determineTrivialAreaFaces(std::vector<int> * output, float min_area);

		// Input: a set of faces, Output: a new set of faces that have no trivial contiguous subfaces.
		// ENSURES: Polygons will be output either open or closed in the manner that they are passed in.
		// ENSURES: Omits faces consisting of only a single long tail.
		// The user is still responsible to deallocating the original vector.
		Face_Vector_Format * clipTails();

		// Returns a copy of the single input face without any trivial area contiguous subfaces. (Tails)
		// May return a 0 point polyline if the input line is non-intersecting.
		std::vector<point_info> * clipTails(Point_Vector_Format * input);

	};
}