#pragma once

#include "ofMain.h"
#include "PolylineGraphMain.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

        void resetState();

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

        scrib::Face_Vector_Format * faces;


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

        // Stuff for face merging.
        int merge_ID_1;
        int merge_ID_2;
        int merge_ID_3;
        std::vector<scrib::face_info *> * merge_faces;

        void updateMergeFaces();
        void   drawMergeFaces();

        std::vector<int> external_faces;

        // Helpful path drawing function.
        void drawPath(vector<ofPoint> &points);
        void drawPath(scrib::Point_Vector_Format * points, int color, float strokeWidth, bool filled);
};
