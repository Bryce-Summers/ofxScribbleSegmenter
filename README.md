# ofxScribbleSegmenter
A segmentation procedure for planar graphs defined by an input polyline.


#Input

A list of 2D points defining a polyline.

#Output:
A list of lists of points, where each list of point represents a polygon representing one face of the planar graph when the input polyline is embedded into it.

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

