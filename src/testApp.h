#pragma once
#include "Cell.h"
#include "ofMain.h"
#include "ofxOpenCv.h"

#define _USE_LIVE_VIDEO		// uncomment this to use a live camera


class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		///////FILE READ

		void readAndParseCSV(vector<string>* array);
		void normalizeNodes(float minX, float maxX, float minY, float maxY);
		void normalizeVertices(float minX, float maxX, float minY, float maxY);

		void invertBrightness(vector<int> brightness);
		void getGrayscaleBrightness();
		void createSynapse();
		void blobMask();
		float distance(ofPoint x, ofPoint y);

		void findEdgeCells();

		void drawAllCells(vector<int> brightness);

        vector<ofPoint> nodes;
        vector<Cell> cells;

        vector<int> cellBrightness;
        vector<int> xCellBrightness;
        vector<uint8_t> iCellBrightness;

        vector<vector<ofPoint> > outLineVertices;

        ///////SERIAL


		bool		bSendSerialMessage;			// a flag for sending serial
		char		bytesRead[3];				// data from serial, we will be trying to read 3
		char		bytesReadString[4];			// a string needs a null terminator, so we need 3 + 1 bytes
		int			nBytesRead;					// how much did we read?
		int			nTimesRead;					// how many times did we read?
		float		readTime;					// when did we last read?

		int         counter;

		ofSerial	serial;


        ///////VIDEO

        #ifdef _USE_LIVE_VIDEO
        ofVideoGrabber 		vidGrabber;
        unsigned char * 	videoInverted;
		ofTexture			videoTexture;
		int 				camWidth;
		int 				camHeight;
		#else
		  ofVideoPlayer 		vidPlayer;
		#endif

        ofxCvColorImage			colorImg;


        ofxCvGrayscaleImage 	grayImage;
		ofxCvGrayscaleImage 	grayImageLast;
		ofxCvGrayscaleImage 	grayDiffBg;
		ofxCvGrayscaleImage 	grayBg;
		ofxCvGrayscaleImage 	grayDiff;
		ofxCvGrayscaleImage 	grayDiffScale;
		ofxCvGrayscaleImage 	grayDiffCopy;

        ofxCvContourFinder 	contourFinder;

		int 				threshold;
		int 				backgroundTimer;
		bool				bLearnBakground;
        int                 grayDiffBgBrt;

		bool				bInvert;
		bool				bGrayscale;
		bool				bSynapse;
        bool				bDrawVideo;
        bool				bDrawCells;
};
