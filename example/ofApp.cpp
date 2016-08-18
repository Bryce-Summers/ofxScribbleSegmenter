#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    num = 0;

    bool display_input_polyline = true;

    // These initial example lists of points represent square-ish shapes.
    /*  |
     * -+-----.
     *  |     |
     *  .-----.
     */

    // Compute an example involving 2 polylines.
    points.push_back(ofPoint(50, 50));//0
    points.push_back(ofPoint(150, 50));//0
    points.push_back(ofPoint(150, 150));//2
    points.push_back(ofPoint(52, 150));//3
    points.push_back(ofPoint(52, 48));//3

    int offset = 0;  // Intersection, one external face with 12 points.
    //int offset = 200; // No intersection, 2 external faces with 8 points each.

    points_2.push_back(ofPoint(offset + 0, 0));//0
    points_2.push_back(ofPoint(offset + 100, 0));//0
    points_2.push_back(ofPoint(offset + 100, 100));//2
    points_2.push_back(ofPoint(offset + 2, 100));//3
    points_2.push_back(ofPoint(offset + 2, -10));//3

    std::vector< std::vector<ofPoint> *> input;

    input.push_back(&points);
    input.push_back(&points_2);

	// Compute the initial graph embedding.
	computeEmbedding();

    external_face_indices.clear();

	post_processor.load_face_vector(faces);
    post_processor.determineExternalFaces(&external_face_indices);


    cout << "setup done!" << endl;
    cout << faces->size() << " Cycles!" << endl;
    cout << "There is/are " << external_face_indices.size() << " external faces with ";

    int len = external_face_indices.size();
    for(int i = 0; i < len; i++)
    {
        int shapes_index = external_face_indices[i];
        int polygon_size = faces -> at(shapes_index) -> size();
        cout << polygon_size << ", ";
    }

    cout << " points in each of them respectively." << endl;

    cout << "\n Information about all of the polygons: \n" << endl;

    len = faces -> size();
    for(int i = 0; i < len; i++)
    {
        std::vector<scrib::point_info> * polygon = faces->at(i);
        int polygon_len = polygon -> size();

        cout << "Polygon " << i << endl;

        for(int j = 0; j < polygon_len; j++)
        {
            scrib::point_info p_info = polygon -> at(j);
            cout << " --Point : " << p_info.point <<
                    ", index = "  << p_info.ID << endl;
        }
    }
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

    if(faces == NULL)
    {
        return;
    }

    ofSetColor(ofColor::red);
    ofFill();

    // Draw all of the faces.
    // Draw the face with index 'num' as filled.
    int len = faces -> size();

    for(int i = 0; i < len; i++)
    {

        std::vector<scrib::point_info> * points = faces -> at(i);

        // If the face is of trivial size.
        if (points -> size() < 1)
        {
            continue;
        }

        int len2 = points->size();

        ofPath p = ofPath();
        p.setStrokeColor(128);
        p.setStrokeWidth(1);

        ofPoint pt = points -> at(0).point;
        p.moveTo(pt.x, pt.y);

        for(int i2 = 1; i2 < len2; i2++)
        {
            ofPoint pt = points -> at(i2).point;
            p.lineTo(pt.x, pt.y);
        }

        p.setFilled(i == num);
        p.close();
        p.draw();
    }

    // -- Draw the entire scribble.

    if(display_input_polyline)
    {
        drawPath(points);
    }

    //drawPath(points_2);


    ofDrawBitmapString("Press 'A' and 'D' to cycle left and right through the faces.", 20, 170);
    ofDrawBitmapString("Click and drag the mouse in a wild pattern, then release to test new scribbles!", 20, 200);
}

void ofApp::drawPath(vector<ofPoint> &points)
{
    // Points 2.
    ofPath p3 = ofPath();
    p3.setStrokeColor(128);
    p3.setStrokeWidth(1);

    int len = points.size();

	if (len == 0)
	{
		return;
	}

    p3.moveTo(points[0]);
    for(int i = 1; i < len; i++)
    {
        p3.lineTo(points[i]);
    }

    p3.setFilled(false);
    p3.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if(faces->size() < 1)
    {
        return;
    }

    if(key == 'd')
    {
        // Increment mod len.
        int len = faces->size();
        num = (num + 1) % len;
    }

    if(key == 'a')
    {
        // Decrement mod len.
        int len = faces->size();
        num = (num + len - 1) % len;
    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

    last_point;

    ofPoint point_new = ofPoint(x, y);

    if(points.size() == 0 || last_point.distance(point_new) > 10.0)
    {
        points.push_back(point_new);
        last_point = point_new;
    }

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

    num = -1;
    points.clear();
    points_2.clear();
    display_input_polyline = true;
}

//--------------------------------------------------------------

void ofApp::mouseReleased(int x, int y, int button)
{
	computeEmbedding();
	num = 0;
	display_input_polyline = false;
}

void ofApp::computeEmbedding()
{
	// Raw face_vector embedding.
	// Allocated and deallocated withing this method.
	scrib::Face_Vector_Format * face_vector;

	switch (use_embedder)
	{
		case FACE_FINDER:
			face_vector = processUsingFaceFinder();
			return;
		case POLYLINE_GRAPH_EMBEDDER:
			face_vector = processUsingGraphEmbedder();
	}

	post_processor.load_face_vector(face_vector);
	this -> faces = post_processor.clipTails();
	delete face_vector;

	cout << faces -> size()      << " faces found." << endl;
	cout << "Number of Points = " << points.size()   << endl;

}

// Derives a face vector using the FaceFinder.
scrib::Face_Vector_Format * ofApp::processUsingFaceFinder()
{
	// The fast solver is pretty fast...
	scrib::Face_Vector_Format * face_vector = segmenter_fast.FindFaces(&points);

	cout << "Rebuilt Scribble Using Face Finder" << endl;
	return face_vector;
}

scrib::Face_Vector_Format * ofApp::processUsingGraphEmbedder()
{
	scrib::Graph * graph      = polyline_embedder.embedPolyline(&points);
	post_processor.load_graph(graph);
	scrib::Face_Vector_Format * face_vector = post_processor.convert_to_face_vectors();
	
	cout << "Rebuilt Scribble Using PolylineGraphEmbedder" << endl;

	return face_vector;
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
