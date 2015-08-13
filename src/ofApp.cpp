#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    num = 0;


    points.push_back(ofPoint(50, 50));//0
    points.push_back(ofPoint(150, 50));//0
    points.push_back(ofPoint(150, 150));//2
    points.push_back(ofPoint(52, 150));//3
    points.push_back(ofPoint(52, 48));//3

    points_2.push_back(ofPoint(0, 0));//0
    points_2.push_back(ofPoint(100, 0));//0
    points_2.push_back(ofPoint(100, 100));//2
    points_2.push_back(ofPoint(2, 100));//3
    points_2.push_back(ofPoint(2, -10));//3

    std::vector< std::vector<ofPoint> *> input;

    input.push_back(&points);
    input.push_back(&points_2);

    std::vector< std::vector<ofPoint> *> * faces;


    //shapes = segmenter_bentley.FindFaces(&points);
    //shapes = segmenter_brute.FindFaces(&points);

    // Test multiple polyline input.
    //shapes = segmenter_bentley.FindFaces(&input);
    shapes = segmenter_brute.FindFaces(&input);

    cout << "setup done!" << endl;
    cout << shapes->size() << " Cycles!" << endl;

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

    ofFill();
    ofSetColor(ofColor::red);

    mutex.lock();

    // -- Draw a particular region.
    int len = shapes -> size();


    int i = num;
    //for(int i = 0; i < len; i++)
    if(i >= 0 && i < len)
    {
        /*
        if(i != num)
        {
            continue;
        }
        */

        std::vector<ofPoint> * points = shapes -> at(i);

        int len2 = points->size();

        ofPath p = ofPath();
        p.setStrokeColor(128);
        p.setStrokeWidth(5);

        ofPoint pt = points -> at(0);
        p.moveTo(pt.x, pt.y);

        for(int i2 = 1; i2 < len2; i2++)
        {
            ofPoint pt = points -> at(i2);
            p.lineTo(pt.x, pt.y);
        }

        p.close();
        p.draw();
    }

    mutex.unlock();

    // -- Draw the entire scribble.

    drawPath(points);
    drawPath(points_2);

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
        num++;
        int len = shapes->size();
        num = min(len - 1, num);
    }

    if(key == 'a')
    {
        num--;
        num = max(num, 0);
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

    std::vector< std::vector<ofPoint> *> * shapes_new  = segmenter_bentley.FindFaces(&points);
    std::vector< std::vector<ofPoint> *> * shapes_new2 = segmenter_brute.FindFaces(&points);

    mutex.lock();
    shapes = shapes_new;
    mutex.unlock();

    cout<< "Rebuilt Scribble" << endl;
    cout << shapes_new2->size() << " Brute Cycles!" << endl;
    cout << shapes->size() << " Fast Algo Cycles!" << endl;
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
