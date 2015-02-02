//
//  DataRecorder.cpp
//  LeapMotion
//
//  Created by Alvin Sun on 2015-01-28.
//
//

#include <stdio.h>
#include "DataRecorder.h"
#include "Leap.h"
#include "Cinder-LeapMotion.h"
#include "cinder/Channel.h"
#include "cinder/Matrix.h"
#include "cinder/Thread.h"
#include "cinder/Vector.h"
#include "DataRecorder.h"
#include <functional>
using namespace std;

DataRecorder::DataRecorder(){
    this->isWriting = false;
    this->currentFileName = "";
    this->currentFrameIndex = 0;
}

void DataRecorder::ParseCurrentFrametoFile(Leap::Frame currentFrame){
    if (!isWriting) {
        time_t t = time(0);   // get time now
        struct tm * now = localtime( & t );
        stringstream ss;
        stringstream ssLeapData;
        stringstream ssLeftHandData;
        stringstream ssRightHandData;
        ss << (now->tm_year + 1900) << '-'
        << (now->tm_mon + 1) << '-'
        <<  now->tm_mday << '-' << now->tm_hour << '-' << now->tm_min << '-' << now->tm_sec;
        
        ssLeapData << ss.str() << ".data" << endl;
        ssLeftHandData << "LeftHand-" <<ss.str() << ".txt" << endl;
        ssRightHandData << "RgithHand-" <<ss.str() << ".txt" << endl;
        currentFileName = ssLeapData.str();
        currentLeftHandFileName = ssLeftHandData.str();
        currentRighthandFileName = ssRightHandData.str();
        InitHandMotionTxtFile(currentLeftHandFileName);
        InitHandMotionTxtFile(currentRighthandFileName);
        
        isWriting = true;
        currentFrameIndex = 0;
    }
    fstream out(currentFileName, std::ios_base::app | std::ios_base::out);
    if(out)
    {
        Leap::Frame frameToSerialize = currentFrame;
        std::string serialized = frameToSerialize.serialize();
        out << (long)serialized.length() << serialized;
        out.flush();
        out.close();
    }
    else if(errno) {
        std::cout << "Error: " << errno << std::endl;
        std::cout << "Couldn't open " << currentFileName << " for writing." << std::endl;
    }
    const Leap::HandList& hands = currentFrame.hands();
    for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
        Leap::Hand hand = *handIter;
        if (hand.isLeft()){
            WriteToLeftHandFile(hand);
        }else{
            WriteToRightHandFile(hand);
        }
    }
}

void DataRecorder::EndReocrding(){
    this->isWriting = false;
}

string DataRecorder::ParseOneRowHandInformation(Leap::Hand hand){
    cinder::Vec3f handDir		= LeapMotion::toVec3f( hand.direction() );
    cinder::Vec3f palmNorm		= LeapMotion::toVec3f( hand.palmNormal() );
    cinder::Vec3f palmPos		= LeapMotion::toVec3f( hand.palmPosition() );
    cinder::Vec3f palmVel		= LeapMotion::toVec3f( hand.palmVelocity() );

    std::stringstream oneRowString;
    oneRowString << handDir.x << " " << handDir.y << " " << handDir.z << " " << palmNorm.x << " " << palmNorm.y << " " << palmNorm.z << " " << palmPos.x << " " << palmPos.y << " " << palmPos.z <<" " << palmVel.x << " " << palmVel.y << " " << palmVel.z << " " << hand.confidence();
    
    return oneRowString.str();
    
}

void DataRecorder::WriteToLeftHandFile(Leap::Hand leftHand){
    
    fstream out(currentLeftHandFileName, std::ios_base::app | std::ios_base::out);
    if(out)
    {
        out << leftHand.frame().timestamp() <<"  "<< leftHand.frame().id() << "  "<< ParseOneRowHandInformation(leftHand) << std::endl;
        out.flush();
        out.close();
    }
    else if(errno) {
        std::cout << "Error: " << errno << std::endl;
        std::cout << "Couldn't open " << currentFileName << " for writing." << std::endl;
    }

}

void DataRecorder::WriteToRightHandFile(Leap::Hand rightHand){
    
    fstream out(currentRighthandFileName, std::ios_base::app | std::ios_base::out);
    if(out)
    {
        out << rightHand.frame().timestamp() <<"  "<< rightHand.frame().id() << "  "<< ParseOneRowHandInformation(rightHand) << std::endl;
        out.flush();
        out.close();
    }
    else if(errno) {
        std::cout << "Error: " << errno << std::endl;
        std::cout << "Couldn't open " << currentFileName << " for writing." << std::endl;
    }
}

void DataRecorder::InitHandMotionTxtFile(string filename){
    fstream out(filename, std::ios_base::app | std::ios_base::out);
    if(out)
    {
        out << "TimeStamp" <<"  "<< "FrameID" << "  "<< "HandDirection:X" << " " << "HandDirection:Y" << " "<<"HandDirection:Z" << "  "<< "PalmNorm:X" << " " << "PalmNorm:Y" << " "<<"PalmNorm:Z"<< "  "<< "PalmPos:X" << " " << "PalmPos:Y" << " "<<"PalmPos:Z"<<  "  "<< "PalmVel:X" << " " << "PalmVel:Y" << " "<<"PalmVel:Z" << " " << "Confidence" << std::endl;
        out.flush();
        out.close();
    }
    else if(errno) {
        std::cout << "Error: " << errno << std::endl;
        std::cout << "Couldn't open " << currentFileName << " for writing." << std::endl;
    }

}
