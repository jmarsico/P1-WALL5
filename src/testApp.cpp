#include "testApp.h"
#include <iostream>
#include <stdio.h>
#include <sstream>

float offX = 60;
float offY = 90;
//--------------------------------------------------------------
void testApp::setup(){

    vector<string> array;

    readAndParseCSV(&array);

    cout << nodes.size();

    Cell cell;

    bInvert = false;
    bGrayscale = true;

    /////VIDEO
    #ifdef _USE_LIVE_VIDEO
    camWidth 		= 320;	// try to grab at this size.
	camHeight 		= 240;

	vidGrabber.setVerbose(false);
	vidGrabber.setDeviceID(1);
	vidGrabber.setDesiredFrameRate(30);
	vidGrabber.initGrabber(camWidth,camHeight);

	#else
        vidPlayer.loadMovie("fingers.mov");
        vidPlayer.play();
	#endif

    colorImg.allocate(320,240);
	grayImage.allocate(320,240);
	grayImageLast.allocate(320,240);
	grayBg.allocate(320,240);
	grayDiff.allocate(320,240);
	grayDiffCopy.allocate(320,240);
	grayDiffScale.allocate(320,240);
    grayDiffBg.allocate(320,240);
	bLearnBakground = true;
	threshold = 80;

	//SERIALxCellBrightness
	bSendSerialMessage = false;
	ofSetLogLevel(OF_LOG_VERBOSE);

//	font.loadFont("DIN.otf", 64);

	serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();

	// this should be set to whatever com port your serial device is connected to.
	// (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
	// arduino users check in arduino app....
	int baud = 2000000;
	//int baud = 460800;

	//serial.setup(2, baud); //open the first device
	//serial.setup("COM4", baud); // windows example
	//serial.setup("/dev/tty.usbserial-A4001JEC", baud); // mac osx example
	cout << serial.setup("/dev/ttyACM0", baud) << endl; //linux example

	nTimesRead = 0;
	nBytesRead = 0;
	readTime = 0;
	memset(bytesReadString, 0, 4);
	counter=0;
	backgroundTimer = ofGetElapsedTimeMillis();

}
int val = 0;
int LEDLength=277;
//--------------------------------------------------------------
void testApp::update(){
    ofLog() << "top of update";

    ofBackground(100,100,100);

    bool bNewFrame = false;

	#ifdef _USE_LIVE_VIDEO
       //vidGrabber.grabFrame();
       vidGrabber.update();
	   bNewFrame = vidGrabber.isFrameNew();
    #else
        vidPlayer.update();
        bNewFrame = vidPlayer.isFrameNew();
	#endif

	if (bNewFrame){

		#ifdef _USE_LIVE_VIDEO
            colorImg.setFromPixels(vidGrabber.getPixels().getData(), 320,240);
	    #else
            colorImg.setFromPixels(vidPlayer.getPixels().getData(), 320,240);
        #endif

        colorImg.mirror(true,true);

        grayImage = colorImg;
        ofLog() << "colorImg - grayImage";
        

		if(ofGetElapsedTimeMillis() - backgroundTimer > 5000){
            backgroundTimer = ofGetElapsedTimeMillis();     //Set the background timer to current time
            grayDiffBg.absDiff(grayImageLast, grayImage);   //Get the difference between current image and last image 5 seconds ago
            grayDiffBgBrt = 0;
            unsigned char * diffPixels = grayDiffBg.getPixels().getData();
            for(int i = 0; i < 320; i++){
                for(int j = 0; j < 240; j++){
                grayDiffBgBrt += diffPixels[j*320 + i];
                }
            }
           // cout << "BackGround LOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOP:  "  << grayDiffBgBrt << endl;
            if(grayDiffBgBrt < 200000){
                bLearnBakground = true;
           //     cout << "BackGround Captured"  << endl;
           //     cout << "BackGround Captured"  << endl;
            //    cout << "BackGround Captured"  << endl;
            //    cout << "BackGround Captured"  << endl;
            }
            grayImageLast.setFromPixels(grayImage.getPixels().getData(), 320, 240);                      //Set the last gray image to current gray Image
	    }

	    if (bLearnBakground == true){
			grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
			bLearnBakground = false;
		}

		// take the abs value of the difference between background and incoming and then threshold:
		grayDiff.absDiff(grayBg, grayImage);
		grayDiff.threshold(threshold);
        grayDiff.dilate();
        grayDiff.dilate();
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayDiff, 800, (320*240)/3, 50, true);	// find holes
	}

	if (bInvert) {
            for (int i = 0; i < cellBrightness.size(); i ++){
                xCellBrightness[i] = 255 - cellBrightness[i];
            }
        }

///////Send Serial/////////////////////
    if (ofGetFrameNum()%3 == 0){
        unsigned char buf[LEDLength+1];
         for(int i = 0; i < LEDLength; i++){
            buf[i] = xCellBrightness[i];
            if(i==LEDLength-1) buf[i+1] = 254;
        }
       serial.writeBytes(&buf[0], LEDLength+1);
    }

///////Receive Serial/////////////////////
		int nBytesRead = 0;
		int nRead  = 0;  // a temp variable to keep count per read

		unsigned char bytesReturned[1];
        unsigned char bytesReadString[2];

		memset(bytesReadString, 0, 2);
		memset(bytesReturned, 0, 1);

		while( (nRead = serial.readBytes( bytesReturned, 1)) > 0){
			nBytesRead = nRead;
            cout << "Arduino:  "  <<  ofToString(bytesReturned)  << endl;
		};

        ofLog() << "bottom of update";
}

//--------------------------------------------------------------
void testApp::draw(){

    ofBackground(255);

    if (bGrayscale) {
        getGrayscaleBrightness();
        createSynapse();
    }

    if (bInvert) {
        for (int i = 0; i < cellBrightness.size(); i ++){
            xCellBrightness[i] = 255 - cellBrightness[i];
        }
    }

    if(bDrawCells){
        drawAllCells(xCellBrightness);  //draw the cells to visualizer
    }

    //////CV////////
    if(bDrawVideo){

        ofPushMatrix();
        ofTranslate(680,0);

        // draw the incoming, the grayscale, the bg and the thresholded difference
        ofSetHexColor(0xffffff);
        //colorImg.draw(20,20);
        grayImage.draw(20,120);
        //grayBg.draw(20,280);
        grayDiff.draw(20,400);

        grayDiffBg.draw(-320,120);

        // then draw the contours:

        ofFill();
        ofSetHexColor(0x333333);
        //ofRect(360,980,320,240);
        ofSetHexColor(0xffffff);

        for (int i = 0; i < contourFinder.nBlobs; i++){
            contourFinder.blobs[i].draw(20,400);
//            if (bGrayscale) {
//            getGrayscaleBrightness();
//        }
        }
    }
	// finally, a report:

	ofSetHexColor(0x000fff);
	char reportStr[1024];
	sprintf(reportStr, "bg subtraction and blob detection\npress ' ' to capture bg\nthreshold %i (press: +/-)\nnum blobs found %i, fps: %f, \nbackDiff: %i", threshold, contourFinder.nBlobs, ofGetFrameRate(),grayDiffBgBrt);
	ofDrawBitmapString(reportStr, 20, 600);
	ofPopMatrix();

}

float testApp::distance(ofPoint p1, ofPoint p2){
    float temp;
    temp = ofDist(p1.x, p1.y, p2.x, p2.y);
    return temp;
}

void testApp::createSynapse(){
  //  vector<int> trianglesOn;
//    for (int i = 0; i < cells.size(); i ++){
//        if(!cells[i].triangle && xCellBrightness[i] > 0){
//            for(int j = 0; j < cells[i].neighbors.size(); j++){
//                int index = cells[i].neighbors[j];
//                //trianglesOn.push_back(index);
//                if(xCellBrightness[cells[index].cellNum] == 0)
//                    xCellBrightness[cells[index].cellNum] = 255;
//            }
//        }
//    }

    for (int i = 0; i < cells.size(); i ++){
        if(cells[i].triangle && xCellBrightness[i] > 0){
                    xCellBrightness[i] = 255;
        }
    }

//    for (int i = 0; i < cells.size(); i ++){
//       // if(xCellBrightness[i] > 0){
//            for(int j = 0; j < cells[i].neighbors.size(); j++){
//                int index = cells[i].neighbors[j];
//                if(xCellBrightness[i] > 0)
//                    triangleOn.push_back(index);
//            }
//       // }
//    }
//    for (int i = 0; i < triangleOn.size(); i ++){
//        xCellBrightness[triangleOn[i]] = 255;
//    }
}

void testApp::getGrayscaleBrightness(){
    ofLog() << "top of grayscalebrightness";
    grayDiffCopy.resize(320,240);
    grayDiffCopy = grayDiff;
    grayDiffCopy.resize(320,200);
    grayDiffScale.scaleIntoMe(grayDiffCopy);

    unsigned char * pixels = grayImage.getPixels().getData();
    unsigned char * pixelsDiff = grayDiff.getPixels().getData();

    for (int i = 0; i < cells.size(); i++){

               int brightAvg = 0;
               int tempVal;
               int tempVal2;
               int tempPixel = (int)cells[i].normalizedNode.x+(int)cells[i].normalizedNode.y*320;
               ofColor tempColor = pixelsDiff[tempPixel];
               tempVal2 = tempColor.getBrightness();
               tempVal = tempVal2;
               tempVal = ofMap(tempVal2, 0, 255, 0, 253);
               cellBrightness[i] = tempVal;
               xCellBrightness[i] = cellBrightness[i];
    }
    ofLog() << "bottom of grayscalebrightness";
}



void testApp::blobMask(){


}



void testApp::drawAllCells(vector<int> brightness){

    ofPushMatrix();
    ofTranslate(20,20);
    ofScale(2,2,0);

    for (int i = 0; i < brightness.size(); i ++){      //Draw Cells
        ofFill();
        ofSetColor(brightness[i]);
        ofSetPolyMode(OF_POLY_WINDING_NONZERO);
        ofBeginShape();
        for (int j = 0; j < cells[i].normalizedVertices.size(); j++){ // Draw Fill
            ofVertex(cells[i].normalizedVertices[j].x, cells[i].normalizedVertices[j].y );
        }
        ofEndShape();
        ofBeginShape();
        ofEnableSmoothing();
        ofNoFill();
        ofSetColor(255);
        for (int j = 0; j < cells[i].normalizedVertices.size(); j++){ // Draw Stroke

            ofVertex(cells[i].normalizedVertices[j].x, cells[i].normalizedVertices[j].y );

        }
        ofEndShape();
    }


    ofPopMatrix();

    }



//--------------------------------------------------------------
void testApp::keyPressed(int key){

    switch (key){
		case ' ':
			bLearnBakground = !bLearnBakground;
			break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
        case 'i':
			bInvert = !bInvert;
			break;
        case 'g':
			bGrayscale = !bGrayscale;
			break;
        case 's':
			bSynapse = !bSynapse;
			break;
        case 'v':
            bDrawVideo = !bDrawVideo;
            break;
        case 'c':
            bDrawCells = !bDrawCells;
            break;
	}

}

void testApp::normalizeNodes(float minX, float maxX, float minY, float maxY){

    for (int i = 0; i < cells.size(); i ++){
        ofPoint temp;
        temp.x = ofMap(cells[i].node.x, minX, maxX, 0.0+offX, 320.0-offX);
        temp.y = ofMap(cells[i].node.y, minY, maxY, 0.0+offY, 240.0-offY);
        cells[i].normalizedNode = temp;

    }

cout << "MinX:" << minX << "  MaxX:" << maxX << "  MinY:" << minY << "  MaxY:" << maxY << endl;

}


void testApp::normalizeVertices(float minX, float maxX, float minY, float maxY){

    for (int i = 0; i < cells.size(); i ++){
        for (int j = 0; j < cells[i].outLineVertices.size(); j++){
            ofPoint temp;
            temp.x = ofMap(cells[i].outLineVertices[j].x, minX, maxX, 0.0+offX, 320.0-offX);
            temp.y = ofMap(cells[i].outLineVertices[j].y, minY, maxY, 0.0+offY, 240.0-offY);
            cells[i].normalizedVertices.push_back(temp);
        }
    }

cout << "MinX:" << minX << "  MaxX:" << maxX << "  MinY:" << minY << "  MaxY:" << maxY << endl;

}

template<class T>
    T fromString(const std::string& s)
{
     std::istringstream stream (s);
     T t;
     stream >> t;
     return t;
}

void testApp::readAndParseCSV(vector<string>* array) {
    ifstream inFile ("WALL5DATA.csv");
    string line;
    int linenum = 0;
    //max and min for normalizing nodes
    float maxX = -1000;
    float minX = 1000;
    float maxY = -1000;
    float minY = 1000;

    while (getline (inFile, line)) //loop through each line of the .csv
    {
        linenum++;

        cout << "\nLine #" << linenum << ":" << endl;
        istringstream linestream(line);
        string item;
        int itemnum = 0;
        int cellNumTemp = 0;
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
        vector<float> tempXVertices;
        vector<float> tempYVertices;
        vector<int> tempNeighbors;
        vector<int> tempLED;
        float x0,y0,x1,y1,x2,y2,x3,y3,x4,y4,x5,y5;

        while (getline (linestream, item, ';')) //loop through items in each line. Semi Colon delineated
       // while (getline (linestream, item, ',')) //loop through items in each line. Comma delineated
        {
            itemnum++;
            //cout << "Item #" << itemnum << ": " << item << endl;
            const char* str_float;
            str_float = item.c_str();
            float f = atof(str_float);
            switch(itemnum) {

                case 34:
                 cellNumTemp = (int)f;
                 break;

                case 2:
                 x = f;
                 break;

                case 3:
                 y = f;
                 break;

                case 4:
                 z = f;
                 break;

                case 6:
                tempXVertices.push_back(f);
                break;

                case 7:
                tempYVertices.push_back(f);
                break;

                case 8:
                tempXVertices.push_back(f);
                break;

                case 9:
                tempYVertices.push_back(f);
                break;

                case 10:
                tempXVertices.push_back(f);
                break;

                case 11:
                tempYVertices.push_back(f);
                break;

                case 12:
                tempXVertices.push_back(f);
                break;

                case 13:
                tempYVertices.push_back(f);
                break;

                case 14:
                tempXVertices.push_back(f);
                break;

                case 15:
                tempYVertices.push_back(f);
                break;

                case 16:
                tempXVertices.push_back(f);
                break;

                case 17:
                tempYVertices.push_back(f);
                break;

                 case 18:
                tempXVertices.push_back(f);
                break;

                case 19:
                tempYVertices.push_back(f);
                break;

                case 20:
                tempXVertices.push_back(f);
                break;

                case 21:
                tempYVertices.push_back(f);
                break;

                case 22:
                tempXVertices.push_back(f);
                break;

                case 23:
                tempYVertices.push_back(f);
                break;

                case 24:
                tempXVertices.push_back(f);
                break;

                case 25:
                tempYVertices.push_back(f);
                break;

                case 26:
                tempNeighbors.push_back((int)f);
                break;

                case 27:
                tempNeighbors.push_back((int)f);
                break;

                case 28:
                tempNeighbors.push_back((int)f);
                break;

                case 29:
                tempNeighbors.push_back((int)f);
                break;

                case 30:
                tempNeighbors.push_back((int)f);
                break;

                case 31:
                tempNeighbors.push_back((int)f);
                break;

//                case 32:
//                tempLED.push_back((int)f);
//                break;

            }

        }

        ofPoint temp;
        temp.set(x,y,z);
        nodes.push_back(temp);
        vector<ofPoint> tempVertList;
        if (x > maxX) maxX = x;
        if (x < minX) minX = x;
        if (y > maxY) maxY = y;
        if (y < minY) minY = y;


        for (int i = 0; i < 10; i++){
            if (tempXVertices[i] >= 0) {
                ofPoint temp1;
                temp1.set(tempXVertices[i],tempYVertices[i],0);
                tempVertList.push_back(temp1);
            }
        }



            Cell tempCell = Cell();
            for (int i = 0; i < tempNeighbors.size(); i++){
            if (tempNeighbors[i] >= 0){
                tempCell.neighbors.push_back(tempNeighbors[i]);
            }
            if (tempCell.neighbors.size() >0)
                tempCell.triangle = false;
            else
                tempCell.triangle = true;

        }
            tempCell.node = temp;
            tempCell.outLineVertices = tempVertList;
            tempCell.brightness = 0;
            tempCell.edge = false;
            tempCell.cellNum = cellNumTemp;
            cells.push_back(tempCell);
            cellBrightness.push_back(0);
            xCellBrightness.push_back(0);

    }

    //Call the Normalize Function Here
    normalizeNodes(minX, maxX, minY, maxY);
    normalizeVertices(minX, maxX, minY, maxY);

    return;
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
