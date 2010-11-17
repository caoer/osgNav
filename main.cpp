/* -*-c++-*- 
 * 
 * osgART - ARToolKit for OpenSceneGraph
 * Copyright (C) 2005-2008 Human Interface Technology Laboratory New Zealand
 * 
 * This file is part of osgART 2.0
 *
 * osgART 2.0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * osgART 2.0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with osgART 2.0.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cstdlib>
#include <OsgNav.h>

// --- Lib Stuff --- //
#include <vrj/Kernel/Kernel.h>

//OSG  includes
#include <osg/Matrix>
#include <osg/Transform>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>

#include <osgART/Foundation>
#include <osgART/VideoLayer>
#include <osgART/PluginManager>
#include <osgART/VideoGeode>
#include <osgART/Utils>
#include <osgART/GeometryUtils>
#include <osgART/MarkerCallback>
#include <osgART/TransformFilterCallback>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/FileUtils>

osg::Group* createImageBackground(osg::Image* video) {
	osgART::VideoLayer* _layer = new osgART::VideoLayer();
	_layer->setSize(*video);
	osgART::VideoGeode* _geode = new osgART::VideoGeode(osgART::VideoGeode::USE_TEXTURE_2D, video);
	addTexturedQuad(*_geode,video->s(),video->t());
	_layer->addChild(_geode);
	return _layer;
}
void freeMemory(OsgNav* application) {
	free(application->offsets);
	for(int i=0; i< application->NUM_Points; i++)
	{
		free(application->points[i]);
	}
	free(application->points);
	free(application->temperature);
	free(application->velocity);
	application->flag_current.clear();
	application->flag_previous.clear();
	application->flag_next.clear();
	application->color.clear();

	delete application;
}

int main(int argc, char* argv[])  {

	vrj::Kernel* kernel = vrj::Kernel::instance();  // Get the kernel
	OsgNav* application = new OsgNav(kernel, argc, argv);             // Instantiate an instance of the app
	if ( argc <= 4 )
	{
		// display some usage info (holding the user by the hand stuff)
		//  this will probably go away once the kernel becomes separate
		//  and can load application plugins.
		std::cout<<"\n"<<std::flush;
		std::cout<<"\n"<<std::flush;

		std::cout << "Usage: " << argv[0]
		<< " modelname vjconfigfile[0] vjconfigfile[1] ... vjconfigfile[n]\n"
			<< std::endl << std::endl;

		std::exit(1);
	}

	application->setModelFileName(std::string(argv[1]));
	application->init_config(argv[2]);
	application->init_VTP();
	application->read_List_File(argv[3]);

	// Load any config files specified on the command line
	for ( int i = 4; i < argc; ++i)
	{
		kernel->loadConfigFile(argv[i]);
	}

	kernel->start();

	kernel->setApplication(application);

	kernel->waitForKernelStop();
	kernel->stop();
	
// 	application->initFlags();
// 	application->initVectors();






	

	// create a root node
	osg::ref_ptr<osg::Group> root = new osg::Group;

	osgViewer::Viewer viewer;
	
	// attach root node to the viewer
	viewer.setSceneData(root.get());

	// add relevant handlers to the viewer
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgViewer::ThreadingHandler);
	viewer.addEventHandler(new osgViewer::HelpHandler);


	// preload the video and tracker
	int _video_id = osgART::PluginManager::instance()->load("osgart_video_artoolkit2");
	//int _tracker_id = osgART::PluginManager::instance()->load("osgart_tracker_sstt");
	int _tracker_id = osgART::PluginManager::instance()->load("osgart_tracker_artoolkit2");

	// Load a video plugin.
	osg::ref_ptr<osgART::Video> video = 
		dynamic_cast<osgART::Video*>(osgART::PluginManager::instance()->get(_video_id));

	// check if an instance of the video stream could be started
	if (!video.valid()) 
	{   
		// Without video an AR application can not work. Quit if none found.
		osg::notify(osg::FATAL) << "Could not initialize video plugin!" << std::endl;
		exit(-1);
	}

	// Open the video. This will not yet start the video stream but will
	// get information about the format of the video which is essential
	// for the connected tracker
	video->open();

	osg::ref_ptr<osgART::Tracker> tracker = 
		dynamic_cast<osgART::Tracker*>(osgART::PluginManager::instance()->get(_tracker_id));

	if (!tracker.valid()) 
	{
		// Without tracker an AR application can not work. Quit if none found.
		osg::notify(osg::FATAL) << "Could not initialize tracker plugin!" << std::endl;
		exit(-1);
	}

	// get the tracker calibration object
	osg::ref_ptr<osgART::Calibration> calibration = tracker->getOrCreateCalibration();

	// load a calibration file
	if (!calibration->load(std::string("data/camera_para.dat"))) 
	{

		// the calibration file was non-existing or couldnt be loaded
		osg::notify(osg::FATAL) << "Non existing or incompatible calibration file" << std::endl;
		exit(-1);
	}

	// set the image source for the tracker
	tracker->setImage(video.get());
	
	osgART::TrackerCallback::addOrSet(root.get(), tracker.get());

	osg::ref_ptr<osgART::Marker> marker = tracker->addMarker("single;data/patt.hiro;80;0;0");
	//osg::ref_ptr<osgART::Marker> marker = tracker->addMarker("peugeot_logo.jpg;8;7.6");
	if (!marker.valid()) 
	{
		// Without marker an AR application can not work. Quit if none found.
		osg::notify(osg::FATAL) << "Could not add marker!" << std::endl;
		exit(-1);
	}

	marker->setActive(true);

	osg::ref_ptr<osg::MatrixTransform> arTransform = new osg::MatrixTransform();

	osgART::attachDefaultEventCallbacks(arTransform.get(),marker.get());

	osg::ref_ptr<osg::Node> nodeT = osgDB::readNodeFile("./ives/11_17_90/n_022.ive");

	//arTransform->addChild(osgART::testCube());
	arTransform->addChild(nodeT);
	arTransform->getOrCreateStateSet()->setRenderBinDetails(100, "RenderBin");

	osg::ref_ptr<osg::Group> videoBackground = createImageBackground(video.get());
	videoBackground->getOrCreateStateSet()->setRenderBinDetails(0, "RenderBin");

	osg::ref_ptr<osg::Camera> cam = calibration->createCamera();
	
	vector< osg::ref_ptr<osg::MatrixTransform> > *vectorTransform;
	vectorTransform = &application->vectorTransform;
	for (int i = 0 ;i < vectorTransform->size(); i++)
	{
		std::cout << "i is " << i <<"\n";
	//	osg::ref_ptr<osg::MatrixTransform> flowVector = (*vectorTransform)[i];
		double** points = application->points;
		vector<int> flag_current = application->flag_current;
		(*vectorTransform)[i]->setMatrix(osg::Matrix::translate(points[flag_current[i]][0],points[flag_current[i]][1],points[flag_current[i]][2]));
		
		osg::Quat *quat = new osg::Quat();
		osg::Vec3f *newVec=new osg::Vec3f(1.0,0.0,0.0);
		if(application->flag_next[i]!=application->start[i])
		{	

			newVec->set(application->points[application->flag_next[i]][0]-application->points[application->flag_current[i]][0],application->points[application->flag_next[i]][1]-application->points[application->flag_current[i]][1],application->points[application->flag_next[i]][2]-application->points[application->flag_current[i]][2]);
			newVec->normalize();
			quat->makeRotate(osg::Vec3f(1.0,0.0,0.0),*newVec);
			//double tmp = 0;
			//quat->getRotate(tmp,*axisVec);
			(*vectorTransform)[i]->preMult(osg::Matrix::rotate(*quat));
			//vectorTransform[i]->preMult(osg::Matrix::rotate(tmp,axisVec->x(),axisVec->y(),axisVec->z()));
		}
		else
		{
			newVec->set(application->points[application->flag_current[i]][0]-application->points[application->flag_previous[i]][0],application->points[application->flag_current[i]][1]-application->points[application->flag_previous[i]][1],application->points[application->flag_current[i]][2]-application->points[application->flag_previous[i]][2]);
			newVec->normalize();
			quat->makeRotate(osg::Vec3f(1.0,0.0,0.0),*newVec);
			//double tmp = 0;
			//quat->getRotate(tmp,*axisVec);
			(*vectorTransform)[i]->preMult(osg::Matrix::rotate(*quat));
			//vectorTransform[i]->preMult(osg::Matrix::rotate(tmp,axisVec->x(),axisVec->y(),axisVec->z()));
		}
		application->updataFlags();
		
		arTransform->addChild((*vectorTransform)[i].get());
	}
	
	


	cam->addChild(arTransform.get());
	cam->addChild(videoBackground.get());

	root->addChild(cam.get());
	

	video->start();
	viewer.run();
	freeMemory(application);
	return 0;
	
}
