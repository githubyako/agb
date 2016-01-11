#include "hwexception.h"

HWException::HWException(std::string _errmsg):m_errmsg(_errmsg)
{

}



const std::string HWException::what()
{
  return m_errmsg;
}
