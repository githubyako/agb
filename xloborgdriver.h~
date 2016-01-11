#ifndef XLOBOGDRIVER_H
#define XLOBOGDRIVER_H
#define _USE_MATH_DEFINES
#include <unistd.h>
#include <cmath>
#include <iostream>
#include <mutex>
#include <thread>

extern "C"{
	#include "XLoBorg.h"
}

class XloborgDriver{
private:
	
	float m_accel[3];
	float m_magnet[3];
	float m_orient;
	unsigned int m_refreshtime;
	XloborgDriver(XloborgDriver const & XloborgDriver);
	XloborgDriver(unsigned int _refreshtime);	
	void calcOrientation();
protected:
	static XloborgDriver* s_driverInstance;
	static bool s_running;
	XloborgDriver();
	~XloborgDriver();
	static std::mutex xloMutex;
	static std::thread xloThread;
	
public:
	static std::string to_string();
	
	// calibration method:
	static float getreading(int _axis);
	
	static float getTemperature();
	void refresh();
	static float getOrientation();
	static void create(int _refreshtime);
	static void end();
};
#endif