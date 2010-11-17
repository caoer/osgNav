/*************** <auto-copyright.pl BEGIN do not edit this line> **************
*
* VR Juggler is (C) Copyright 1998-2007 by Iowa State University
*
* Original Authors:
*   Allen Bierbaum, Christopher Just,
*   Patrick Hartling, Kevin Meinert,
*   Carolina Cruz-Neira, Albert Baker
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
*************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _OSG_NAV_
#define _OSG_NAV_

#include <vrj/vrjConfig.h>

#include <iostream>
#include <iomanip>
#include <math.h>

#include <vrj/Draw/OGL/GlApp.h>

#include <gadget/Type/PositionInterface.h>
#include <gadget/Type/AnalogInterface.h>
#include <gadget/Type/DigitalInterface.h>

//OSG  includes
#include <osg/Matrix>
#include <osg/Transform>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>

#include <osgUtil/SceneView>

#include <osgText/Text>
#include <osgText/Font>

#include "VTP_File_Reader.h"

#ifdef TWEEK_HAVE_CXX
#include <tweek/CORBA/CorbaManager.h>
#endif

#include "nav.h"

#include "OsgApp.h"
#include <vector>
//#include <vrj/Draw/OSG/OsgApp.h>


/**
* Demonstration Open Scene Graph application class.
*/



class OsgNav : public vrj::OsgApp, public VTP_File_Reader
{
public:
	 	//flowing vector parameters
	osg::ref_ptr<osg::Cone>								cone;
	osg::ref_ptr<osg::Cylinder>							tail;
	vector< vector< osg::ref_ptr<osg::ShapeDrawable> > >	vectorDrawable;
	vector< vector< osg::ref_ptr<osg::Geode> > >			vectorGeode;
	vector< osg::ref_ptr<osg::MatrixTransform> > 			vectorTransform;
	OsgNav(vrj::Kernel* kern, int& argc, char** argv);

	virtual ~OsgNav()
	{
		/* Do nothing. */ ;
	}

	/**
	* Execute any initialization needed before the API is started.
	*
	* This is called once before OSG is initialized.
	*/
	virtual void initScene();

	void myInit();
	void myPrint(int x) {
		printf("%d",x);
	}
	float getDistance(float ax, float ay, float az, float bx, float by, float bz)
	{
		float distance=0;
		distance = sqrt(pow((ax-bx),2)+pow((ay-by),2)+pow((az-bz),2));
		return distance;
	}

	void initTweek(int& argc, char* argv[]);

	virtual osg::Group* getScene()
	{
		return mRootNode.get();
	}

	virtual void configSceneView(osgUtil::SceneView* newSceneViewer)
	{
		vrj::OsgApp::configSceneView(newSceneViewer);

		newSceneViewer->getLight()->setAmbient(osg::Vec4(0.3f,0.3f,0.3f,1.0f));
		newSceneViewer->getLight()->setDiffuse(osg::Vec4(0.9f,0.9f,0.9f,1.0f));
		newSceneViewer->getLight()->setSpecular(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
		newSceneViewer->setLightingMode(osgUtil::SceneView::SKY_LIGHT);
	}

	void bufferPreDraw();

	// ----- Drawing Loop Functions ------
	//
	//  The drawing loop will look similar to this:
	//
	//  while (drawing)
	//  {
	//        preFrame();
	//       <Application Data Syncronization>
	//        latePreFrame();
	//       draw();
	//        intraFrame();     // Drawing is happening while here
	//       sync();
	//        postFrame();      // Drawing is now done
	//
	//       UpdateTrackers();
	//  }
	//------------------------------------

	/**
	* Function called after tracker update but before start of drawing.
	*
	* Called once before every frame.
	*/
	virtual void preFrame();

	/** Function called after ApplicationData syncronization but before draw() */
	virtual void latePreFrame();

	/**
	* Function called after drawing has been triggered but BEFORE it
	* completes.
	*
	* Called once during each frame.
	*/
	virtual void intraFrame()
	{
		// Put your intra frame computations here.
	}

	/**
	* Function called before updating trackers but after the frame is drawn.
	*
	* Called once after every frame.
	*/
	virtual void postFrame()
	{
		// Put your post frame computations here.
	}

	//MultiFiles - code for loading in multiple .osg files
	void setModelFileName(std::string filename)
	{
		FILE *filp;
		char buf[256];
		std::cout << "control file: " << filename << std::endl << std::flush;
		if ((filp = fopen(filename.c_str(), "r"))) 
		{
			do
			{
				buf[0] = 0;
				fgets(buf, 256, filp);
				if (buf[0] != '\n' && buf[0] != 0)
				{
					buf[strlen(buf)-1] = 0;
					std::cout << "file: " << buf << std::endl << std::flush;
					mFilesToLoad.push_back(std::string(buf));
				}
			}
			while (buf[0]);
			fclose(filp);
		}
		else
		{
			std::cerr << "Couldn't open " << filename << std::endl << std::flush;
		}
	}

	void updateVectorColor(int index)
	{
		std::cout<<"change color"<<endl;
		for(int i=0;i<2;i++)
			vectorDrawable[index][i]->setColor(osg::Vec4(color[flag_current[index]].r,color[flag_current[index]].g,color[flag_current[index]].b,1.0f));
	}


	void initVectors()
	{
		
		int index = 0;
		cone= new osg::Cone(osg::Vec3(0,0,0),0.005*myScale*FVSizeScale,0.04*myScale*FVSizeScale);
		cone->setRotation(osg::Quat(gmtl::Math::deg2Rad(90.0f),osg::Vec3f(0.0,1.0,0)));
		tail= new osg::Cylinder(osg::Vec3(-0.025*myScale*FVSizeScale,0,0),0.0025*myScale*FVSizeScale,0.035*myScale*FVSizeScale);
		tail->setRotation(osg::Quat(gmtl::Math::deg2Rad(90.0f),osg::Vec3f(0.0,1.0,0)));

		for(int i=0;i<NUM_Lines;i++)
		{
			int start_index = (i==0)?0:offsets[i-1];
			int end_index = offsets[i];
			int temp_lenght = end_index-start_index;
			if(temp_lenght>=MIN_Points&&temp_lenght<=MAX_Points)//sample of streamlines
			{
				for(int j=start_index;j<end_index;j+=distance)
				{
					//drawable
					vector<osg::ref_ptr<osg::ShapeDrawable> > temp_Drawable;
					osg::ref_ptr<osg::ShapeDrawable> cone_Drawable = new osg::ShapeDrawable(cone.get());
					cone_Drawable->setColor(osg::Vec4(R,G,B,A));
					temp_Drawable.push_back(cone_Drawable);
					osg::ref_ptr<osg::ShapeDrawable> tail_Drawable = new osg::ShapeDrawable(tail.get());
					tail_Drawable->setColor(osg::Vec4(R,G,B,A));
					temp_Drawable.push_back(tail_Drawable);
					vectorDrawable.push_back(temp_Drawable);

					//geode
					vector<osg::ref_ptr<osg::Geode> > temp_Geode;
					osg::ref_ptr<osg::Geode> cone_Geode = new osg::Geode();
					cone_Geode->addDrawable(cone_Drawable.get());
					cone_Geode->setCullingActive(false);
					temp_Geode.push_back(cone_Geode);
					osg::ref_ptr<osg::Geode> tail_Geode = new osg::Geode();
					tail_Geode->addDrawable(tail_Drawable.get());
					tail_Geode->setCullingActive(false);
					temp_Geode.push_back(tail_Geode);
					vectorGeode.push_back(temp_Geode);

					//transform
					
					osg::ref_ptr<osg::MatrixTransform> temp_Transform = new osg::MatrixTransform();;
					temp_Transform->addChild(temp_Geode[0].get());
					temp_Transform->addChild(temp_Geode[1].get());
					temp_Transform->setMatrix(osg::Matrix::scale(0.00001,0.00001,0.00001));
					vectorTransform.push_back(temp_Transform);
					mNavTrans->addChild(temp_Transform.get());
					
					




					////cone
					//vectorDrawable[index][0] = new osg::ShapeDrawable(cone.get());
					//vectorDrawable[index][0]->setColor(osg::Vec4(0.6f,0.6f,0.8f,1.0f));
					//vectorGeode[index][0] = new osg::Geode();
					//vectorGeode[index][0]->addDrawable(vectorDrawable[index][0].get());
					////tail
					//vectorDrawable[index][1] = new osg::ShapeDrawable(tail.get());
					//vectorDrawable[index][1]->setColor(osg::Vec4(0.6f,0.6f,0.8f,1.0f));
					//vectorGeode[index][1] = new osg::Geode();
					//vectorGeode[index][1]->addDrawable(vectorDrawable[index][1].get());
					////transform
					//vectorTransform[index] = new osg::MatrixTransform();
					//vectorTransform[index]->addChild(vectorGeode[index][0].get());
					//vectorTransform[index]->addChild(vectorGeode[index][1].get());
					//mNavTrans->addChild(vectorTransform[index].get());
					//vectorTransform[index]->setMatrix(osg::Matrix::scale(0,0,0));

					//index++;//next vector
				}
			}
		}
	}



private:

	string config_file_name;
	string vtp_list_file_name;

	osg::ref_ptr<osg::Group>             mRootNode;
	osg::ref_ptr<osg::Group>             mNoNav;
	osg::ref_ptr<osg::MatrixTransform>   mNavTrans;
	osg::ref_ptr<osg::MatrixTransform>   mModelTrans;
	osg::ref_ptr<osg::MatrixTransform>   mColorBarTrans;
	osg::ref_ptr<osg::MatrixTransform>   mTableTrans1;
	osg::ref_ptr<osg::MatrixTransform>   mAxisTrans;
	std::vector< osg::ref_ptr<osg::Node> > mModels;
	osg::ref_ptr<osg::Switch>            mSwitch;
	osg::ref_ptr<osg::MatrixTransform>   hudTrans;


// 	//flowing vector parameters --moved to public by zitao
// 	osg::ref_ptr<osg::Cone>								cone;
// 	osg::ref_ptr<osg::Cylinder>							tail;
// 	vector< vector< osg::ref_ptr<osg::ShapeDrawable> > >	vectorDrawable;
// 	vector< vector< osg::ref_ptr<osg::Geode> > >			vectorGeode;
// 	vector< osg::ref_ptr<osg::MatrixTransform> > 			vectorTransform;


	OsgNavigator  mNavigator;       /**< Navigation class */

	std::vector<std::string> mFilesToLoad; //MultiFiles

	unsigned int currentModel;//for keeping track of the current model being shown with mSwitch

	/** Time of the start of the last preframe */
	vpr::Interval mLastPreFrameTime;

#ifdef TWEEK_HAVE_CXX
	tweek::CorbaManager mCorbaManager;
#endif

public:
	gadget::PositionInterface  mWand;     /**< the wand */
	gadget::PositionInterface  mHead;     /**< the head */
	gadget::DigitalInterface   mButton0;
	gadget::DigitalInterface   mButton1;
	gadget::DigitalInterface   mButton2;
	gadget::DigitalInterface   mButton3;
	gadget::DigitalInterface   mButton4;
	gadget::DigitalInterface   mButton5;
	
	gadget::AnalogInterface mAnalog0;	
	gadget::AnalogInterface mAnalog1;
};


#endif
