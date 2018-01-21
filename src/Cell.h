// make sure we have one and only one "First class"
#pragma once
#include "ofMain.h"
// give the class a name
class Cell{
// declare all the public variables and methods, that is, all the values that can
// be shared
public:
   // Cell();


    ofPoint node;
    ofPoint normalizedNode;
    vector<ofPoint> outLineVertices;
    vector<ofPoint> normalizedVertices;
    vector<int> neighbors;
    int brightness;
    int cellNum;
    bool edge;
    bool triangle;


// declare all the private variables and methods, that is, all the values that are
// only for the internal workings of the class, not for sharing
private:

};

