#ifndef OFFSETCURVES_H
#define OFFSETCURVES_H

#include <vector>
#include "ofMain.h"
#include "Line.h"
#include "Intersector.h"
#include "FaceFinder.h"


/*
* Offset Curve computer.
* Written by Bryce Summers on 2/19/2016.
* Purpose: For a given *closed* curve,
*
* this computes the curve defined by all points at a signed distance d away form the curve.
*
* Features: A solution to the problem through a reduction to the external face of the planar graph segmentation.
*/

namespace scrib {

    class OffsetCurves
    {
    public:
        OffsetCurves();
        virtual ~OffsetCurves();

        std::vector<scrib::point_info> * computeOffsetCurve(std::vector<ofPoint> * inputs, double dist);

        std::vector<ofPoint> * offsetCurve(std::vector<ofPoint> * inputs, double dist);

        // Takes a closed loop and returns the closed loop cooresponding to its exterior.
        std::vector<scrib::point_info> * computeExterior(std::vector<ofPoint> * inputs);

    };

}

#endif // OFFSETCURVES_H
