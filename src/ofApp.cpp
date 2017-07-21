#include "ofApp.h"
#include <assert.h>
#define RANGE 25.0
//--------------------------------------------------------------
float ofApp::gaze[2];
float ofApp::head[3];
void gaze_point_callback(tobii_gaze_point_t const* gaze_point, void* user_data)
{
	if (gaze_point->validity == TOBII_VALIDITY_VALID) {
		/*for (int i = 0; i < 2; i++)
		{
			ofApp::gaze[i] = ofApp::gaze[i] * 0.85 + (gaze_point->position_xy[i]) * 0.15;
		}*/
		int gazePixels[2];
		gazePixels[0] = gaze_point->position_xy[0] * ofGetScreenWidth();
		gazePixels[1] = gaze_point->position_xy[1] * ofGetScreenHeight();
		//if (gazePixels[0] > ofGetWindowPositionX() && gazePixels[0] < ofGetWindowPositionX() + ofGetWindowWidth())
			ofApp::gaze[0] = ofApp::gaze[0] * 0.85 + (gazePixels[0] - ofGetWindowPositionX()) * 0.15;
		//if (gazePixels[1] > ofGetWindowPositionY() && gazePixels[1] < ofGetWindowPositionY() + ofGetWindowHeight())
			ofApp::gaze[1] = ofApp::gaze[1] * 0.85 + (gazePixels[1] - ofGetWindowPositionY()) * 0.15;
	}

}
void head_pose_callback(tobii_head_pose_t const* head_pose, void* user_data)
{
	//for (int i = 0; i < 3; ++i)
	//	if (head_pose->rotation_validity_xyz[i] == TOBII_VALIDITY_VALID) {
	//		ofApp::head[i] = ofApp::head[i] * 0.85 + head_pose->rotation_xyz[i] * 0.15;
	//		//printf("%f ", head_pose->rotation_xyz[i]);
	//	}
	ofApp::head[0] = ofApp::head[0] * 0.85 + (0.2-head_pose->rotation_xyz[0]) * 3 * ofGetScreenHeight() * 0.15;
	ofApp::head[1] = ofApp::head[1] * 0.85 + (0.5 - head_pose->rotation_xyz[1]) * ofGetScreenWidth() * 0.15;
	ofApp::head[2] = ofApp::head[2] * 0.85 + (head_pose->rotation_xyz[2])* 0.15;
	//printf("\n");
	/*for (int i = 0; i < 3; ++i)
		if (head_pose->position_validity == TOBII_VALIDITY_VALID)
			ofApp::head[i] = head_pose->position_xyz[i];*/
}

void ofApp::exit() {
	error = tobii_gaze_point_unsubscribe(device);
	assert(error == TOBII_ERROR_NO_ERROR);
	error = tobii_head_pose_unsubscribe(device);
	assert(error == TOBII_ERROR_NO_ERROR);
	error = tobii_device_destroy(device);
	assert(error == TOBII_ERROR_NO_ERROR);
	error = tobii_api_destroy(api);
	assert(error == TOBII_ERROR_NO_ERROR);
}

void ofApp::setup(){
	printf("Set pitch input:\n");
	printf("1. Tobii 4c Head tracking\n2. Tobii 4c Eye Tracking\n3. Mouse Pointer\n");
	scanf("%d", &pitchInput);
	printf("Set volume input:\n");
	printf("1. Tobii 4c Head tracking\n2. Tobii 4c Eye Tracking\n3. Mouse Pointer\n");
	scanf("%d", &VolumeInput);
	printf("Set vibrato input:\n");
	printf("1. Tobii 4c Head tracking\n2. Tobii 4c Eye Tracking\n3. Mouse Pointer\n");
	scanf("%d", &vibratoInput);
	ofBackground(34, 34, 34);

	// 2 output channels,
	// 0 input channels
	// 22050 samples per second
	// 512 samples per buffer
	// 4 num buffers (latency)
	
	int bufferSize		= 256;
	sampleRate 			= 44100;
	phase 				= 0;
	vibPhase = 0.f;
	phaseAdder 			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	volume				= 0.1f;
	bNoise 				= false;
	vibPhaseAdder = ( 5.0f / (float)sampleRate) * TWO_PI;
	pan = 0.5f;
	lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);
	
	soundStream.printDeviceList();
	
	//if you want to set the device id to be different than the default
	//soundStream.setDeviceID(1); 	//note some devices are input only and some are output only 

	soundStream.setup(this, 2, 0, sampleRate, bufferSize, 4);

	// on OSX: if you want to use ofSoundPlayer together with ofSoundStream you need to synchronize buffersizes.
	// use ofFmodSetBuffersize(bufferSize) to set the buffersize in fmodx prior to loading a file.
	
	ofSetFrameRate(60);

	targetVolume = 0.8;
	
	vibWidth = 0.f;
	error = tobii_api_create(&api, NULL, NULL);
	assert(error == TOBII_ERROR_NO_ERROR);
	error = tobii_device_create(api, NULL, &device);
	assert(error == TOBII_ERROR_NO_ERROR);
	error = tobii_gaze_point_subscribe(device, gaze_point_callback, 0);
	assert(error == TOBII_ERROR_NO_ERROR);
	error = tobii_head_pose_subscribe(device, head_pose_callback, 0);
	assert(error == TOBII_ERROR_NO_ERROR);
}


//--------------------------------------------------------------
void ofApp::update(){
	error = tobii_wait_for_callbacks(device);
	assert(error == TOBII_ERROR_NO_ERROR || error == TOBII_ERROR_TIMED_OUT);
	error = tobii_process_callbacks(device);
	assert(error == TOBII_ERROR_NO_ERROR);
	switch (VolumeInput) {
	case 1:
		targetVolume = (ofGetWindowHeight() - head[0]) / ofGetWindowHeight();
		break;
	case 2:
		//targetVolume = ((1 - gaze[1]) * ofGetScreenHeight() - (ofGetScreenHeight() - (ofGetWindowPositionY() + ofGetWindowHeight()))) / ofGetWindowHeight();
		targetVolume = (ofGetWindowHeight() - gaze[1]) / ofGetWindowHeight();
		break;
	default:
		targetVolume = (float)(ofGetWindowHeight() - mouseY )/ (float)ofGetHeight();
	}
	if (targetVolume > 0.98)
		targetVolume = 0.98;
	if (targetVolume < 0.1)
		targetVolume = 0;

	float widthPct;
	switch (pitchInput) {
	case 1:
		widthPct = head[1] / ofGetWidth();;
		break;
	case 2:
		//widthPct = (gaze[0] * ofGetScreenWidth() - ofGetWindowPositionX()) / ofGetWidth();
		widthPct = gaze[0] / ofGetWidth();
		break;
	default:
		widthPct = (float)mouseX / (float)ofGetWidth();
	}
	float tempVibW;
	switch (vibratoInput) {
	case 1:
		tempVibW = (1.f - head[0]/ (float)ofGetHeight())*1.2f;;
		break;
	case 2:
		tempVibW = (1.f - gaze[1] / (float)ofGetHeight())*1.2f;
		break;
	default:
		tempVibW = (1.f - (float)mouseY / (float)ofGetHeight())*1.2f;
	}
	targetFrequency = 127.0958*pow(2, widthPct* RANGE / 12.f);//half semitone under DO_3
	phaseAdderTarget = (targetFrequency / (float)sampleRate) * TWO_PI;
	vibWidth = 0.95 * vibWidth + 0.05*tempVibW*pow(2, targetFrequency / 500.f);
}


//--------------------------------------------------------------
void ofApp::draw(){
	ofSetLineWidth(1);
	for (int i = 0; i < (int)RANGE; i++) {
		int semi = i % 12;
		if (semi == 0 || semi == 2 || semi == 4 || semi == 5 || semi == 7 || semi == 9 || semi == 11)
			ofSetColor(255);
		else
			ofSetColor(0);
		//ofRectangle(i*ofGetWidth() / RANGE, 0, ofGetWidth() / RANGE, ofGetHeight());
		ofFill();
		ofDrawRectangle(i*ofGetWidth() / RANGE, 0, ofGetWidth() / RANGE, ofGetHeight());
		ofSetColor(255, 0, 0);
		ofLine(i*ofGetWidth() / RANGE, 0, i*ofGetWidth() / RANGE, ofGetHeight());
	}

	ofSetColor(0,255,0);

	ofNoFill();

	ofSetLineWidth(1);
	ofSetColor(120, 120, 120);
	ofPoint pointer;
	switch (VolumeInput) {
	case 1:
		pointer.y = head[0];
		//ofLine(ofPoint(0, (0.5-head[0]) * ofGetScreenHeight() - ofGetWindowPositionY()), ofPoint(ofGetWidth(), (0.5 - head[0]) * ofGetScreenHeight() - ofGetWindowPositionY()));
		break;
	case 2:
		pointer.y = gaze[1];
		//ofLine(ofPoint(0, gaze[1] * ofGetScreenHeight() - ofGetWindowPositionY()), ofPoint(ofGetWidth(), gaze[1] * ofGetScreenHeight() - ofGetWindowPositionY()));
		break;
	default:
		pointer.y = mouseY;
	}
	switch (pitchInput) {
	case 1:
		pointer.x = head[1];
		//ofLine(ofPoint((0.5 - head[1]) * ofGetWidth(), 0), ofPoint((0.5 - head[1]) * ofGetWidth(), ofGetHeight()));
		break;
	case 2:
		pointer.x = gaze[0];
		//ofLine(ofPoint(gaze[0] * ofGetScreenWidth() - ofGetWindowPositionX(), 0), ofPoint(gaze[0] * ofGetScreenWidth() - ofGetWindowPositionX(), ofGetHeight()));
		break;
	default:
		pointer.x = mouseX;
	}
	//ofCircle(ofPoint(gaze[0]*ofGetScreenWidth()-ofGetWindowPositionX(), gaze[1]*ofGetScreenHeight()-ofGetWindowPositionY()), ofGetScreenHeight()*0.06);
	
	//ofLine(ofPoint(0, gaze[1] * ofGetScreenHeight() - ofGetWindowPositionY()), ofPoint(ofGetWidth(), gaze[1] * ofGetScreenHeight() - ofGetWindowPositionY()));
	//ofLine(ofPoint((0.5 - head[1]) * ofGetWidth(), 0), ofPoint((0.5 - head[1]) * ofGetWidth(), ofGetHeight()));
	//ofCircle(ofPoint((0.5-head[1]) * ofGetWidth(), (0.5-head[0]) * ofGetHeight()), head[2]*ofGetWidth());
	//ofDrawRectangle(0, myTobii.eventParams.Y*1.1, ofGetWidth(), myTobii.eventParams.Y*0.9);
	ofSetColor(120, 120, 120);
	for (int i = 0; i < (int)RANGE; i++) {
		ofFill();
		for (int y = 0; y < ofGetHeight(); y += ofGetScreenHeight()*0.05) {
			ofCircle(i * ofGetWidth() / RANGE, y, ofGetHeight()*0.002);
			ofCircle((i + 0.25)*ofGetWidth() / RANGE, y, ofGetHeight()*0.002);
			ofCircle((i + 0.5)*ofGetWidth() / RANGE, y , ofGetHeight()*0.004);
			ofCircle((i + 0.75)*ofGetWidth() / RANGE, y , ofGetHeight()*0.002);
		}
	}
	ofNoFill();
	ofCircle(pointer, 76);
	/*ofDrawBitmapString(ofGetFrameRate(), 10, 10);

	ofDrawBitmapString(vibWidth, 10, 60);*/
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	if (key == '-' || key == '_' ){
		volume -= 0.05;
		volume = MAX(volume, 0);
	} else if (key == '+' || key == '=' ){
		volume += 0.05;
		volume = MIN(volume, 1);
	}
	
	if( key == 's' ){
		soundStream.start();
	}
	
	if( key == 'e' ){
		soundStream.stop();
	}
	if (key == 'f') {
		ofToggleFullscreen();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	//float width = (float)ofGetWidth();
	//float widthPct = x / width * RANGE/12.f;
	//targetFrequency = 127.0958*pow(2,widthPct);//half semitone under DO_3
	//phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels){
	float leftScale = 1 - pan;
	float rightScale = pan;

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI) {
		phase -= TWO_PI;
	}
	while (vibPhase > TWO_PI) {
		vibPhase -= TWO_PI;
	}
	
	float cosPhase = cos(vibPhase)*vibWidth;

	phaseAdder = phaseAdderTarget;
	volume = 0.8*volume + 0.2*targetVolume;
	for (int i = 0; i < bufferSize; i++) {
		vibPhase += vibPhaseAdder;
		phase += phaseAdder;
		float sample = 0;
		for (int j = 1; j<=8; j+=1) {
			float amplitude = 1.f/(float)(j*j*j);
			sample += amplitude*sin(j *(phase + cos(vibPhase)*vibWidth));
		}
		lAudio[i] = output[i*nChannels] = sample * volume * leftScale;
		rAudio[i] = output[i*nChannels + 1] = sample * volume * rightScale;
	}


}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
