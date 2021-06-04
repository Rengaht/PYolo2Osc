#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	std::string cfgfile = ofToDataPath("cfg/yolo9000.cfg");
	std::string weightfile = ofToDataPath("yolo9000.weights");
	std::string namesfile = ofToDataPath("cfg/9k.names");

	//darknet.init(cfgfile, weightfile, namesfile);
	yolo.setup();


	useCamera = false;

	//ofLog() << "Open video...";
	video.load("test.mov");
	video.setLoopState(ofLoopType::OF_LOOP_NORMAL);

	video.play();

	sender.setup(HOST, PORT);

}

//--------------------------------------------------------------
void ofApp::update(){
	float thresh = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 1);

	// if a detected object overlaps >maxOverlap with another detected
	// object with a higher confidence, it gets omitted
	float maxOverlap = 0.25;

	ofSetColor(255);
	

	if (useCamera) {
		
		camera.update();

		if (camera.isFrameNew()) {
			detections.clear();
			//detections = darknet.yolo(camera.getPixels(), thresh, maxOverlap);
			detections = yolo.detect(camera.getPixels());
		}

	}
	else {
		
		video.update();
		
		if (video.isFrameNew()) {
			detections.clear();
			//detections = darknet.yolo(camera.getPixels(), thresh, maxOverlap);
			detections = yolo.detect(video.getPixels());
		}
	}

	for (auto d : detections) {
		ofxOscMessage m;
		m.setAddress("/detect");

		/*m.addStringArg(d.label);
		m.addFloatArg(d.rect.x);
		m.addFloatArg(d.rect.y);
		m.addFloatArg(d.rect.getWidth());
		m.addFloatArg(d.rect.getHeight());
		m.addFloatArg(d.probability);*/

		m.addStringArg(yolo.getName(d.obj_id));
		m.addFloatArg(d.x);
		m.addFloatArg(d.y);
		m.addFloatArg(d.w);
		m.addFloatArg(d.h);
		m.addFloatArg(d.prob);
		
		sender.sendMessage(m, false);
	}
	
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
}

//--------------------------------------------------------------
void ofApp::draw(){
	
	if(useCamera) camera.draw(0, 0);
	else video.draw(0, 0);


	ofNoFill();
	for (auto d : detections)
	{
		//ofSetColor(d.color);
		//glLineWidth(ofMap(d.probability, 0, 1, 0, 8));
		//ofNoFill();
		//ofDrawRectangle(d.rect);
		//ofDrawBitmapStringHighlight(d.label + ": " + ofToString(d.probability), d.rect.x, d.rect.y + 20);

		//// optionally, you can grab the 1024-length feature vector associated
		//// with each detected object
		//vector<float> & features = d.features;


		ofSetColor(ofColor::fromHsb(ofMap(d.obj_id,0,20,0,255),255,255));
		glLineWidth(ofMap(d.prob, 0, 1, 0, 8));
		ofNoFill();
		ofDrawRectangle(d.x,d.y,d.w,d.h);
		ofDrawBitmapStringHighlight(yolo.getName(d.obj_id) + ": " + ofToString(d.prob), d.x, d.y + 20);


	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
		
	if (key == 'q' || key == 'Q') {
		useCamera = !useCamera;

		if (useCamera) {
			if (setupCamera()) {
				video.stop();
				return;
			}
			else {
				ofLog() << "Fail to open camera (0)";
				useCamera = false;
			}
		}
			if (camera.isInitialized()) camera.close();
			video.play();
		
	}
}


void ofApp::mouseReleased(int x, int y, int button){

}

bool ofApp::setupCamera() {

	ofLog() << "Setup Camera...";
	
	camera.setDeviceID(0);
	camera.setDesiredFrameRate(30);
	camera.initGrabber(VideoWidth, VideoHeight);

	return camera.isInitialized();
	
}