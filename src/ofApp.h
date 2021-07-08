#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
//#include "ofxDarknet.h"
#include "ofxYolo2.h"
#include "ofxSpout.h"

#define HOST "localhost"
#define PORT 5555


#define VideoWidth 640
#define VideoHeight 480


#define TRACKING_ID_HISTROY 30
#define USE_YOLO_9000
//#define USE_YOLO_3


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void mouseReleased(int x, int y, int button);
		

		ofVideoGrabber camera;
		
		ofVideoPlayer video;


		bool useCamera;
		//ofxDarknet darknet;
		//std::vector< detected_object > detections;

		ofxYolo2 yolo;
		std::vector<bbox_t> detections;
		map<int, int> labelCount;

		ofxOscSender sender;

		vector<string> selected;


		ofxSpout::Sender spout_sender;
		
	private:
		bool setupCamera();
};
