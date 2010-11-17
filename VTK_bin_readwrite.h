/******************************************
VTK_bin_readwrite.h

Functionality:
1) Write data to binary format
2) Read in binary data

MODIFICATIONS (Modification, Author, Date):
(1)Created this file, John Moreland(morelanj@calumet.purdue.edu), 08_18_08
********************************************/
#include "VTK_File_Reader.h"

void writeBin(float old_temperatureData[numFrames][MAX_GRID_NUM], float old_co2Data[numFrames][MAX_GRID_NUM], float old_x_velData[numFrames][MAX_GRID_NUM], float old_y_velData[numFrames][MAX_GRID_NUM], float old_z_velData[numFrames][MAX_GRID_NUM], float old_coData[numFrames][MAX_GRID_NUM])
{
	std::cout<<"writing old_temperature.bin.dat...";
	ofstream fout_old_temperature("old_temperature.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (long j=0;j<MAX_GRID_NUM;j++)
			fout_old_temperature.write((char *)(&old_temperatureData[i][j]), sizeof(&old_temperatureData[i][j]));
	std::cout<<"done."<<std::endl;

	std::cout<<"writing old_co2.bin.dat...";
	ofstream fout_old_co2("old_co2.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (long j=0;j<MAX_GRID_NUM;j++)
			fout_old_co2.write((char *)(&old_co2Data[i][j]), sizeof(&old_co2Data[i][j]));
	std::cout<<"done."<<std::endl;

	std::cout<<"writing old_x_vel.bin.dat...";
	ofstream fout_old_x_vel("old_x_vel.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (long j=0;j<MAX_GRID_NUM;j++)
			fout_old_x_vel.write((char *)(&old_x_velData[i][j]), sizeof(&old_x_velData[i][j]));
	std::cout<<"done."<<std::endl;

	std::cout<<"writing old_y_vel.bin.dat...";
	ofstream fout_old_y_vel("old_y_vel.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (long j=0;j<MAX_GRID_NUM;j++)
			fout_old_y_vel.write((char *)(&old_y_velData[i][j]), sizeof(&old_y_velData[i][j]));
	std::cout<<"done."<<std::endl;

	std::cout<<"writing old_z_vel.bin.dat...";
	ofstream fout_old_z_vel("old_z_vel.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (long j=0;j<MAX_GRID_NUM;j++)
			fout_old_z_vel.write((char *)(&old_z_velData[i][j]), sizeof(&old_z_velData[i][j]));
	std::cout<<"done."<<std::endl;
	
	std::cout<<"writing old_co.bin.dat...";
	ofstream fout_old_co("old_co.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (long j=0;j<MAX_GRID_NUM;j++)
			fout_old_co.write((char *)(&old_coData[i][j]), sizeof(&old_coData[i][j]));
	std::cout<<"done."<<std::endl;

}

void readBin(float old_temperatureData[numFrames][MAX_GRID_NUM], float old_co2Data[numFrames][MAX_GRID_NUM], float old_x_velData[numFrames][MAX_GRID_NUM], float old_y_velData[numFrames][MAX_GRID_NUM], float old_z_velData[numFrames][MAX_GRID_NUM], float old_coData[numFrames][MAX_GRID_NUM])
{
	std::cout<<"reading old_temperature.bin.dat...";
	ifstream fin_old_temperature("old_temperature.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (int j=0;j<MAX_GRID_NUM;j++)
			fin_old_temperature.read((char *)(&old_temperatureData[i][j]), sizeof(&old_temperatureData[i][j]));
	std::cout<<"done."<<std::endl;

	std::cout<<"reading old_co2.bin.dat...";
	ifstream fin_old_co2("old_co2.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (int j=0;j<MAX_GRID_NUM;j++)
			fin_old_co2.read((char *)(&old_co2Data[i][j]), sizeof(&old_co2Data[i][j]));
	std::cout<<"done."<<std::endl;

	std::cout<<"reading old_x_vel.bin.dat...";
	ifstream fin_old_x_vel("old_x_vel.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (int j=0;j<MAX_GRID_NUM;j++)
			fin_old_x_vel.read((char *)(&old_x_velData[i][j]), sizeof(&old_x_velData[i][j]));
	std::cout<<"done."<<std::endl;

	std::cout<<"reading old_y_vel.bin.dat...";
	ifstream fin_old_y_vel("old_y_vel.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (int j=0;j<MAX_GRID_NUM;j++)
			fin_old_y_vel.read((char *)(&old_y_velData[i][j]), sizeof(&old_y_velData[i][j]));
	std::cout<<"done."<<std::endl;

	std::cout<<"reading old_z_vel.bin.dat...";
	ifstream fin_old_z_vel("old_z_vel.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (int j=0;j<MAX_GRID_NUM;j++)
			fin_old_z_vel.read((char *)(&old_z_velData[i][j]), sizeof(&old_z_velData[i][j]));
	std::cout<<"done."<<std::endl;
	
	std::cout<<"reading old_co.bin.dat...";
	ifstream fin_old_co("old_co.bin.dat", ios::binary);
	for (int i=0;i<numFrames;i++)
		for (int j=0;j<MAX_GRID_NUM;j++)
			fin_old_co.read((char *)(&old_coData[i][j]), sizeof(&old_coData[i][j]));
	std::cout<<"done."<<std::endl;

}


