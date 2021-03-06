#include "testApp.h"

void testApp::setup(){
    
    // SETUP VIDEO INPUT
    camWidth = CAM_WIDTH;
    camHeight = CAM_HEIGHT;
	vidGrabber.setVerbose(true);
	//vidGrabber.listDevices();
	vidGrabber.initGrabber(camWidth,camHeight);
    
    //vidGrabber.videoSettings();
    
    // SETUP FACE DETECTION
	img.loadImage("test02.jpg");
	finder.setup("haarcascade_frontalface_default.xml");
	finder.findHaarObjects(img);
    
	mask.loadImage("mask.png");
    mask.resize(PCA_WIDTH, PCA_HEIGHT);
    mask_pixels = mask.getPixels();
    
    // SETUP FACE RECOGNITION
    rec.learn();
    gray.allocate(PCA_WIDTH, PCA_HEIGHT);
    color.allocate(PCA_WIDTH, PCA_HEIGHT);
    
    // PRECALCULATE TRANSLUCENT FACE SPRITES
    calcFaceSprites();
    
    showEigens=false;
    showFaces=false;
    showTest = false;
    showLeastSq=false;
    showClock=false;
    bgSubtract=false;
    
    ofBackground(0,0,0);
    ofEnableAlphaBlending();
    //    ofHideCursor();
    
    counter = 0;
    confidence = 0;
    bPersonFound = false;
    minhavar = 0;
    
    bShowVideo = false;

}
//------------------------------------------------------------------------
void testApp::calcFaceSprites() {
    if(!rec.isTrained()) return;
    
    for (int i=0; i<rec.numPeople(); i++) {
        ofImage masked;
        
        unsigned char* pixels = rec.getPersonPixels(i);
        unsigned char* rgba_pixels = new unsigned char[4*PCA_WIDTH*PCA_HEIGHT];
        for(int x=0; x<PCA_WIDTH; x++)
            for(int y=0; y<PCA_HEIGHT; y++) {
                rgba_pixels[(x+(y*PCA_WIDTH))*4] = pixels[(x+(y*PCA_WIDTH))*3];
                rgba_pixels[(x+(y*PCA_WIDTH))*4+1] = pixels[(x+(y*PCA_WIDTH))*3+1];
                rgba_pixels[(x+(y*PCA_WIDTH))*4+2] = pixels[(x+(y*PCA_WIDTH))*3+2];
                rgba_pixels[(x+(y*PCA_WIDTH))*4+3] = mask_pixels[x+y*PCA_WIDTH];
            }
        masked.setFromPixels(rgba_pixels, PCA_WIDTH, PCA_HEIGHT, OF_IMAGE_COLOR_ALPHA);
        faces.push_back(masked);
        
        delete rgba_pixels;

    };
}

//------------------------------------------------------------------------
void testApp::update(){
    
    counter++;
    
    lastName = person;
    if(bPersonFound == false) minhavar = person;
    
    vidGrabber.update();
    
	if (vidGrabber.isFrameNew()){
		unsigned char * pixels = vidGrabber.getPixels();
		videoTexture.loadData(pixels, camWidth, camHeight, OF_IMAGE_COLOR);
		if(bgSubtract) {
		    for(int x=0; x<camWidth; x++)
                for(int y=0; y<camHeight; y++)
                    if ((((pixels[(x+y*camWidth)*3] - bgImage.getPixels()[(x+y*camWidth)*3])*
                          (pixels[(x+y*camWidth)*3] - bgImage.getPixels()[(x+y*camWidth)*3])) +
                         ((pixels[(x+y*camWidth)*3+1] - bgImage.getPixels()[(x+y*camWidth)*3+1])*
                          (pixels[(x+y*camWidth)*3+1] - bgImage.getPixels()[(x+y*camWidth)*3+1]))+
                         ((pixels[(x+y*camWidth)*3+2] - bgImage.getPixels()[(x+y*camWidth)*3+2])*
                          (pixels[(x+y*camWidth)*3+2] - bgImage.getPixels()[(x+y*camWidth)*3+2]))) < 100.0) {
                             pixels[(x+y*camWidth)*3]=0;
                             pixels[(x+y*camWidth)*3+1]=0;
                             pixels[(x+y*camWidth)*3+2]=0;
                         }
		}
		img.setFromPixels(pixels, camWidth, camHeight, OF_IMAGE_COLOR, true);
		test_image.setFromPixels(pixels, camWidth, camHeight, OF_IMAGE_COLOR);
        test_image.resize(camWidth/TEST_DIV, camHeight/TEST_DIV);
        test_image.update();
        finder.findHaarObjects(test_image);
        //        finder.findHaarObjects(img);
	}
    

    
}

//------------------------------------------------------------------------
void testApp::draw(){
    // draw current video frame to screen
//    if(bShowVideo == false)    img.draw(0, 0, camWidth, camHeight);
    if(bShowVideo == false)    img.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    
    //print a counter (dont use for anything)
    //    ofDrawBitmapStringHighlight("counter: " + ofToString(counter), ofPoint(10,30));
    
    
    // display other items
    if(showTest) test_image.draw(camWidth*SCALE +100, 0);
	if(showFaces) rec.drawFaces(0, ofGetHeight()*0.8, ofGetWidth());
	if(showEigens) rec.drawEigens(0, ofGetHeight()*0.9, ofGetWidth());
    
    person = -1;
    
    if(bgSubtract) bgImage.draw(0, 400);
    std::ostringstream fr;
    std::ostringstream o;
    
	for(int i = 0; i < finder.blobs.size(); i++) {
		ofRectangle cur = finder.blobs[i].boundingRect;
        
        cur.x*=TEST_DIV;
        cur.y*=TEST_DIV;
        cur.width*=TEST_DIV;
        cur.height*=TEST_DIV;
        
		face.grabScreen(cur.x, cur.y, cur.width, cur.height);
        
        int tx = cur.x;
        int ty = cur.y;
        int tw = cur.width;
        int th = cur.height;
        
        unsigned char *img_px = img.getPixels();
        
		unsigned char *temp = new unsigned char[tw*th*3];
		for (int x=0; x<tw; x++)
            for (int y=0; y<th; y++) {
//                temp[(x+y*tw)*3] = img_px[((x+tx)+(y+ty)*camWidth)*3];
                temp[(x+y*tw)*3+1] = img_px[((x+tx)+(y+ty)*camWidth)*3+1];
                temp[(x+y*tw)*3+2] = img_px[((x+tx)+(y+ty)*camWidth)*3+2];
            }
        //        face.setFromPixels(temp, cur.width, cur.height, OF_IMAGE_COLOR);
        face.setFromPixels(temp, 600, 10, OF_IMAGE_COLOR);
        delete temp;
        
        //        face.resize(PCA_WIDTH, PCA_HEIGHT);
        //        face.update();
        color = face.getPixels();
        gray = color;
        
        unsigned char *pixels = gray.getPixels();
        for(int x=0; x<PCA_WIDTH; x++)
            for(int y=0; y<PCA_HEIGHT; y++)
                if(mask.getPixels()[x+y*PCA_HEIGHT]<=0)
                    pixels[x+y*PCA_HEIGHT]=128;
        gray = pixels;
        
        
        person=rec.recognize(gray);
        
        
        ofSetColor(255, 255, 255, 192);
        
        // super-impose matched face over detected face
//        if(bShowVideo == true) faces[person].draw(cur.x, cur.y, cur.width, cur.height);
        
        //print persons name on the screen!
//        rec.drawNames(10, 10, person);
        // CONFIDENCE CALCULATION
//        cout << "currentPerson: " << person << " prevPerson: " << lastName << endl;
        if(person == lastName){
            confidence ++;
        } else {
            confidence = confidence - 0.4;
        }
        
//        cout << "foo " << minhavar << endl;
        if(confidence >= 20) {
            rec.drawNames(cur.x+70, cur.y, minhavar);
            bPersonFound = true;
        }
        cout << "minha var :: " << minhavar << endl;

        ofDrawBitmapStringHighlight("confidence: "+ofToString(confidence), ofPoint(ofGetWindowWidth() - 150, 10));
        
        // show fit data
        if(showLeastSq) {
            o << rec.getLeastDistSq();
            ofDrawBitmapString(o.str(), cur.x, cur.y);
        }
        
        // show timing
        if (showClock) {
            fr << ofGetFrameRate();
            ofDrawBitmapString(fr.str(), 20, 20);
        }
        
        // highlight current face from board of faces
        if(showFaces) rec.drawHilight(person, 0, ofGetHeight()*0.8, ofGetWidth());
        
        // reset color
        ofSetColor(255, 255, 255, 255);
        
    }
    
    ofDrawBitmapStringHighlight("Room D1204", ofPoint(15,15));

}


//------------------------------------------------------------------------
void testApp::keyPressed  (int key){
    
    if(key == 's')
        face.saveImage("face.tif");
    
    if (key == 'S') {
        ofImage screengrab;
        screengrab.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
        screengrab.saveImage("screen.tif");
    };
    
    if(key == 'r'){
        minhavar = 0;
        bPersonFound = false;
        confidence = 0;
    }
    
    if(key=='R'){
        setup();
        
    }
    if (key == 't') ofToggleFullscreen();
    
    if((key == 'e') || (key == 'E'))
        showEigens = (showEigens == false);
    
    if((key == 'f') || (key == 'F'))
        showFaces = (showFaces == false);
//    
//    if((key == 't') || (key == 'T'))
//        showTest = (showTest == false);
    
    if((key == 'l') || (key == 'L'))
        showLeastSq = (showLeastSq == false);
    
    if(((key == 'b') || (key == 'B')) && (!bgSubtract)) {
        bgImage.setFromPixels(img.getPixels(), camWidth, camHeight, OF_IMAGE_COLOR);
        bgSubtract = true;
    }
    if(key=='v') bShowVideo = !bShowVideo;
    
}
