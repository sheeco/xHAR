#pragma once

//#include "Hotspot.h"
#include "Helper.h"


//包含用于文件解析的函数的类
class CFileHelper :
	public CHelper
{
public:

	//传入nodeID和time，将从文件中解析到的信息赋给pos
	static void getPositionFromFile(int nodeID, int time, double &x, double &y);
	//static CPosition* findPositionByID(vector<CPosition *> positions, int ID);

	////将hotspot的时间、坐标和cover数保存到文件
	//static void writeHotspotToFile(char *filename, CHotspot *hotspot, bool withDetail);
	//static void writeHotspotToFile(char *filename, vector<CHotspot *> list_hotspot, bool withDetail);
	//static void writeHotspotStatisics(int time, char *filename, vector<CHotspot *> list_hotspot);
	
	//从hotspot_detail.txt获取hotspot信息，构造hotspot，需要传入所需的positions
	//static vector<CHotspot *> getHotspotFromFile(char *filename, int time, vector<CPosition *> positions);
	//static void SaveHotspotsToFile(int time, vector<CHotspot *> hotspots);

};

