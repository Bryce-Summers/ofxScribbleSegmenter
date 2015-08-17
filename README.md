# ofxScribbleSegmenter
A segmentation procedure for planar graphs defined by an input polyline.


#Input:

A list of 2D points defining a polyline.

Alternativly you can use a list of polylines.

#Output:
A list of lists of points, where each list of point represents a polygon representing one face of the planar graph when the input polyline is embedded into it.

#External Faces:

Please note the some of the polygons in the output list will be external faces, in that they include the entire infinite cartesian plane minus the space encompassed by the union of the internal faces.

Another way of saying this is that if I were to give a square-ish polyline as an input the the program, the it would produce 2 faces, one that contains the indide of the square and one that contains the outside of the square.

![alt text](https://github.com/Bryce-Summers/ofxScribbleSegmenter/blob/master/Screenshots/Scribble.png "Scribble")


Example Screen Shot
-

Please Note that the exact number may be slightly off due to randomization.

![alt text](https://github.com/Bryce-Summers/ofxScribbleSegmenter/blob/master/Screenshots/ExampleSchot-8-1-2015.png "Example and Information Output")

Usage
-

```
#include "ofMain.h"
#include "FaceFinder.h"

...

std::vector<ofPoint> input; // The input is a polyline, *not* a polygon.

<Populate Input here.>

FaceFinder segmenter;

// Data Structures for computed information.
std::vector< std::vector<scrib::point_info> *> * polygonal_output;
std::vector<int> external_face_indices;

// Using the FaceFinder to derive polygonal face infomation.
polygonal_output = segmenter.findFaces(&input);
// Appends the indices of every external face to the external_face_indices vector.
segmenter.determineExternalFaces(polygonal_output, &external_face_indices);

```

It is also possible to derive faces for a list of polyline inputs.

```
std::vector<std::vector<ofPoint> *> input;

<Same code as in singular polyline case from her on out.>

// - Function is overridden by a function of the same name that takes in a
//   vector<vector<ofPoint *> >; instead of a vector<ofPoint>.
polygonal_output = segmenter.findFaces(&input);

```

