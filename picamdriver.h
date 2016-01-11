#ifndef PICAMDRIVER_H
#define PICAMDRIVER_H
#include <iostream>
#include <mutex>
#include <thread>

class PicamDriver{
	private:
		
		unsigned int m_fps,m_VIDwidth,m_VIDheight;
		PicamDriver(PicamDriver const & _pcd);
		PicamDriver();
		~PicamDriver();
	protected:
		
		static PicamDriver* s_driverInstance;
		static bool s_running;
		
		static std::mutex piCamMutex;
		static std::thread piCamThread;
		
	public:
		
		static void create(unsigned int _fps, unsigned int _vidWidth, unsigned int _vidHeight);
		static void end();

  
};
#endif