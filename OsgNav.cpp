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

//#include <vrj/vrjConfig.h>

#include "OsgNav.h"

//#include <osg/Math>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Vec3>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include <gmtl/Vec.h>
#include <gmtl/Coord.h>
#include <gmtl/Xforms.h>
#include <gmtl/Math.h>

#include "VTK_bin_readwrite.h"
#include "time.h"
#include "stdio.h"

const int dataMode=0;//0-read ascii data, 1-read binary data, 2-convert ascii to binary
//static osg::Quat *quat=new osg::Quat();
//static osg::Vec3f *axisVec=new osg::Vec3f(0.0,0.0,1.0);
//static osg::Vec3f *newVec=new osg::Vec3f(1.0,0.0,0.0);
static int arrowOn=0;
static int arrowDelay=1;
//static double tmp;

/*record and playback*/
static vector<gmtl::Matrix44f> tourMatrix; //record tour matrix
static vector<gmtl::Matrix44f> tempMatrix; //record temp Matrix
static bool recordPath=0;
static bool playPath=0;
static int pathCounter=0;
//static double tempAnalog;
static clock_t cycleButton_timeoffset,FVButton_timeoffset,time_start,time_pause, time_release;
static int frameCounter=0;
static int firstCycleButtonPressed = 1;
static int firstFVButtonPressed = 1;
static vector<clock_t> CB_timeoffsets,FVB_timeoffsets;
static vector <int> modleNumbers, FVButtons;
static int write_counter=0;
static int read_counter=0;
static int isPause = 0;
static int currentModelID;
static int currentFVStatus;


#ifdef USE_REMOTE_NAV
#   include <remotenav/Subject/RemoteNavSubjectImpl.h>
#endif

OsgNav::OsgNav(vrj::Kernel* kern, int& argc, char** argv) : vrj::OsgApp(kern)
{
	//mFileToLoad = std::string("");

#ifdef USE_REMOTE_NAV
	// Initialize tweek and register RemoteNavSubjectImpl with the nameserver.
	initTweek( argc, argv );
#endif
}

void OsgNav::latePreFrame()
{
	//cout<<"In latePreFrame"<<endl;
	gmtl::Matrix44f world_transform;
	gmtl::invertFull(world_transform, mNavigator.getCurPos());
	// Update the scene graph
	osg::Matrix osg_current_matrix;
	osg_current_matrix.set(world_transform.getData());
	mNavTrans->setMatrix(osg_current_matrix);

	// Finish updating the scene graph.
	vrj::OsgApp::latePreFrame();
}

void OsgNav::preFrame()
{
	//cout<<"In preFrame"<<endl;
	static bool firstFrame=1;
	frameCounter++;
	arrowDelay++;

	if(firstFrame)
	{	
		//init_VTP();
		//read_List_File();
		initFlags();
		initVectors();
		cout<<"NUM_VEctors: "<<vectorTransform.size()<<endl;
		
		time_start = clock();
	}

	vpr::Interval cur_time = mWand->getTimeStamp();
	vpr::Interval diff_time(cur_time-mLastPreFrameTime);
	if (mLastPreFrameTime.getBaseVal() >= cur_time.getBaseVal())
	{  diff_time.secf(0.0f); }

	float time_delta = diff_time.secf();
	mLastPreFrameTime = cur_time;

	// Get wand data
	gmtl::Matrix44f wandMatrix = mWand->getData();      // Get the wand matrix
	static gmtl::Matrix44f tmpWandMatrix = gmtl::Matrix44f();
	static gmtl::Matrix44f tmpUserMatrix = gmtl::Matrix44f();
	const int delay = 4; //animation speed (number of frames between cycler
	static int delayCounter = 0; //increment each frame, reset when delay is reached


	if ( mButton4->getData() == gadget::Digital::TOGGLE_ON)
	{
		if(isPause == 0)
		{
			isPause = 1;
			time_pause = clock();
			currentModelID = currentModel;
			currentFVStatus = arrowOn;
		}
		else
		{
			isPause = 0;
			time_release = clock();
			cout<<"old time_start:"<<time_start;
			time_start += time_release - time_pause;
			cout<<"; puase_time:"<<time_pause<<"release time:"<<time_release<<"new time_start:"<<time_start<<endl;
			currentModel = currentModelID;
			arrowOn = currentFVStatus;
			mSwitch->setSingleChildOn(currentModel);
		}
	}

	/***************
	regular driving
	****************/
	if(!playPath||isPause)
	{
		if ( mButton0->getData() == gadget::Digital::ON)	//forwading or backforwading
		{
			if ( mButton5->getData() == gadget::Digital::ON ) //acceleration when button 5 is pressed at the same time
			{
				gmtl::Vec3f direction;
				gmtl::Vec3f Zdir = gmtl::Vec3f(0.0f, 0.0f, -fast_Speed);
				gmtl::xform(direction, wandMatrix, Zdir);
				mNavigator.setVelocity(direction);
			}  // Make sure to reset the velocity when we stop pressing the button.
			else
			{
				gmtl::Vec3f direction;
				gmtl::Vec3f Zdir = gmtl::Vec3f(0.0f, 0.0f, -init_Speed);
				gmtl::xform(direction, wandMatrix, Zdir);
				mNavigator.setVelocity(direction);
			}  // Make sure to reset the velocity when we stop pressing the button.
		}
		else if ( mButton0->getData() == gadget::Digital::TOGGLE_OFF)
			mNavigator.setVelocity(gmtl::Vec3f(0.0, 0.0, 0.0));


		if ( mButton1->getData() == gadget::Digital::TOGGLE_ON) //cycle backward
		{
			delayCounter=0; 
			if(currentModel>0) currentModel--;else currentModel=(mFilesToLoad.size()-1);
			std::cout<<"currentModel: "<<currentModel<<"/"<<mFilesToLoad.size()<<std::endl;
			mSwitch->setSingleChildOn(currentModel);
		}

		if ( mButton2->getData() == gadget::Digital::ON) //rotate
		{
			cout<<"rotating..."<<endl;
			gmtl::EulerAngleXYZf euler(gmtl::makeXRot(mWand->getData()), gmtl::makeYRot(mWand->getData()), gmtl::makeZRot(mWand->getData()));
			gmtl::Matrix44f rot_mat = gmtl::makeRot<gmtl::Matrix44f>(euler);
			mNavigator.setRotationalVelocity(rot_mat);
		}
		else if(mButton2->getData() == gadget::Digital::TOGGLE_OFF) //stop rotating
			mNavigator.setRotationalVelocity(gmtl::Matrix44f());


		if ( mButton3->getData() == gadget::Digital::TOGGLE_ON) //cycle forward
		{
			if(currentModel<(mFilesToLoad.size()-1)) currentModel++;else currentModel=0;
			std::cout<<"currentModel: "<<currentModel<<"/"<<mFilesToLoad.size()<<std::endl;
			mSwitch->setSingleChildOn(currentModel);
		}

		/*********************************************************************************
		write current model number and clock offset time(clocktime-start time) to file
		**********************************************************************************/
		if(recordPath&&(mButton1->getData()==gadget::Digital::TOGGLE_ON||mButton3->getData()==gadget::Digital::TOGGLE_ON)&&!isPause) 
		{
			cycleButton_timeoffset = clock() - time_start;
			ofstream tmpFile;
			if(firstCycleButtonPressed&&!isPause)	
			{
				tmpFile.open("cycleButton.txt");
				firstCycleButtonPressed = 0;
			}
			else		
				tmpFile.open("cycleButton.txt",ios::app);
			tmpFile<<cycleButton_timeoffset<<" "<<currentModel<<endl;   
			tmpFile.close();
		}
		
		if ( (mButton5->getData() == gadget::Digital::TOGGLE_ON)&&(mButton0->getData() == gadget::Digital::OFF))
		{
			arrowOn++;
			arrowOn = arrowOn%3;

			/*write FV button information to file*/
			if(recordPath&&!isPause)
			{
				FVButton_timeoffset = clock() - time_start;
				ofstream tmpFile;
				if(firstFVButtonPressed)	
				{
					tmpFile.open("FVButton.txt");
					firstFVButtonPressed = 0;
				}
				else		
					tmpFile.open("FVButton.txt",ios::app);
				tmpFile<<FVButton_timeoffset<<" "<<arrowOn<<endl;
				tmpFile.close();
			}
		}
	}

	/**************************************
	Flowing Vector running when arrowOn=1
	***************************************/
	if(arrowOn == 1 && arrowDelay%Speed==0)
	{
		cout<<"Number of vectors:"<<vectorTransform.size()<<endl;
		for(unsigned int i=0;i<vectorTransform.size();i++)
		{
			if(temperature_open==1||velocity_open==1) updateVectorColor(i);
			{
				cout<<"Vecotr "<<i<<": x="<<points[flag_current[i]][0]<<", y="<<points[flag_current[i]][1]<<", z="<<points[flag_current[i]][2]<<endl;
				vectorTransform[i]->setMatrix(osg::Matrix::translate(points[flag_current[i]][0],points[flag_current[i]][1],points[flag_current[i]][2]));
			}
			/*
			if(flag_next[i]!=start[i])
			{				
				newVec->set(points[flag_next[i]][0]-points[flag_current[i]][0],points[flag_next[i]][1]-points[flag_current[i]][1],points[flag_next[i]][2]-points[flag_current[i]][2]);
				quat->makeRotate(osg::Vec3f(1.0,0.0,0.0),*newVec);
				double tmp = 0;
				quat->getRotate(tmp,*axisVec);
				vectorTransform[i]->preMult(osg::Matrix::rotate(tmp,axisVec->x(),axisVec->y(),axisVec->z()));
			}
			else
			{
				newVec->set(points[flag_current[i]][0]-points[flag_previous[i]][0],points[flag_current[i]][1]-points[flag_previous[i]][1],points[flag_current[i]][2]-points[flag_previous[i]][2]);
				quat->makeRotate(osg::Vec3f(1.0,0.0,0.0),*newVec);
				double tmp = 0;
				quat->getRotate(tmp,*axisVec);
				vectorTransform[i]->preMult(osg::Matrix::rotate(tmp,axisVec->x(),axisVec->y(),axisVec->z()));
			}
			*/
			cout<<"vector "<<i<<"'s position changed"<<endl;
			
			osg::Quat *quat = new osg::Quat();
			osg::Vec3f *newVec=new osg::Vec3f(1.0,0.0,0.0);
			if(flag_next[i]!=start[i])
			{	
				
				newVec->set(points[flag_next[i]][0]-points[flag_current[i]][0],points[flag_next[i]][1]-points[flag_current[i]][1],points[flag_next[i]][2]-points[flag_current[i]][2]);
				newVec->normalize();
				quat->makeRotate(osg::Vec3f(1.0,0.0,0.0),*newVec);
				//double tmp = 0;
				//quat->getRotate(tmp,*axisVec);
				vectorTransform[i]->preMult(osg::Matrix::rotate(*quat));
				//vectorTransform[i]->preMult(osg::Matrix::rotate(tmp,axisVec->x(),axisVec->y(),axisVec->z()));
			}
			else
			{
				newVec->set(points[flag_current[i]][0]-points[flag_previous[i]][0],points[flag_current[i]][1]-points[flag_previous[i]][1],points[flag_current[i]][2]-points[flag_previous[i]][2]);
				newVec->normalize();
				quat->makeRotate(osg::Vec3f(1.0,0.0,0.0),*newVec);
				//double tmp = 0;
				//quat->getRotate(tmp,*axisVec);
				vectorTransform[i]->preMult(osg::Matrix::rotate(*quat));
				//vectorTransform[i]->preMult(osg::Matrix::rotate(tmp,axisVec->x(),axisVec->y(),axisVec->z()));
			}
			
			cout<<"vecotr "<<i<<" done"<<endl;
		}
		cout<<"done before flags"<<endl;
		updataFlags();
		cout<<"done after flags"<<endl;
	}

	/*************
	 FV disapear
	*************/
	if(arrowOn == 0)
	{
		for(unsigned int i=0;i<vectorTransform.size();i++)
			vectorTransform[i]->setMatrix(osg::Matrix::scale(0.00001,0.00001,0.00001));
	}

	/*****************************
	 When button 4 is pushed left
	******************************/	
	if(mAnalog0->getData()<0.4)
	{
		//cout<<"start to record........"<<endl;
		recordPath = 1;
		time_start = clock();
	}
	
	/***************************************************************
	 Write tour matrix to tourMatrx.txt when button 4 is pushed down
	****************************************************************/
	if(mAnalog0->getData()>0.6)
	{
		//std::cout<<"writing to file..."<<std::endl;
		write_counter++;
		std::ofstream tourMatrixFile;
		if(write_counter)
		{
			tourMatrixFile.open ("tourMatrx.txt");
			for (unsigned int i=0;i<tourMatrix.size();i++)
				tourMatrixFile<<tourMatrix[i][0][0]<<" "<<tourMatrix[i][0][1]<<" "<<tourMatrix[i][0][2]<<" "<<tourMatrix[i][0][3]<<" "<<tourMatrix[i][1][0]<<" "<<tourMatrix[i][1][1]<<" "<<tourMatrix[i][1][2]<<" "<<tourMatrix[i][1][3]<<" "<<tourMatrix[i][2][0]<<" "<<tourMatrix[i][2][1]<<" "<<tourMatrix[i][2][2]<<" "<<tourMatrix[i][2][3]<<" "<<tourMatrix[i][3][0]<<" "<<tourMatrix[i][3][1]<<" "<<tourMatrix[i][3][2]<<" "<<tourMatrix[i][3][3]<<" ";
				
			tourMatrixFile.close();
			recordPath=0;
			std::cout<<"tour matrix stored"<<std::endl;
		}
	}

	/***************************************************************
		Reading from files for playback(when button 4 is pushed down)
	****************************************************************/	
	if(mAnalog1->getData()<0.4)
	{
		read_counter++;
		pathCounter=0;
		gmtl::Matrix44f tempMatrix;
		string str;
		vector<double> tempValues;
		double temp;
		clock_t tmpClock;
		int tmpModel;
		if(read_counter==1)	//read only for the firt trigger of this button
		{
			std::cout<<"reading from files........"<<std::endl;

			/*read tourMatrix from file*/
			fstream tmpFile("tourMatrx.txt");
			if(!tmpFile)
				cout<<"can't open this file"<<endl;
			else
			{
				//cout<<"getting data from file"<<endl;
				tourMatrix.clear();
				while (tmpFile)
				{
					if(getline(tmpFile,str))
					{
						istringstream stream(str);
						while(stream>>temp)
							tempValues.push_back(temp);
					}
				}
				
				for(unsigned int m=0;m<tempValues.size();m+=16)
				{			
					for(int i=0;i<4;i++)
						for(int j=0;j<4;j++)
							tempMatrix[i][j] = tempValues[m+4*i+j];
						
					tourMatrix.push_back(tempMatrix);
				}
				tmpFile.close();
			}

			/*reading CB_timeoffsets and model_Numbers from CycleButton.txt file*/
			ifstream CB_tmpFile("cycleButton.txt");
			CB_timeoffsets.clear();
			modleNumbers.clear();
			int tt=0;
			while(CB_tmpFile)
			{
				if(getline(CB_tmpFile,str))
				{
					istringstream stream(str);
					stream>>tmpClock;
					stream>>tmpModel;
					CB_timeoffsets.push_back(tmpClock);
					modleNumbers.push_back(tmpModel);
				}
			}
			CB_tmpFile.close();

			/*reading FVB_timeoffsets and FV_buttons from FVButton.txt file*/
			ifstream FVB_tmpFile("FVButton.txt");
			FVB_timeoffsets.clear();
			FVButtons.clear();
			while(FVB_tmpFile)
			{
				if(getline(FVB_tmpFile,str))
				{
					istringstream stream(str);
					clock_t tmpClock;
					int tmpButton;
					stream>>tmpClock;
					stream>>tmpButton;
					FVB_timeoffsets.push_back(tmpClock);
					FVButtons.push_back(tmpButton);
				}			
			}
			FVB_tmpFile.close();
		}
	}
		
	/********************************************
	start to playback when button 4 is push up	
	*********************************************/
	if(mAnalog1->getData()>0.6)		
	{
		pathCounter=0;
		recordPath=0;
		playPath=1;
		arrowOn = 0;
		currentModel = 0;
		mSwitch->setSingleChildOn(currentModel);
		time_start = clock();
	}
	
	/**********************
	record tour path Matrix	
	***********************/	
	if (recordPath)
	{
		//cout<<"add navigator to tourMatrix"<<endl;
		tourMatrix.push_back(mNavigator.getCurPos());//set user's position to what we read from the file
	}		
	
	/*****************
		playback	
	*****************/	
	if (playPath&&!isPause) 
	{
		/*update the current Model*/
		for(unsigned int i=0;i<CB_timeoffsets.size();i++)
		{
			cout<<"CB current offset:"<<clock()-time_start<<"CB_timeoffsets["<<i<<"]:"<<CB_timeoffsets[i]<<endl;
			if(clock()-time_start >= CB_timeoffsets[i]-10000 && clock()-time_start <= CB_timeoffsets[i]+10000&& modleNumbers[i]!=currentModel)
			{
				currentModel = modleNumbers[i];
				mSwitch->setSingleChildOn(currentModel);
				cout<<"time_start:"<<time_start<<"; current offset:"<<clock()-time_start<<";current model:"<<modleNumbers[i]<<endl;
				break;
			}
		}
		
		/*update Flowing Vector Mode, 0-disapear; 1-running; 2-pause*/
		for(unsigned int i=0;i<FVB_timeoffsets.size();i++)
		{
			cout<<"FV current offset:"<<clock()-time_start<<"FVB_timeoffsets["<<i<<"]:"<<FVB_timeoffsets[i]<<endl;
			if(clock()-time_start >= FVB_timeoffsets[i]-10000 && clock()-time_start <= FVB_timeoffsets[i]+10000 && FVButtons[i]!=arrowOn)			
			{
				arrowOn = FVButtons[i];
				cout<<"time_start:"<<time_start<<"; current offset:"<<clock()-time_start<<";FV status:"<<arrowOn<<endl;
				break;
			}
		}
		
		/*update current Position when viewing*/
		mNavigator.setCurPos(tourMatrix[pathCounter]);
		pathCounter++;
		if(pathCounter>(int)tourMatrix.size())	//go to next roun when tour is finished 
		{
			pathCounter = 0;
			arrowOn = 0;
			currentModel = 0;
			mSwitch->setSingleChildOn(currentModel);
			time_start = clock();
		}
	}

	// Update the navigation using the time delta between
	mNavigator.update(time_delta);

	static bool cache=0;
	static int modelIndex=0;
	if ((mButton1->getData()==gadget::Digital::ON)&&(mButton3->getData()==gadget::Digital::ON)) //cache entire cycle
	{
		cache=1;
		modelIndex=currentModel;
		//for(int i=currentModel;i<(mFilesToLoad.size()-1);i++)
	}
	if (cache)
	{
		if(currentModel<(mFilesToLoad.size()-1)) currentModel++;else currentModel=0;
		mSwitch->setSingleChildOn(currentModel);
		std::cout<<"currentModel: "<<currentModel<<"/"<<mFilesToLoad.size()<<std::endl;
	}
	if(currentModel==modelIndex) 
	{
		cache=0;
		mSwitch->setSingleChildOn(modelIndex);
	}

	if (firstFrame) firstFrame =0;
}

void OsgNav::bufferPreDraw()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void OsgNav::initScene()
{
	cout<<"In initScene"<<endl;
	//DeBugOut = Debug::getStream(0, 3, true, true, 1, true);

	//std::cout << "---------- App:init() ---------------" << std::endl;
	// Initialize devices
	const std::string wand("VJWand");
	const std::string vjhead("VJHead");
	const std::string but0("VJButton0");
	const std::string but1("VJButton1");
	const std::string but2("VJButton2");
	const std::string but3("VJButton3");
	const std::string but4("VJButton4");
	const std::string but5("VJButton5");

	mWand.init(wand);
	mHead.init(vjhead);
	mButton0.init(but0);
	mButton1.init(but1);
	mButton2.init(but2);
	mButton3.init(but3);
	mButton4.init(but4);
	mButton5.init(but5);

	const std::string ana0("VJAnalog0");
	const std::string ana1("VJAnalog1");
	mAnalog0.init(ana0);
	mAnalog1.init(ana1);


	myInit();
}

void OsgNav::myInit()
{
	cout<<"in myInit"<<endl;
	//
	//          /-- mNoNav
	// mRootNode
	//          \-- mNavTrans -- mModelTrans -- mModel

	//The top level nodes of the tree
	mRootNode = new osg::Group();
	mNoNav    = new osg::Group();
	mNavTrans = new osg::MatrixTransform();

	currentModel = 0;//for mSwitch

	mNavigator.init();

	mRootNode->addChild(mNoNav.get());
	mRootNode->addChild(mNavTrans.get());


	// Transform node for the model
	mModelTrans  = new osg::MatrixTransform();
	//This can be used if the model orientation needs to change
	osg::Matrix m;
	m(0,0)=1.0;
	m(1,1)=0.0;
	m(2,2)=0.0;
	m(3,3)=1.0;
	m(1,2)=-1.0;
	m(2,1)=1.0;

	mModelTrans->setMatrix(m);
	//mModelTrans->setMatrix(osg::Matrix::translate(0,0,0));

	//mModelTrans->preMult(osg::Matrix::rotate(gmtl::Math::deg2Rad(-90.0f),1.0f, 0.0f, 0.0f));


	mModelTrans->preMult(osg::Matrix::scale(myScale,myScale,myScale));

	// Switch node for the models
	mSwitch = new osg::Switch();
	// Add the transform to the tree
	mModelTrans->addChild(mSwitch.get());
	mNavTrans->addChild(mModelTrans.get());

	// MultiFiles - This for loop will load the osg models from the list file provided
	for(unsigned int i = 0; i < mFilesToLoad.size(); ++i)
	{
		std::cout << "Attempting to load file: " << mFilesToLoad[i] << "... ";
		osg::ref_ptr<osg::Node> tmp = osgDB::readNodeFile(mFilesToLoad[i]);

		if ( ! tmp.valid() )
		{	
			std::cout << "ERROR: Could not load file: " << mFilesToLoad[i] << std::endl;
		}
		else
		{
			mModels.push_back(tmp);
			mSwitch->addChild(tmp.get());
			std::cout << "done." << std::endl;
		}
	}
	mSwitch->setAllChildrenOff();
	mSwitch->setSingleChildOn(currentModel);

	// run optimization over the scene graph
	osgUtil::Optimizer optimizer;
	optimizer.optimize(mRootNode.get());

	//user's initial position
	gmtl::Matrix44f initialPos = gmtl::Matrix44f();

	//translation
	initialPos[0][3]=init_X;//x
	initialPos[1][3]=init_Y;//y
	initialPos[2][3]=init_Z;//z

	mNavigator.setCurPos(initialPos);

}

void OsgNav::initTweek(int& argc, char* argv[])
{
#ifdef USE_REMOTE_NAV
	std::cout << "\n\nSTARTING TWEEK INITIALIZATION!!\n\n" << std::flush;

	std::string name_context( "OSG_REMOTE_NAV_");

	try
	{
		if ( mCorbaManager.init(name_context, argc, argv).success() )
		{
			try
			{
				if ( mCorbaManager.createSubjectManager().success() )
				{
					RemoteNavSubjectImpl* remote_nav_interface =
						new RemoteNavSubjectImpl(&mNavigator);

					mCorbaManager.getSubjectManager()->addInfoItem("OsgNav",
						"RemoteNav");
					try
					{
						mCorbaManager.getSubjectManager()->registerSubject(
							remote_nav_interface, "RemoteNavSubject"
							);
					}
					catch (...)
					{
						std::cout << "Failed to register subject" << std::endl;
					}
				}
			}
			catch (CORBA::Exception& ex)
			{
				std::cout
					<< "Caught an unknown CORBA exception when trying to register!"
					<< std::endl;
			}
		}

	}
	catch (...)
	{
		std::cout << "Caught an unknown exception while initializing Tweek!"
			<< std::endl;
	}

	std::cout << "\n\nDONE WITH TWEEK INITIALIZATION!!\n\n" << std::flush;
#else
	boost::ignore_unused_variable_warning(argc);
	boost::ignore_unused_variable_warning(argv);
#endif
}
