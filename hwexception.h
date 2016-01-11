#ifndef HWEXCEPTION_H
#define HWEXCEPTION_H
#include <exception>
#include <string>

class HWException:public std::exception{
private:
std::string m_errmsg;  
  

public:
  HWException(std::string _errmsg);
  std::string const what();
};
#endif