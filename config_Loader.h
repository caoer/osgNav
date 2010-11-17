#include "iostream"
#include "stdio.h"
#include "fstream"
#include "sstream"
#include "string.h"

using namespace std;

class config_Loader{
public:
	double 	init_X;
	double 	init_Y;
	double 	init_Z;
	double 	init_rot_X;
	double 	init_rot_Y;
	double 	init_rot_Z;
	int 	animation;	//0 for frame by frame, 1 for animation
	double	init_Speed;	
	double 	fast_Speed;
	int		NUM_VTP_Files;
	int		NUM_Points;
	int		NUM_Lines;
	int		MAX_Points;
	int		MIN_Points;
	double	myScale;
	double 	FVScale;
	double 	FVSizeScale;
	int 	Speed;
	double  distance;
	double	R;
	double	G;
	double	B;
	double	A;
	int		velocity_open;
	int		temperature_open;
	

public:
	config_Loader()
	{
	}
	
	void init_config(string config_file_name)
	{
		cout<<"config file:"<<config_file_name<<endl;
		ifstream file_open;
		file_open.open(config_file_name.c_str());
		if (!file_open)	
			cout<< "Can't open the config file."<<endl;
		else
		{
			string str;
			cout<<str<<endl;
			getline(file_open,str);
			istringstream stream(str);
			stream>>init_X;
			stream>>init_Y;
			stream>>init_Z;
			stream>>init_rot_X;
			stream>>init_rot_Y;
			stream>>init_rot_Z;
			stream>>animation;
			stream>>init_Speed;
			stream>>fast_Speed;
			stream>>NUM_VTP_Files;
			stream>>NUM_Points;
			stream>>NUM_Lines;
			stream>>MAX_Points;
			stream>>MIN_Points;
			stream>>myScale;
			stream>>FVScale;
			stream>>FVSizeScale;
			stream>>Speed;
			stream>>distance;
			stream>>R;
			stream>>G;
			stream>>B;
			stream>>A;
			stream>>velocity_open;
			stream>>temperature_open;
		}
		file_open.close();
	}
};
