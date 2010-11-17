/************************************************************************
  			Datafile_Reader.cpp

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
#include "VTK_File_Reader.h"

int VTK_File_Reader::ReadGridfile(char name[]) 
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

int VTK_File_Reader::ReadDatafile(char name[],int index) 
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

int VTK_File_Reader::GetGridArrayIndex(void)
{
	return grid_count;
}


int VTK_File_Reader::FeedGridArray(float (&vertex)[MAX_GRID_NUM][3] )
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


int VTK_File_Reader::FeedDataArray( float (&point_value)[MAX_GRID_NUM],int index)
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

//================================Private Functions=================================
int VTK_File_Reader:: gridGetNextFloat(float (&fnum)[10]) {  // get a line of the file, and then transform them to float
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

int VTK_File_Reader:: dataGetNextFloat(float (&fnum)[10],int index) {  // get a line of the file, and then transform them to float
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