# ofxScribbleSegmenter
A segmentation procedure for planar graphs defined by an input polyline.

![alt text](https://github.com/Bryce-Summers/ofxScribbleSegmenter/blob/master/Screenshots/Scribble.png "Scribble")

#Input:

A list of 2D points defining a polyline.

Alternativly you can use a list of polylines.

#Output:
A list of lists of points, where each list of point represents a polygon representing one face of the planar graph when the input polyline is embedded into it.

#External Faces:

Please note the some of the polygons in the output list will be external faces, in that they include the entire infinite cartesian plane minus the space encompassed by the union of the internal faces.

Another way of saying this is that if I were to give a square-ish polyline as an input the the program, the it would produce 2 faces, one that contains the indide of the square and one that contains the outside of the square.


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


#Example Output (One possible run)

```
setup done!
4 Cycles!
There is/are 1 external faces with 12,  points in each of them respectively.

 Information about all of the polygons:

Polygon 0
 --Point : 51.7093, 49.0416, 0, index = 11
 --Point : 51.7056, 48.465, 0, index = 4
 --Point : 51.7093, 49.0416, 0, index = 11
 --Point : 99.7357, 49.5628, 0, index = 13
 --Point : 99.0668, -0.946655, 0, index = 6
 --Point : 1.83067, 0.255829, 0, index = 10
 --Point : 1.7207, 100.376, 0, index = 8
 --Point : 52.033, 99.7853, 0, index = 12
 --Point : 51.7093, 49.0416, 0, index = 11
 --Point : 49.5549, 49.0182, 0, index = 0
Polygon 1
 --Point : 99.7357, 49.5628, 0, index = 13
 --Point : 100.393, 99.2178, 0, index = 7
 --Point : 52.033, 99.7853, 0, index = 12
 --Point : 52.3484, 149.233, 0, index = 3
 --Point : 149.137, 149.424, 0, index = 2
 --Point : 150.136, 50.1098, 0, index = 1
Polygon 2
 --Point : 149.137, 149.424, 0, index = 2
 --Point : 52.3484, 149.233, 0, index = 3
 --Point : 52.033, 99.7853, 0, index = 12
 --Point : 1.7207, 100.376, 0, index = 8
 --Point : 1.83067, 0.255829, 0, index = 10
 --Point : -0.580078, 0.285645, 0, index = 5
 --Point : 1.83067, 0.255829, 0, index = 10
 --Point : 1.84265, -10.6498, 0, index = 9
 --Point : 1.83067, 0.255829, 0, index = 10
 --Point : 99.0668, -0.946655, 0, index = 6
 --Point : 99.7357, 49.5628, 0, index = 13
 --Point : 150.136, 50.1098, 0, index = 1
Polygon 3
 --Point : 99.7357, 49.5628, 0, index = 13
 --Point : 51.7093, 49.0416, 0, index = 11
 --Point : 52.033, 99.7853, 0, index = 12
 --Point : 100.393, 99.2178, 0, index = 7

Process returned 0 (0x0)   execution time : 8.427 s
Press any key to continue.
```

