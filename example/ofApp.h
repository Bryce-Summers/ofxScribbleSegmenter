#pragma once

#include "ofMain.h"
#include "PolylineGraphMain.h"

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

        ofPoint last_point;

        bool display_input_polyline;

        std::vector< std::vector<scrib::point_info> *> * faces;


        scrib::FaceFinder segmenter_fast  = scrib::FaceFinder();
        scrib::FaceFinder segmenter_brute = scrib::FaceFinder(false);


		scrib::PolylineGraphEmbedder polyline_embedder;
		scrib::PolylineGraphPostProcessor post_processor = scrib::PolylineGraphPostProcessor(NULL);


		#define FACE_FINDER 0
		#define POLYLINE_GRAPH_EMBEDDER 1

		const int use_embedder = POLYLINE_GRAPH_EMBEDDER;

		// Computes an embedding using the desired embedder.
		// Converts this -> points to this->shapes.
		// Chooses an embedder based off of the value of this->use_embedder
		void computeEmbedding();
		scrib::Face_Vector_Format * processUsingFaceFinder();
		scrib::Face_Vector_Format * processUsingGraphEmbedder();


        int num;

        // Helpful path drawing function.
       void drawPath(vector<ofPoint> &points);
};
