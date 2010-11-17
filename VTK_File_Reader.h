/************************************************************************
  			Datafile_Reader.h

CLASS: VTK_File_Reader 			
FUNCTIONALITY: 
(1)Read the grid coordinate to array
(2)Read the value of grid to array

MODIFICATIONS (Modification, Author, Date):
(1)	Initially create this class,Dong Fu(fudong1985@gmail.com), 07_28_2008@PUC CFD Lab

(2)	Add MAX_DATASET_NUM, specify the number of total datasets, Dong Fu, 08_13_2008@PUC CFD Lab

(3)	Modify gridGetNextFloat,dataGetNextFloat,FeedGridArray,FeedDataArray, 
	The class can read the data/grid file with (1)XML tag (2)Empty Space line, Dong Fu, 08_15_2008@PUC CFD Lab
**************************************************************************/
#include "iostream"
#include "stdio.h"
#include "fstream"
#include "sstream"
#include "string.h"


#ifndef __DATAFILE_READER_h__
#define __DATAFILE_READER_h__

using namespace std;
#define TRUE 1
#define FALSE 0

//file reader
#define VALID_LINE 1	//Valid Grid/Data Value, can be used to feed the grid/data array
#define INVALID_LINE 2  //Invalid Grid/Data value or XML Tag 
#define	END_OF_FILE 0	//End of the input file

#define MAX_GRID_NUM 640000 //Max total grid points 
#define MAX_DATASET_NUM 15 //Max Datasets number
#define numFrames 1 //number of frames in animation

struct CoordinateRange 
{ 
	float max_X, max_Y, max_Z, min_X, min_Y, min_Z;
}; 

class VTK_File_Reader 
{   
public:
	//Read CFD grid coordniate file 
	int ReadGridfile(char name[]) 
	{
		char fileName[100];// file name
		strcpy(fileName, name);

		grid_file_stream.open(fileName);
		if (!grid_file_stream)
		{
			cerr<< "Can't open the grid file: " <<fileName<<endl;
			return FALSE;
		}

		return TRUE;
	}

	//Read CFD data value file (Pressure,Temperature,ect..)
	int ReadDatafile(char name[],int index) 
	{
		char fileName[100];// file name
		strcpy(fileName, name);

		data_file_stream[index].open(fileName);
		if (!data_file_stream[index])
		{
			cerr<< "Can't open the data file: " <<fileName<<endl;
			return FALSE;
		}

		return TRUE;
	}


	//Feed CFD grid coordniate (X,Y,Z) to the array 
	int FeedGridArray(float (&vertex)[MAX_GRID_NUM][3] )
	{
		int st, j = 0;
		float fln[10];
		float max_X=-1000,max_Y=-1000,max_Z=-1000,min_X=1000,min_Y=1000,min_Z=1000;

		//Read Points
		while ( st = this->gridGetNextFloat(fln) ) // Loop untill the st=END_OF_FILE=0
		{
			//if this line is not data, go to the next line
			if (st!=VALID_LINE)
			continue;

			vertex[j][0] = fln[0];
			vertex[j][1] = fln[1];
			vertex[j][2] = fln[2];
			vertex[j+1][0] = fln[3];
			vertex[j+1][1] = fln[4];
			vertex[j+1][2] = fln[5];
			j=j+2;
		
			//=========================Get Max========================
			//get max_X
			if (fln[0]>max_X)
			max_X=fln[0];
			if (fln[3]>max_X)
			max_X=fln[3];

			//get max_Y
			if (fln[1]>max_Y)
			max_Y=fln[1];
			if (fln[4]>max_Y)
			max_Y=fln[4];

			//get max_Z
			if (fln[2]>max_Z)
			max_Z=fln[2];
			if (fln[5]>max_Z)
			max_Z=fln[5];
			//=========================Get Min========================
			//get min_X
			if (fln[0]<min_X)
			min_X=fln[0];
			if (fln[3]<min_X)
			min_X=fln[3];

			//get min_Y
			if (fln[1]<min_Y)
			min_Y=fln[1];
			if (fln[4]<min_Y)
			min_Y=fln[4];

			//get min_Z
			if (fln[2]<min_Z)
			min_Z=fln[2];
			if (fln[5]<min_Z)
			min_Z=fln[5];
		}

		grid_count=j;

		XYZRange.max_X=max_X;
		XYZRange.max_Y=max_Y;
		XYZRange.max_Z=max_Z;
		XYZRange.min_X=min_X;
		XYZRange.min_Y=min_Y;
		XYZRange.min_Z=min_Z;
		return TRUE;
	}


	//Feed CFD data value to the array, multiple datasets can be read
	int FeedDataArray( float (&point_value)[MAX_GRID_NUM],int index)
	{
		int st, j = 0;
		float fln[10];
		j=0;

		while ( st = this->dataGetNextFloat(fln,index) ) // Loop untill the st=END_OF_FILE=0
		{
			//if this line is not data, go to the next line
			if (st!=VALID_LINE)		
			continue;
			
			point_value[j] = fln[0];
			point_value[j+1]= fln[1];
			point_value[j+2]= fln[2];
			point_value[j+3]= fln[3];
			point_value[j+4]= fln[4];
			point_value[j+5]= fln[5];

			j=j+6;
		}
		return TRUE;
	}

	//Get the total grid point number.
	int GetGridArrayIndex(void)
	{
		return grid_count;
	}

	//Get the coordinate range for current CFD model
	struct CoordinateRange XYZRange; 
	
private:
	
	ifstream grid_file_stream; //grid file stream
	ifstream data_file_stream[MAX_DATASET_NUM]; // data file stream

	int grid_count;//total grid point number 
	
	//read grid file format "x1 y1 z1 x2 y2 z2 x3 y3 z3..."
	int gridGetNextFloat(float (&fnum)[10]) {  // get a line of the file, and then transform them to float
		int i = 0;
		string line;
		if (getline(grid_file_stream,line)) 
		{
			istringstream stream(line);
			while (stream >> fnum[i])
			{
				i++;	
			}
		
			// i<3 ==> This line is NOT valid grid value, it is XML tag or invalid grid value line.
			if (i<3)
				return INVALID_LINE;
			else
				return VALID_LINE;
		} 
		else 
		{
			return END_OF_FILE;//return 0
		}
	}

	//read data value file format "grid1 grid2 grid3..."
	int dataGetNextFloat(float (&fnum)[10],int index) {  // get a line of the file, and then transform them to float
		int i = 0;
		string line;
		if (getline(data_file_stream[index],line)) 
		{
			istringstream stream(line);
			while (stream >> fnum[i]) 
			{
				i++;
			}

			// i==0 ==> This line is NOT valid data value, it is XML tag or invalid data line.
			if (i==0)
				return INVALID_LINE;
			else
				return VALID_LINE;
		} 
		else 
		{
			return END_OF_FILE;//return 0
		}
	}
};





#endif //__DATAFILE_READER_h__
