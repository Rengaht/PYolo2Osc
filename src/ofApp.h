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
//#define USE_YOLO_9000
//#define USE_YOLO_3

#define HUMAN_OBJ_ID 14
#define PYTHON_PORT 5566

struct fbox_t {
	unsigned int x, y, w, h;	// (x,y) - top-left corner, (w, h) - width & height of bounded box
	//float prob;					// confidence - probability that the object was found correctly
	unsigned int track_id;		// tracking id for video (0 - untracked, 1 - inf - tracked object)
	float age;
	string gender;
	string emotion;
};

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
		ofxOscReceiver receiver;

		vector<string> selected;


		ofxSpout::Sender spout_sender;

		std::vector<bbox_t> human_detections;
		vector<fbox_t> face_detections;
		
	private:
		bool setupCamera();
		
		void updateReceiver();
		void sendFaceOsc(fbox_t face);
		void sendDetectOsc(bbox_t detect);

};
