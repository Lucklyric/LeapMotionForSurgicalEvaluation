//
//  CanonView.h
//  LeapMotion
//
//  Created by Alvin Sun on 2015-03-15.
//
//
#ifndef _CANONVIEW_ 
#define _CANONVIEW_

#include "cinder/app/AppBasic.h"
#include "cinder/app/AppNative.h"

typedef std::shared_ptr<class CanonView> CanonViewRef;

class CanonView{
public:
	struct Settings{
	public:
		Settings():mHeight(500.0f), mWidth(500.0f){}
		
		ci::app::WindowRef getWindow(){ return mWindow; }
		void setWindow(ci::app::WindowRef window){ mWindow = window; }
		Settings& window(ci::app::WindowRef window){ setWindow(window); return *this; }

		float getHeight(){ return mHeight; }
		void setHeight(float height){ mHeight = height; }
		Settings& height(float height){ setHeight(height); return *this; }

		float getWidth(){ return mWidth; }
		void setWidth(float width){ mWidth = width; }
		Settings& width(float width){ setWidth(width); return *this; }

	private:
		float mHeight;
		float mWidth;
		ci::app::WindowRef mWindow;
	};

	static CanonViewRef create(Settings settings = Settings());
	void update();
	void draw();

//protected:
	CanonView(Settings settings = Settings());

	//void setup();
	void connectWindow(ci::app::WindowRef window);

private:
	Settings mSettings;
};

#endif // !_CANONVIEW_ 