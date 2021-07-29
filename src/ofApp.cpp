#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	
	
#if defined(USE_YOLO_9000)
	yolo.setup("9000/yolo9000.cfg","9000/yolo9000.weights","9000/9k.names");

	std::ifstream file(ofToDataPath("selected.names"));
	if (file.is_open()) {
		for (std::string line; file >> line;) selected.push_back(line);
		ofLog() << "Load seleced names:";
		for (auto &p : selected) ofLog() << p;
	}
#elif defined(USE_YOLO_3)

	//yolo.setup("yolo3/yolov3.cfg", "yolo3/yolov3.weights", "yolo3/coco.names");
	yolo.setup("yolo3/yolov3-tiny.cfg", "yolo3/yolov3-tiny.weights", "yolo3/coco.names");

#else
	yolo.setup();
#endif


	yolo.trackHistory = TRACKING_ID_HISTROY;


	useCamera = false;

	//ofLog() << "Open video...";
	video.load("face.mp4");
	video.setLoopState(ofLoopType::OF_LOOP_NORMAL);

	video.play();

	sender.setup(HOST, PORT);
	receiver.setup(PYTHON_PORT);


	spout_sender.init("Camera");
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

			spout_sender.send(camera.getTexture());
		
		}

	}
	else {
		
		video.update();
		
		if (video.isFrameNew()) {

			detections.clear();
			human_detections.clear();
			//detections = darknet.yolo(camera.getPixels(), thresh, maxOverlap);
			auto new_detections = yolo.detect(video.getPixels());

			
			for(auto &p : new_detections){
				//ofLog() << p.obj_id<< yolo.getName(p.obj_id);
				if (selected.size() < 1
					|| std::find(selected.begin(), selected.end(), yolo.getName(p.obj_id)) != selected.end()) {
					
					//ofLog() << p.obj_id;
					if (p.obj_id == HUMAN_OBJ_ID) human_detections.push_back(p);
					detections.push_back(p);
				}
			}

			spout_sender.send(video.getTexture());


		}
	}
	
	// calculate label count
	labelCount.clear();
	for (auto &dd : detections) {
		if (labelCount.find(dd.obj_id) != labelCount.end()) {
			labelCount[dd.obj_id]++;
		}else {
			labelCount[dd.obj_id] = 1;
		}
	}

	for (auto d : detections) {
		sendDetectOsc(d);
	}
	
	updateReceiver();


	ofSetWindowTitle(ofToString(ofGetFrameRate()));
}

//--------------------------------------------------------------
void ofApp::draw(){
	
	if(useCamera) camera.draw(0, 0);
	else video.draw(0, 0);

	ofPushStyle();
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
		ofDrawBitmapStringHighlight(ofToString(d.track_id)+"/"+ofToString(labelCount[d.obj_id]), d.x, d.y - 20);

	}

	for (auto d : face_detections)
	{
		
		ofSetColor(ofColor::red);
		ofNoFill();
		ofDrawRectangle(d.x, d.y, d.w, d.h);
		ofDrawBitmapStringHighlight(d.gender + " / " + ofToString(d.age)+" / " +d.emotion, d.x, d.y + 20);
		ofDrawBitmapStringHighlight(ofToString(d.track_id), d.x, d.y - 20);

	}

	ofPopStyle();
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

void ofApp::updateReceiver() {

	face_detections.clear();

	while(receiver.hasWaitingMessages()){

		ofxOscMessage m;
		receiver.getNextMessage(m);

		if(m.getAddress()=="/face"){

			fbox_t face;

			face.x = m.getArgAsFloat(0);
			face.y = m.getArgAsFloat(1);
			face.w = m.getArgAsFloat(2);
			face.h = m.getArgAsFloat(3);

			face.gender = m.getArgAsString(4);
			face.age = m.getArgAsFloat(5);

			face.emotion = m.getArgAsString(6);
			face.track_id = m.getArgAsInt32(7);

			face_detections.push_back(face);
		}

	}

	

	// remap id
	for(auto& face : face_detections){
		
		ofRectangle rect(face.x, face.y, face.w, face.h);
		
		//ofLog() << "current human= " << human_detections.size();

		for (auto& human : human_detections) {
			ofRectangle rect2(human.x, human.y, human.w, human.h);

			//ofLog() << "compare: " << rect << " => " << rect2;

			if (rect.intersects(rect2)) {
				ofLog() << "intersect!" <<face.track_id <<" => "<<human.track_id;

				face.track_id = human.track_id;
				break;
			}
			else {
				ofLog() << "no intersect!";
			}
		}

		sendFaceOsc(face);

	}

}

void ofApp::sendFaceOsc(fbox_t d) {
	ofxOscMessage m;
	m.setAddress("/face");

	m.addFloatArg(d.x);
	m.addFloatArg(d.y);
	m.addFloatArg(d.w);
	m.addFloatArg(d.h);
	
	m.addStringArg(d.gender);
	m.addFloatArg(d.age);
	m.addStringArg(d.emotion);

	m.addIntArg(d.track_id);

	sender.sendMessage(m, false);
}
void ofApp::sendDetectOsc(bbox_t d) {
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

	m.addIntArg(d.track_id);

	m.addIntArg(labelCount[d.obj_id]);

	sender.sendMessage(m, false);
}