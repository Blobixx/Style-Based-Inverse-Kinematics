#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <QTextStream>

#include "operateur.hpp"
#include "bvh.hpp"
#include "main.hpp"

using namespace std;

bvhPart::bvhPart()
{
}

//Function reading the file using init
bvh::bvh(QString* bvhFile)
{
	init(bvhFile); 
	return;
}

// Read and process every line of the file
void bvh::process(QString line)
{
	if (line == "OFFSET") {
		vertIndex = 0;	
		theMode = OFFSET;
	} else if (line == "ROOT") {
		root = new bvhPart;
		root->parent = NULL;
		current = root;
		theMode = ROOT;
	} else if (line == "{") {
			
	} else if (line == "}") { 
			if (current != root) { 
				current = current->parent;
		       	theMode = NONE;
			}
	} else if (line == "JOINT") {
		bvhPart *temp = new bvhPart;
		temp->parent = current;
		current->child.push_back(temp);
		current = temp;
		theMode = JOINT; 
	} else if (line == "CHANNELS") {
		theMode = CHANNELS;
	} else if (line == "End") {	
                theMode = End;
	} else if (line == "Site") {
		bvhPart *temp = new bvhPart;
		temp->parent = current;
		current->child.push_back(temp);
		current = temp;
		theMode = NONE; 
	} else if (line == "MOTION") {
		theMode = MOTION;
	} else if (line == "Frames:") {
		theMode = Frames;
	} else if (line == "Frame") {
		theMode = Frame;
	} else if (line == "Time:") {
		theMode = Time;		
	} else {
		
		switch (theMode) {
			case (ROOT):
				root->name = line;
				theMode = NONE;
				break;

			case (JOINT): 
				break ;
			
			case (OFFSET):
                current->offset.vertex[vertIndex] = atof(line.toStdString().c_str());
				vertIndex++;
				break;

			case (CHANNELS):
				if (line == "Xposition") {		
					current->channels.push_back(bvhPart::Xpos);	
				} else if (line == "Yposition") {					
					current->channels.push_back(bvhPart::Ypos);	
				} else if (line == "Zposition") {				
					current->channels.push_back(bvhPart::Zpos);	
				} else if (line == "Zrotation") {
					current->channels.push_back(bvhPart::Zrot);				
				} else if (line == "Yrotation") {
					current->channels.push_back(bvhPart::Yrot);				
				} else if (line == "Xrotation") {
					current->channels.push_back(bvhPart::Xrot);				
				}
				
				break;

			case (Frames):
                framesNum = atoi(line.toStdString().c_str());
				theMode = NONE;
				break;

			case (Frame):
				break;

			case (Time):
                frameTime = atof(line.toStdString().c_str());
				theMode = MOTIONDATA;
				current = root;
				recurs(root);
				break;

			case (MOTIONDATA):
				
				data++;
				switch (bvhPartsLinear[partIndex]->channels[channelIndex]) {
					case (bvhPart::Xpos):	
                        tempMotion.Translate(atof(line.toStdString().c_str()),0,0);
						channelIndex++;
						break;
					case (bvhPart::Ypos):
                        tempMotion.Translate(0,atof(line.toStdString().c_str()),0);
						channelIndex++;
						break;
					case (bvhPart::Zpos):
                        tempMotion.Translate(0,0,atof(line.toStdString().c_str()));
						channelIndex++;
						break;
					case (bvhPart::Zrot):
                        tempMotionZ.RotateZ((float)-DEG_TO_RAD(atof(line.toStdString().c_str())));
						channelIndex++;
						break;
					case (bvhPart::Yrot):
                        tempMotionY.RotateY((float)-DEG_TO_RAD(atof(line.toStdString().c_str())));
						channelIndex++;
						break;
					case (bvhPart::Xrot):
                        tempMotionX.RotateX((float)-DEG_TO_RAD(atof(line.toStdString().c_str())));
						channelIndex++;
						break;
				}
				
				if (channelIndex >= bvhPartsLinear[partIndex]->channels.size()) {
					tempMotion = tempMotion * (tempMotionZ *tempMotionX * tempMotionY );
					bvhPartsLinear[partIndex]->motion.push_back(tempMotion);
					
					tempMotion.LoadIdentity();
					tempMotionX.LoadIdentity();
					tempMotionY.LoadIdentity();
					tempMotionZ.LoadIdentity();
	

					channelIndex = 0;
					partIndex++;
				}	
				if (partIndex >= bvhPartsLinear.size()) {
					partIndex = 0;
				}	
				
			case (NONE):
			case (End):
			case (Site):
			case (MOTION):
				break;
			
		}
	}
}

// Create the bvhPartsLinear and the motion0 (with the offset) for every bvhPart
void bvh::recurs(bvhPart* some)
{
	matrix16f motion0;
	motion0.LoadIdentity();
	 
	motion0.Translate(some->offset.vertex[0],some->offset.vertex[1],some->offset.vertex[2]);
	some->motion.push_back(motion0);
	if (some->child.size() != 0) bvhPartsLinear.push_back(some);

	unsigned i;
	for (i = 0; i < some->child.size(); i++)
		recurs(some->child[i]);				
}
 
// Read the file line per line thanks to process() 
void bvh::init(QString* bvhFile)
{
	data = 0;
	partIndex = 0 ;
	channelIndex = 0 ;	

	tempMotion.LoadIdentity();
	tempMotionX.LoadIdentity();
	tempMotionY.LoadIdentity();
	tempMotionZ.LoadIdentity();

    //ifstream bvhStream(bvhFile.toStdString().c_str());
    QTextStream bvhStream(bvhFile,QIODevice::ReadOnly);
	
    //istream_iterator<string> bvhIt(bvhStream);
    //istream_iterator<string> sentinel;

    //vector<QString> lines(bvhIt,sentinel);
    QString line;
    while (!bvhStream.atEnd()){

        line = bvhStream.readLine();
        process(line);

    }
	
    /*
	unsigned i;
	for (i =0; i< lines.size(); i++) {
		process(lines[i]);
    }*/
    //bvhStream.close();
	
    //seg fault avec cette ligne ... :/
    //framesNum = bvhPartsLinear[0]->motion.size();
}

