/************************************************************************
  			Datafile_Reader.h

CLASS: VTK_File_Reader 			
FUNCTIONALITY: 
(1)Read the grid coordinate to array
(2)Read the value of grid to array

MODIFICATIONS (Modification, Author, Date):
(1)Initially create this class,Dong Fu, 07_28_2008@PUC CFD Lab
(2)fixed several typos (gird > grid)
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
#define MAX_INDEX 60000 //Max total grid points 

struct CoordinateRange 
{ 
	float max_X, max_Y, max_Z, min_X, min_Y, min_Z;
}; 

class VTK_File_Reader 
{   
public:
	//Read CFD grid coordniate file 
	int ReadGridfile(char[]);

	//Read CFD data value file (Pressure,Temperature,ect..)
	int ReadDatafile(char[],int index);

	//Feed CFD grid coordniate (X,Y,Z) to the array 
	int FeedGridArray(float (&)[MAX_INDEX][3]);

	//Feed CFD data value to the array, multiple datasets can be read
	int FeedDataArray(float (&)[MAX_INDEX],int index);

	//Get the total grid point number.
	int GetGridArrayIndex(void);

	//Get the coordinate range for current CFD model
	struct CoordinateRange XYZRange; 
	
private:
	
	ifstream grid_file_stream; //grid file stream
	ifstream data_file_stream[10]; // data file stream

	int grid_count;//total grid point number 
	
	//read grid file format "x1 y1 z1 x2 y2 z2 x3 y3 z3..."
	int gridGetNextFloat(float (&)[10]);

	//read data value file format "grid1 grid2 grid3..."
	int dataGetNextFloat(float (&)[10],int index);
};
//================================Pubilc Functions=================================
int VTK_File_Reader::ReadGridfile(char name[]) 
{
	char fileName[100];// file name
	strcpy_s(fileName, name);

	grid_file_stream.open(fileName);
	if (!grid_file_stream)
	{
		cerr<< "Can't open the grid file: " <<fileName<<endl;
		return FALSE;
	}

	return TRUE;
}

int VTK_File_Reader::ReadDatafile(char name[],int index) 
{
	char fileName[100];// file name
	strcpy_s(fileName, name);

	data_file_stream[index].open(fileName);
	if (!data_file_stream[index])
	{
		cerr<< "Can't open the data file: " <<fileName<<endl;
		return FALSE;
	}

	return TRUE;
}

int VTK_File_Reader::GetGridArrayIndex(void)
{
	return grid_count;
}


int VTK_File_Reader::FeedGridArray(float (&vertex)[MAX_INDEX][3] )
{
	int st, j = 0;
	float fln[10];
	float max_X=-1000,max_Y=-1000,max_Z=-1000,min_X=1000,min_Y=1000,min_Z=1000;

	//Read Points
	st = this->gridGetNextFloat(fln);
	while (st == TRUE) 
	{
		vertex[j][0] = fln[0];
		vertex[j][1] = fln[1];
		vertex[j][2] = fln[2];
		vertex[j+1][0] = fln[3];
		vertex[j+1][1] = fln[4];
		vertex[j+1][2] = fln[5];
		st = this->gridGetNextFloat(fln);
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


int VTK_File_Reader::FeedDataArray( float (&point_value)[MAX_INDEX],int index)
{
	int st, j = 0;
	float fln[10];
	j=0;
	st = this->dataGetNextFloat(fln,index);
	while (st == TRUE) 
	{
		point_value[j] = fln[0];
		point_value[j+1]= fln[1];
		point_value[j+2]= fln[2];
		point_value[j+3]= fln[3];
		point_value[j+4]= fln[4];
		point_value[j+5]= fln[5];
		st = this->dataGetNextFloat(fln,index);
		j=j+6;
	}
	return TRUE;
}

//================================Private Functions=================================
int VTK_File_Reader:: gridGetNextFloat(float (&fnum)[10]) {  // get a line of the file, and then transform them to float
	int i = 0;
	string line;
	if (getline(grid_file_stream,line)) {
		istringstream stream(line);
		while (stream >> fnum[i]) {
			i++;
		}
		return TRUE;
	} 
	else {
		return FALSE;
	}
}

int VTK_File_Reader:: dataGetNextFloat(float (&fnum)[10],int index) {  // get a line of the file, and then transform them to float
	int i = 0;
	string line;
	if (getline(data_file_stream[index],line)) {
		istringstream stream(line);
		while (stream >> fnum[i]) {
			i++;
		}
		return TRUE;
	} 
	else {
		return FALSE;
	}
}
#endif //__DATAFILE_READER_h__