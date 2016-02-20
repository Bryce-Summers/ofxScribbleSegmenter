#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    num = 0;

    // These lists of points represent square-ish shapes.
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

    std::vector< std::vector<ofPoint> *> * faces;


    // -- Use these function calls to compute the faces for
    //    either of the polylines by themeselves.
    //shapes = segmenter_fast.FindFaces(&points);
    //shapes = segmenter_brute.FindFaces(&points);

    segmenter_fast.setClosed(true);

    // -- Use these function calls to compute the faces when the
    //    plane is segmented by both polylines.
    shapes = segmenter_fast.FindFaces(&input);
    //shapes = segmenter_brute.FindFaces(&input);

    external_face_indices.clear();
    segmenter_fast.determineExternalFaces(shapes, &external_face_indices);


    cout << "setup done!" << endl;
    cout << shapes->size() << " Cycles!" << endl;
    cout << "There is/are " << external_face_indices.size() << " external faces with ";

    int len = external_face_indices.size();
    for(int i = 0; i < len; i++)
    {
        int shapes_index = external_face_indices[i];
        int polygon_size = shapes -> at(shapes_index) -> size();
        cout << polygon_size << ", ";
    }

    cout << " points in each of them respectively." << endl;

    cout << "\n Information about all of the polygons: \n" << endl;

    len = shapes -> size();
    for(int i = 0; i < len; i++)
    {
        std::vector<scrib::point_info> * polygon = shapes->at(i);
        int polygon_len = polygon -> size();

        cout << "Polygon " << i << endl;

        for(int j = 0; j < polygon_len; j++)
        {
            scrib::point_info p_info = polygon -> at(j);
            cout << " --Point : " << p_info.point <<
                    ", index = " << p_info.ID << endl;
        }
    }
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

    if(shapes == NULL)
    {
        return;
    }

    //ofFill();
    ofSetColor(ofColor::red);

    mutex.lock();

    // -- Draw a particular region.
    int len = shapes -> size();

    int i = num;
    for(int i = 0; i < len; i++)
    //if(i >= 0 && i < len)
    {
        /*
        if(i != num)
        {
            continue;
        }
        */

        std::vector<scrib::point_info> * points = shapes -> at(i);

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

        p.setFilled(false);
        p.close();
        p.draw();
    }

    mutex.unlock();

    // -- Draw the entire scribble.

    drawPath(points);
    drawPath(points_2);

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

    if(key == 'd')
    {
        // Increment mod len.
        int len = shapes->size();
        num = (num + 1) % len;
    }

    if(key == 'a')
    {
        // Decrement mod len.
        int len = shapes->size();
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

    points.push_back(ofPoint(x, y));
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

    num = -1;
    points.clear();
    points_2.clear();
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

    std::vector< std::vector<scrib::point_info> *> * shapes_new  = segmenter_fast.FindFaces(&points);
    std::vector< std::vector<scrib::point_info> *> * shapes_new2 = segmenter_brute.FindFaces(&points);

    scrib::OffsetCurves offsetter;
    shapes_new -> clear();

    for(int i = 20; i < 300; i += 20)
    {
        shapes_new -> push_back(offsetter.computeOffsetCurve(&points, i));
    }


    mutex.lock();
    shapes = shapes_new;
    mutex.unlock();

    cout<< "Rebuilt Scribble" << endl;
    cout << shapes_new2->size() << " Brute Cycles!" << endl;
    cout << shapes -> size() << " Fast Algo Cycles!" << endl;
    cout << "Size = " << points.size() << endl;

    num = 0;
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
