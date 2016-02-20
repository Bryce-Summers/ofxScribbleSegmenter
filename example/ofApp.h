#pragma once

#include "ofMain.h"
#include "../include/FaceFinder.h"
#include "../include/OffsetCurves.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

    private:

        vector<ofPoint> points;
        vector<ofPoint> points_2;
        std::vector<int> external_face_indices;

        std::vector< std::vector<scrib::point_info> *> * shapes;


        scrib::FaceFinder segmenter_fast  = scrib::FaceFinder();
        scrib::FaceFinder segmenter_brute = scrib::FaceFinder(false);

        ofMutex mutex;

        int num;

        // Helpful path drawing function.
       void drawPath(vector<ofPoint> &points);
};
