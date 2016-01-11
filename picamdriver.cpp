#include "picamdriver.h"
#include "hwexception.h"

std::mutex PicamDriver::piCamMutex;
bool PicamDriver::s_running;
PicamDriver * PicamDriver::s_driverInstance = nullptr;
std::thread PicamDriver::piCamThread;

PicamDriver::PicamDriver(const PicamDriver& _pcd)
{

}

PicamDriver::PicamDriver()
{

}

PicamDriver::~PicamDriver()
{

}

void PicamDriver::create(unsigned int _fps, unsigned int _vidWidth, unsigned int _vidHeight)
{
	if(s_driverInstance!=nullptr){
		s_driverInstance = new PicamDriver();
		s_running=1;
	}else{
		throw HWException("Picam instance already created");
	}
}

void PicamDriver::end()
{
		
}
