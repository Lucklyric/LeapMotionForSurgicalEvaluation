//
//  main.cpp
//  LeapMotion
//
//  Created by Alvin Sun on 2015-01-23.
//  Copyright (c) 2015 Alvin Sun. All rights reserved.
//

#include "cinder/app/AppBasic.h"
#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Quaternion.h"
#include "Leap.h"
#include "Cinder-LeapMotion.h"
#include "cinder/Thread.h"
#include "cinder/params/Params.h"
#include "AntTweakBar.h"
#include <functional>
//Windows header files used
//#define WINVER 0x0500
//#include <iostream>
//#include <time.h>

using namespace LeapMotion;
using namespace Leap;
using namespace ci;
using namespace ci::app;

#define __BONES__

class LeapMotionMain : public ci::app::AppBasic
{
public:
    void draw();
    void drawHand(Leap::Hand &hand, Vec3f position);
    void drawHands();
    void setup();
    void shutdown();
    void setupGui();
    void update();
    void prepareSettings(Settings *settings);
    void keyDown( KeyEvent event );
    std::shared_ptr<std::thread>		mThread;
private:
    
    Frame                   mFrame;
    LeapMotion::DeviceRef	mLeap;
    void 					onFrame( Leap::Frame frame );
    
    ci::CameraPersp			mCamera;
    ci::params::InterfaceGlRef	mParams;
    bool showParams;
    
    
    
    Quatf mObjOrientation;
    float mScale;
    Vec3f mTranslate;
    
    bool mStaticPosHand, mStaticOrientHand;
    
    
    
};
void LeapMotionMain::prepareSettings( Settings *settings){
    //setting the window size and the frame rate for processing
    settings->setWindowSize ( 1600, 900);
    settings->setFrameRate (100.0f);
}

void LeapMotionMain::shutdown()
{
}

void LeapMotionMain::drawHand(Leap::Hand &hand, Vec3f position)
{
    gl::translate(position);
    gl::scale(mScale, mScale, mScale);
    
    Leap::Matrix handTransform = hand.basis();
    
    if (!mStaticOrientHand)
    {
        handTransform = handTransform.rigidInverse();
        gl::pushMatrices();
        
        gl::multModelView(LeapMotion::toMatrix44f(handTransform));
    }
    
    gl::drawColorCube(Vec3f(0,0,0), Vec3f( 40, 10, 40 ) );
    
    
    
    if (!mStaticOrientHand)
        gl::popMatrices();
    else
    {
        gl::pushMatrices();
        gl::multModelView(LeapMotion::toMatrix44f(handTransform));
    }
    
    const Leap::FingerList& fingers = hand.fingers();
    for ( Leap::FingerList::const_iterator pointIter = fingers.begin(); pointIter != fingers.end(); ++pointIter ) {
        const Leap::Finger& finger = *pointIter;
        
        Leap::Vector fingerTipOrigin;
        
#ifdef __BONES__
        
        Leap::Vector fingerPipPos;
        Leap::Vector fingerDipPos;
        Leap::Vector fingerMcpPos;
        
        
        Leap::Bone tipBone = finger.bone(Bone::Type::TYPE_DISTAL);
        Leap::Bone pipBone = finger.bone(Bone::Type::TYPE_INTERMEDIATE);
        Leap::Bone dipBone = finger.bone(Bone::Type::TYPE_PROXIMAL);
        Leap::Bone mcpBone = finger.bone(Bone::Type::TYPE_METACARPAL);
        
        fingerTipOrigin =  tipBone.center();
        fingerPipPos = pipBone.center();
        fingerDipPos = dipBone.center();
        fingerMcpPos = mcpBone.center();
        
        fingerPipPos -= hand.palmPosition();
        fingerDipPos -= hand.palmPosition();
        fingerMcpPos -= hand.palmPosition();
        
#else
        fingerTipOrigin =  finger.tipPosition();
#endif
        
        fingerTipOrigin -= hand.palmPosition();
        
        Leap::Matrix fingerTransform;
        
        gl::color( ColorAf::gray( 0.8f ) );
        
#ifdef __BONES__
        
        gl::drawSphere(LeapMotion::toVec3f(fingerMcpPos), 10);
        gl::drawSphere(LeapMotion::toVec3f(fingerPipPos), 10);
        gl::drawSphere(LeapMotion::toVec3f(fingerDipPos), 10);
        gl::drawSphere(LeapMotion::toVec3f(fingerTipOrigin), 10);
        gl::drawLine(LeapMotion::toVec3f(fingerMcpPos), LeapMotion::toVec3f(fingerPipPos) );
        gl::drawLine(LeapMotion::toVec3f(fingerPipPos), LeapMotion::toVec3f(fingerDipPos) );
        gl::drawLine(LeapMotion::toVec3f(fingerDipPos), LeapMotion::toVec3f(fingerTipOrigin) );
        
        
        fingerTransform = tipBone.basis().rigidInverse();
        gl::pushMatrices();
        gl::translate(LeapMotion::toVec3f(fingerTipOrigin));
        gl::multModelView(LeapMotion::toMatrix44f(fingerTransform));
        gl::drawColorCube(Vec3f(0,0,0), Vec3f( 10, 10, tipBone.length() ) );
        gl::popMatrices();
        
        fingerTransform = pipBone.basis().rigidInverse();
        gl::pushMatrices();
        gl::translate(LeapMotion::toVec3f(fingerPipPos));
        gl::multModelView(LeapMotion::toMatrix44f(fingerTransform));
        gl::drawColorCube(Vec3f(0,0,0), Vec3f( 10, 10, pipBone.length() ) );
        gl::popMatrices();
        
        fingerTransform = dipBone.basis().rigidInverse();
        gl::pushMatrices();
        gl::translate(LeapMotion::toVec3f(fingerDipPos));
        gl::multModelView(LeapMotion::toMatrix44f(fingerTransform));
        gl::drawColorCube(Vec3f(0,0,0), Vec3f( 10, 10, dipBone.length() ) );
        gl::popMatrices();
        
#else
        float mag = finger.direction().dot(-hand.palmNormal());
        Leap::Vector right = finger.direction().cross(-hand.palmNormal());
        if (mag < -0.45 || mag > 0.45)
            right = finger.direction().cross(hand.direction());
        
        right = right.normalized();
        Leap::Vector down = finger.direction().cross(right).normalized();
        Leap::Vector up = -down;
        
        fingerTransform = Leap::Matrix(right, up, finger.direction()).rigidInverse();
        gl::pushMatrices();
        gl::translate(LeapMotion::toVec3f(fingerTipOrigin*0.5));
        gl::multModelView(LeapMotion::toMatrix44f(fingerTransform));
        gl::drawColorCube(Vec3f(0,0,0), Vec3f( 10, 10, finger.length() ) );
        gl::popMatrices();
#endif
        
    }
    if (mStaticOrientHand)
        gl::popMatrices();
}

void LeapMotionMain::drawHands()
{
    gl::setMatrices( mCamera );
    
    gl::translate(mTranslate);
    gl::rotate(mObjOrientation);
    float headLength = 6.0f;
    float headRadius = 3.0f;
    // Iterate through hands
    const Leap::HandList& hands = mFrame.hands();
    for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
        
        Leap::Hand hand = *handIter;
        
        
        // Get hand data
        Vec3f handDir		= LeapMotion::toVec3f( hand.direction() );
        Vec3f palmNorm		= LeapMotion::toVec3f( hand.palmNormal() );
        Vec3f palmPos		= LeapMotion::toVec3f( hand.palmPosition() );
        Vec3f palmVel		= LeapMotion::toVec3f( hand.palmVelocity() );
        Vec3f sphereCenter	= LeapMotion::toVec3f( hand.sphereCenter() );
        float sphereRadius	= hand.sphereRadius();
        
//        // Hand sphere
//        gl::enableWireframe();
//        gl::color( ColorAf( Colorf::gray( 0.9f ), 0.5f ) );
//        gl::drawSphere( sphereCenter, sphereRadius, 16 );
//        gl::disableWireframe();
//        
//        // Hand plane
//        gl::color( ColorAf( 0.75f, 0.0f, 0.75f, 0.25f ) );
//        gl::pushMatrices();
//        gl::translate( palmPos );
//        gl::rotate( Quatf( palmPos, handDir ) );
//        for ( float i = 0.25f; i <= 1.0f; i += 0.25f ) {
//            gl::drawStrokedCircle( Vec2f::zero(), sphereRadius * i, 16 );
//        }
//        gl::popMatrices();
//        
//        // Hand direction
//        gl::color( 1.0f, 0.0f, 1.0f, 0.5f );
//        gl::drawVector( palmPos, palmPos + handDir * 30.0f, headLength, headRadius );
        
        // Hand normal
        gl::color( 0.0f, 0.0f, 1.0f, 0.5f );
        gl::drawVector( palmPos, palmPos + palmNorm * 30.0f, headLength, headRadius );
        
//        // Hand velocity
//        gl::color( 0.0f, 1.0f, 0.0f, 0.5f );
//        gl::drawVector( palmPos, palmPos + palmVel * 0.05f, headLength, headRadius );

        
        
        gl::pushMatrices();
        
        if (mStaticPosHand)
        {
            if (hands.count() > 1)
            {
                if (hand.id() == hands.leftmost().id())
                    gl::translate(Vec3f(-100,0,0)*mScale);
                else
                    gl::translate(Vec3f(100,0,0)*mScale);
            }
        }
        else
            gl::translate(LeapMotion::toVec3f(hand.palmPosition()*mScale));
        
        drawHand(hand, Vec3f(0,0,0));
        
        gl::popMatrices();
    }
}

void LeapMotionMain::draw()
{
    // Clear window
    gl::setViewport( getWindowBounds() );
    gl::clear( ColorA(0, 0, 0, 0.0) );
    
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    gl::pushMatrices();
    
    drawHands();
    
    gl::popMatrices();
    glPopAttrib();
    
    gl::color(ColorA(0, 0, 0,1));
    if (!showParams)
        return;
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    mParams->draw();
    glPopAttrib();
}

void LeapMotionMain::update()
{
    mLeap->update();
    //std::cout << "frame" <<std::endl;
}

void LeapMotionMain::onFrame( Leap::Frame frame )
{
    mFrame = frame;
    
    //std::cout << "calback" <<std::endl;
    
    
    if (!frame.hands().isEmpty()) {
        // Get the first hand
        const Hand hand = frame.hands()[0];
        const Hand hand2 = frame.hands()[1];
        
        // Get the hand's normal vector and direction
        const Vector normal = hand.palmNormal();
        const Vector direction = hand.direction();
        const Vector position = hand.palmPosition();
        
        const Vector normal2 = hand2.palmNormal();
        const Vector direction2 = hand2.direction();
        const Vector position2 = hand2.palmPosition();
    }
}

void LeapMotionMain::setupGui()
{
    //         std::cout << "Run here" <<std::endl;
        mParams = cinder::params::InterfaceGl::create("Parameters", Vec2i(200,300));
        mParams->addParam ("Scene Rotation", &mObjOrientation);
        mParams->addParam ("Hand Translation", &mTranslate);
        mParams->addParam ("Hand Scale", &mScale);
        mParams->addParam ("Static Hand Orientation", &mStaticOrientHand);
        mParams->addParam ("Static Hand Position", &mStaticPosHand);
    
        TwDefine(" Parameters color='0 128 255' alpha=255 ");
        TwDefine(" Parameters/'Scene Rotation' opened=true ");
        TwDefine(" Parameters/'Hand Translation' opened=true ");
}


void LeapMotionMain::setup()
{
    
    mStaticOrientHand = NO;
    mStaticPosHand = NO;
    
    mScale = 1;
    mTranslate.set(0,200,0);
    
    showParams = TRUE;
    
    
    
    // Set up OpenGL
    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    // Set up camera
    mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 5000.0f );
    mCamera.lookAt( Vec3f( 0.0f, 500.0f, 500.0f ), Vec3f( 0.0f, 250.0f, 0.0f ) );
    
//    // Start device
    mLeap = LeapMotion::Device::create();
    mLeap->connectEventHandler( &LeapMotionMain::onFrame, this );
    setupGui();
}

void LeapMotionMain::keyDown( KeyEvent event )
{
    if(event.getChar() == 'g')
    {
        showParams = !showParams;
    }
}



CINDER_APP_BASIC( LeapMotionMain, RendererGl )


