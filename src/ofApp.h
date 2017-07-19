#pragma once

#include "ofMain.h"
#include "tobii.h"
#include <tobii_streams.h>

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		void audioOut(float * input, int bufferSize, int nChannels);

		ofSoundStream soundStream;

		float 	pan;
		int		sampleRate;
		bool 	bNoise;
		float 	volume;
		float targetVolume;

		vector <float> lAudio;
		vector <float> rAudio;
		
		//------------------- for the simple sine wave synthesis
		float 	targetFrequency;
		float 	phase;
		float 	phaseAdder;
		float 	phaseAdderTarget;
		tobii_error_t error;
		tobii_device_t* device;
		tobii_api_t* api;
		static float gaze[2];
		static float head[3];
};
