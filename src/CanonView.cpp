//
//  CanonView.cpp
//  LeapMotion
//
//  Created by Alvin Sun on 2015-03-15.
//
//
#include "../include/CanonView.h"

using namespace std;
using namespace ci;

CanonViewRef CanonView::create(CanonView::Settings settings){
	return CanonViewRef(new CanonView(settings));
}

CanonView::CanonView(Settings settings):mSettings(settings){
	if (mSettings.getWindow()){
		connectWindow(mSettings.getWindow());
	}
}

void CanonView::connectWindow(app::WindowRef window){
	window->getSignalPostDraw().connect(std::bind(&CanonView::draw, this));
	app::App::get()->getSignalUpdate().connect(std::bind(&CanonView::update, this));
}

void CanonView::draw(){

}

void CanonView::update(){
	/*_RPT1(_CRT_WARN, "I have go here%d",mSettings.getWindow());*/
}