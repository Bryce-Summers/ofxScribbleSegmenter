#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){


    /*
    points->push_back(ofPoint(0, 0));
    points->push_back(ofPoint(1, 0));
    points->push_back(ofPoint(1, 1));
    points->push_back(ofPoint(.5, 1));
    points->push_back(ofPoint(.5, -1));
    points->push_back(ofPoint(0, 1));
    */

    points.push_back(ofPoint(0, 100));
    points.push_back(ofPoint(200, 100));
    points.push_back(ofPoint(200, 200));
    points.push_back(ofPoint(100, 200));
    points.push_back(ofPoint(100, 0));
    points.push_back(ofPoint(300, 0));
    points.push_back(ofPoint(300, 150));
    points.push_back(ofPoint(0, 150));

    std::vector< std::vector<ofPoint> *> * faces;

    shapes = segmenter.FindFaces(&points);
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

    for(int i = 0; i < len; i++)
    {
        if(i != num)
        {
            continue;
        }

        std::vector<ofPoint> * points = shapes -> at(i);

        int len2 = points->size();

        ofPath p = ofPath();
        p.setStrokeColor(128);
        p.setStrokeWidth(1);

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

    ofPath p2 = ofPath();
    p2.setStrokeColor(128);
    p2.setStrokeWidth(1);

    len = points.size();
    p2.moveTo(points[0]);
    for(int i = 1; i < len; i++)
    {
        p2.lineTo(points[i]);
    }

    p2.setFilled(false);
    p2.draw();

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
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

    std::vector< std::vector<ofPoint> *> * shapes_new = segmenter.FindFaces(&points);

    mutex.lock();
    shapes = shapes_new;
    mutex.unlock();

    cout<< "Rebuilt Scribble" << endl;
    cout << shapes->size() << " Cycles!" << endl;

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
