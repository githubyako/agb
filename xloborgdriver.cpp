#include "xloborgdriver.h"

std::mutex XloborgDriver::xloMutex;
bool XloborgDriver::s_running;
XloborgDriver* XloborgDriver::s_driverInstance = nullptr;
std::thread XloborgDriver::xloThread;

XloborgDriver::XloborgDriver(){}

XloborgDriver::XloborgDriver(const XloborgDriver& XloborgDriver){}

XloborgDriver::XloborgDriver(unsigned int _refreshtime):m_refreshtime(_refreshtime)
{
	XLoInit();
	XLoReadAccelerometer(m_accel);
	XLoReadCompassRaw(m_magnet);
	s_running=true;
	m_orient = floor(180 * atan2(m_magnet[1],m_magnet[0]) / M_PI);
}

void XloborgDriver::create(int _refreshtime)
{
	if(s_driverInstance!=nullptr){
		std::cout << "Xloborg driver has already been created." << std::endl;
	}else{
		s_driverInstance = new XloborgDriver(_refreshtime);
		xloThread = std::thread(&XloborgDriver::refresh, s_driverInstance);
	}
	
}

void XloborgDriver::end()
{
	s_driverInstance->s_running=false;
	xloThread.join();
	delete s_driverInstance;
}


XloborgDriver::~XloborgDriver()
{
	XLoShutdown();
}

void XloborgDriver::calcOrientation()
{
	m_magnet[0]+=1984.1595;
	m_magnet[1]-=1370.225;
	m_magnet[2]-=630;
	float AngersDecl = 0.4822222;
	float accelmagnitude= sqrt(pow(m_accel[0], 2.0) + pow(m_accel[1], 2.0) + pow(m_accel[2], 2.0));
	m_orient[1] = asin(-(m_accel[1]) / accelmagnitude);
	m_orient[2] = asin(m_accel[0] / accelmagnitude);
	float x = ((-(m_magnet[0])) * cos(m_orient[2])) + (m_magnet[2] * sin(m_orient[2]));
	float y = (m_magnet[0] * sin(m_orient[1]) * sin(m_orient[2])) + (m_magnet[1] * cos(m_orient[1])) + (m_magnet[2] * sin(m_orient[1]) * cos(m_orient[2]));
	float heading =  atan2(y,x) + AngersDecl;
	if(heading < 0){
		heading += (2* M_PI);
	}else if(heading > 2*M_PI){
		heading -= (2* M_PI);
	}
	heading = heading * 180/M_PI;
	heading = floor(heading);
	
	float heading = floor(180 * atan2(m_magnet[1],m_magnet[0]) / M_PI);
	m_orient = heading;
}

float XloborgDriver::getOrientation()
{
	
	return s_driverInstance->m_orient;
}

float XloborgDriver::getreading(int _axis)
{
	switch(_axis){
		case 0: 
			return s_driverInstance->m_magnet[0];
			break;
		case 1 :
			return s_driverInstance->m_magnet[1];
			break;
		case 2: 
			return s_driverInstance->m_magnet[2];
			break;
		default:
			return 0.0;
	}
}


float XloborgDriver::getTemperature()
{
	xloMutex.lock();
	float result = XLoReadTemperature();
	xloMutex.unlock();
	return result;
	
}

std::string XloborgDriver::to_string()
{
	return "mx=" + std::to_string(s_driverInstance->m_magnet[0]) + ", my=" + std::to_string(s_driverInstance->m_magnet[1]) + ", mz=" + std::to_string(s_driverInstance->m_magnet[2]) ;
}



void XloborgDriver::refresh()
{
	while(s_running){
		usleep(m_refreshtime);
		xloMutex.lock();
		XLoReadAccelerometer(m_accel);
		XLoReadCompassRaw(m_magnet);
		calcOrientation();
		xloMutex.unlock();
	}
}

