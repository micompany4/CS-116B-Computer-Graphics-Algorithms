//--------------------------------------------------------------
//  Mandelbrot example -
//    source for mathematics - wikipedia
//      https://en.wikipedia.org/wiki/Mandelbrot_set
//
//  Kevin Smith  = 3/9/2020
//

#include "ofApp.h"

#define MAX_ITERATIONS 25
#define BOUNDS_VALUE 10


void ofApp::setup(){

	//  write mandelbrot to an image
	//
	image.allocate(1000, 1000, OF_IMAGE_COLOR);

	for (int y = 0; y < image.getHeight(); y++) {
		for (int x = 0; x < image.getWidth(); x++) {

			// set pixels here (using mandlebrot)

			// normalize pixel coordinate values to something in -1.5 to +1.5
			// range
			//
			float a0 = ofMap(x, 0, image.getWidth(), -1.5, 1.5);
			float b0 = ofMap(y, 0, image.getHeight(), -1.5, 1.5);

			//  complex number has real and imaginary parts
			//  use  a + bi
			//
			float a = a0;
			float b = b0;
	
			int i = 0;

			for (; i < MAX_ITERATIONS; i++) {
				float r = a * a - b * b + a0;
				float im = 2 * a * b + b0;

				if ((r*r + im*im) > BOUNDS_VALUE)
					break;

				a = r;
				b = im;
			}
			if (i < MAX_ITERATIONS) {
				image.setColor(x, y, ofColor::black);
			}
			else if (i == MAX_ITERATIONS) {
				image.setColor(x, y, ofColor::indigo);
			}
			else  {
				image.setColor(x, y, ofColor::yellow);
			}
		}



	}
	image.save("mandelbrot.jpg");

	// hack - I couldn't get image to draw from memory without saving it out 
	//        to an image on disk first.
	//
	im.loadImage("mandelbrot.jpg");
}


//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSetColor(255, 255, 255);
	im.draw(10, 10);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

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
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
