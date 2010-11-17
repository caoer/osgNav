/************************************************************************
VTP_File_Reader.h

CLASS: VTP_File_Reader 			
FUNCTIONALITY: 
(1)Read the value of points to array
(2)Read the record of streamlines to array

**************************************************************************/

#include "iostream"
#include "stdio.h"
#include "fstream"
#include "sstream"
#include "string.h"
#include <vector>
#include <cmath>
#include "config_Loader.h"
using namespace std;



struct rgb{
	float r,g,b;
};

static double MAX_temperature = 0;
static double MIN_temperature = 0;
static double MAX_velocity = 0;
static double MIN_velocity = 0;


class VTP_File_Reader:public config_Loader{

public:
	vector<int> flag_current, flag_previous,flag_next,start,length;
	double* offsets;
	double** points;
	double* temperature;
	double* velocity;
	vector<rgb> color;




public:
	VTP_File_Reader()
	{
	}

	void init_VTP()
	{
		cout<<"init in VTP_File_Reader"<<endl;
		cout<<"init_Speed:"<<init_Speed<<endl;
		cout<<"NUM_Lines:"<<NUM_Lines<<"; NUM_Points:"<<NUM_Points<<endl;
		
		offsets = (double*)malloc(NUM_Lines * sizeof(double)); 
		points = (double**)malloc(NUM_Points*sizeof(double*));
		
		if(!offsets || !points || !points[0])
		{
			cout<<"memory mallocation error!!!!!!!!"<<endl;
			exit(0);
		}
		
		for(int i = 0; i < NUM_Points; i++)
		{
			points[i] = (double*)malloc(3*sizeof(double));
			if(!points[i])
			{
				cout<<"memory mallocation error!!!!!!!!"<<endl;
				exit(0);
			}
		}
		//if(temperature_open==1)
		temperature = (double*)malloc(NUM_Points * sizeof(double));
		//if(velocity_open==1)
		velocity = (double*)malloc(NUM_Points * sizeof(double));
		if(!temperature || !velocity)
		{
			cout<<"memory mallocation error!!!!!!!!"<<endl;
			exit(0);
		}
	}

	void recordPoints(char* temp_file_Address,int points_start,int points_end,int points_offset,int previous_points,int temp_points)
	{
		fstream current_VTP_File(temp_file_Address);
		int current_Line = 1;
		string str;
		int point_Position = 0;
		double temp;
		vector<double> vector_points;
		if (!current_VTP_File)	
		{
			cout<<"can't open this current VTP file."<<endl;
			return;
		}
		else
		{
			cout<<"start to record points in:"<<temp_file_Address<<endl;
			while(current_VTP_File)
			{
				if(getline(current_VTP_File,str)&&current_Line>=points_start&&current_Line<=points_end)
				{
					istringstream stream(str);
					while(stream >> temp)
						vector_points.push_back(temp);
				}
				if(current_Line>points_end) break;
				current_Line++;
			}

			cout<<"vector_points["<<vector_points.size()<<"]"<<endl;
			cout<<"points_offset:"<<points_offset<<"; temp_points+points_offset:"<<temp_points+points_offset<<endl;
			cout<<"myScale:"<<myScale<<"; FVScale:"<<FVScale<<endl;
			for(int i=3*points_offset;i<3*(temp_points+points_offset);i+=3)
			{
				
				for(int j=0;j<3;j++)
				{
					points[point_Position+previous_points][j] = vector_points[i+j]*myScale*FVScale;
					//cout<<"points["<<point_Position+previous_points<<"]["<<j<<"]:"<<vector_points[i+j]<<endl;
				}
				//cout<<"point_position:"<<point_Position<<endl;
				point_Position++;
				if(point_Position>=temp_points) break;
				if(point_Position+previous_points>=NUM_Points) break;
				
			}
			vector_points.clear();
			current_VTP_File.close();
		}
	}

	void recordOffsets(char* temp_file_Address,int offsets_start,int offsets_end,int previous_points,int previous_lines, int temp_lines)
	{
		fstream current_VTP_File(temp_file_Address);
		int current_Line = 1;
		string str;
		int offsets_Position = 0;
		int tmp;
		if (!current_VTP_File)	
		{
			cout<<"can't open this current VTP file."<<endl;
			return;
		}
		else
		{
			cout<<"start to record offsets"<<endl;
			while(current_VTP_File)
			{
				if(getline(current_VTP_File,str)&&current_Line>=offsets_start&&current_Line<=offsets_end)
				{
					istringstream stream(str);
					while(stream >> tmp)
					{
						offsets[previous_lines+offsets_Position] = tmp + previous_points;						
						offsets_Position++;
					}
				}
				current_Line++;
				if(previous_lines+offsets_Position >= NUM_Lines) break;
				if(offsets_Position >= temp_lines) break;
			}
			current_VTP_File.close();
		}
		
	}


	void recordTemperature(char* temp_file_Address,int temperature_start,int temperature_end,int previous_points,int temp_points,int points_offset)
	{

		fstream current_VTP_File(temp_file_Address);
		int current_Line = 1;
		string str;
		int temperature_Position = 0;
		double temp_temperature;
		if (!current_VTP_File)	
		{
			cout<<"can't open this current VTP file."<<endl;
			return;
		}
		else
		{
			cout<<"start to record temperature"<<endl;
			while(current_VTP_File)
			{
				if(getline(current_VTP_File,str)&&current_Line>=temperature_start&&current_Line<=temperature_end)
				{
					istringstream stream(str);
					while(stream >> temp_temperature)
					{	
						if(temperature_Position>=points_offset&&temperature_Position<=points_offset+temp_points)
							temperature[previous_points+temperature_Position] = temp_temperature;
						else
						{	
							current_VTP_File.close();
							return;
						}
						temperature_Position++;
					}
				}
				current_Line++;
			}
			current_VTP_File.close();
		}

	}

	void recordVelocity(char* temp_file_Address,int velocity_start,int velocity_end,int previous_points,int temp_points,int points_offset)
	{
		fstream current_VTP_File(temp_file_Address);
		int current_Line = 1;
		string str;
		int velocity_Position = 0;
		double temp_velocity;
		if(!current_VTP_File)
		{
			cout<<"can't open this current VTP file."<<endl;
			return;
		}
		else
		{
			cout<<"start to record velocity"<<endl;
			while(current_VTP_File)
			{
				if(getline(current_VTP_File,str)&&current_Line>=velocity_start&&current_Line<=velocity_end)
				{
					istringstream stream(str);
					while(stream >> temp_velocity)
					{
						if(velocity_Position>=points_offset&&velocity_Position<points_offset+temp_points)
						{
							velocity[previous_points+velocity_Position] = temp_velocity;
						}
						else
						{
							current_VTP_File.close();
							return;
						}
						velocity_Position++;
					}
				}
				current_Line++;
			}
			current_VTP_File.close();
		}
		cout<<"velocity[0]: "<<velocity[0]<<endl;
	}

	void read_List_File(string vtp_list_file_name)
	{
		//for list file
		string str;
		int flag = 0;

		//for flags in vtp file
		char* temp_file_Address;
		int points_start;
		int points_end;
		int offsets_start;
		int offsets_end;
		int points_offset;
		int previous_points;
		int temp_points;
		int previous_lines;
		int temp_lines;
		int temperature_start;
		int temperature_end;
		double Temp_MAX_temperature;
		double Temp_MIN_temperature;
		int velocity_start;
		int velocity_end;		
		double Temp_MAX_velocity;
		double Temp_MIN_velocity;

		cout<<"vtp list file is:"<<vtp_list_file_name<<endl;
		ifstream list_File(vtp_list_file_name.c_str());
		if (!list_File)	
			cout<<"can't open this vtplist file."<<endl;
		else
		{
			while(list_File)
			{
				if(getline(list_File,str)&&flag>0)
				{
					istringstream stream(str);
					string temp_address;
					stream>>temp_address;
					temp_file_Address = new char[temp_address.length()];
					strcpy(temp_file_Address,temp_address.c_str());
					stream>>points_start;
					stream>>points_end;
					stream>>offsets_start;
					stream>>offsets_end;
					stream>>points_offset;
					stream>>previous_points;
					stream>>temp_points;
					stream>>previous_lines;
					stream>>temp_lines;
					stream>>temperature_start;
					stream>>temperature_end;
					stream>>Temp_MAX_temperature;
					stream>>Temp_MIN_temperature;
					stream>>velocity_start;
					stream>>velocity_end;	
					stream>>Temp_MAX_velocity;
					stream>>Temp_MIN_velocity;
					MAX_temperature = (MAX_temperature>=Temp_MAX_temperature?MAX_temperature:Temp_MAX_temperature);
					MIN_temperature = (MIN_temperature<Temp_MIN_temperature?MIN_temperature:Temp_MIN_temperature);
					MAX_velocity = (MAX_velocity>=Temp_MAX_velocity?MAX_velocity:Temp_MAX_velocity);
					MIN_velocity = (MIN_velocity<Temp_MIN_velocity?MIN_velocity:Temp_MIN_velocity);

					recordPoints(temp_file_Address,points_start,points_end,points_offset,previous_points,temp_points);
					cout<<"point recorded.......done"<<endl;
					recordOffsets(temp_file_Address,offsets_start,offsets_end,previous_points,previous_lines,temp_lines);
					cout<<"offsets recorded......done"<<endl;
					if(temperature_start!=-1&&temperature_open==1)
						recordTemperature(temp_file_Address,temperature_start,temperature_end,previous_points,temp_points,points_offset);
					
					if(velocity_start!=-1&&velocity_open==1)
					{
						recordVelocity(temp_file_Address,velocity_start,velocity_end,previous_points,temp_points,points_offset);
					}
					
					
				}
				flag++;
			}
			list_File.close();
		}

		if(velocity_open == 1)
		{
			calculateColor(MAX_velocity, MIN_velocity);
		}	
	}

	void initFlags()
	{
		int start_index = 0;
		int end_index = 0;
		int temp_length = 0;
		for(int i=0;i<NUM_Lines;i++)
		{
			start_index = (i==0)?0:offsets[i-1];
			end_index = offsets[i];
			temp_length = end_index - start_index;
			if(temp_length>=MIN_Points&&temp_length<=MAX_Points)//sample of streamlines
			{
				for(int j=start_index;j<end_index;j+=distance)
				{
					if(j==start_index)
					{
						flag_current.push_back(j);
						flag_previous.push_back(end_index-1);
						flag_next.push_back(j+1);
					}
					else if(j==end_index)
					{
						flag_current.push_back(j);
						flag_previous.push_back(j-1);
						flag_next.push_back(start_index);
					}
					else
					{
						flag_current.push_back(j);
						flag_previous.push_back(j-1);
						flag_next.push_back(j+1);
					}
					start.push_back(start_index);
					length.push_back(end_index-start_index);
				}
			}
		}
	}

	void updataFlags()
	{
		for(int i=0;i<flag_current.size();i++)
		{
			flag_current[i] = start[i]+(flag_current[i]+1)%length[i];
			flag_previous[i] = start[i]+(flag_previous[i]+1)%length[i];
			flag_next[i] = start[i]+(flag_next[i]+1)%length[i];
		}
	}


	rgb temp2rgb(double myTemp,int temporvel, double MAX_value, double MIN_value)
	{
		
		rgb color;
		float normTemp;
	/*	if(temporvel==1)
			normTemp= 1-((MAX_temperature-myTemp>=0?(MAX_temperature-myTemp):0)/(MAX_temperature-MIN_temperature));
		if(temporvel==0)
		{
			normTemp= 1-((MAX_velocity-myTemp>=0?(MAX_velocity-myTemp):0)/(MAX_velocity-MIN_velocity));

			cout<<"MAX_velocity="<<MAX_velocity<<"; MIN:"<<MIN_velocity<<"; normTemp="<<normTemp<<endl;
		}*/
		
		normTemp = 1 - ((MAX_value-myTemp>=0?(MAX_value-myTemp):0)/(MAX_value-MIN_value));
		//cout<<"MAX_value="<<MAX_value<<"; MIN:"<<MIN_value<<"; normTemp="<<normTemp<<endl;
		if (normTemp <= 0.25)
		{
			color.r=0;
			color.g=normTemp/0.25; //incline
			color.b=1;

		}
		else if (normTemp > 0.25 && normTemp <= 0.5)
		{
			color.r=0;
			color.g=1;
			color.b=4*(0.5-normTemp); //decline
		}
		else if (normTemp >0.5 && normTemp <= 0.75)
		{
			color.r=(normTemp-0.5)/0.25; //incline
			color.g=1;
			color.b=0;

		}
		else if (normTemp >0.75 && normTemp <= 1.0)
		{
			color.r=1;
			color.g=4*(1.0-normTemp); //decline
			color.b=0;
		}
		//cout<<"r="<<color.r<<"; g="<<color.g<<"; b="<<color.b<<endl;
		return color;
	}

	void calculateColor(double MAX_value, double MIN_value)
	{
		for(int i=0; i<NUM_Points; i++)
		{
			if(velocity_open==1&&temperature_open==0)
			{
				rgb temp = temp2rgb(velocity[i],0, MAX_value, MIN_value);
				color.push_back(temp);
			}
			if(temperature_open==1&&velocity_open==0)
				color.push_back(temp2rgb(temperature[i],1, MAX_value, MIN_value));	
		}
	}
};

