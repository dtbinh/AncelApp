#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <iostream>
#include <string>

namespace ResException
{
	class ResError: public std::exception
	{
	public:
		ResError() {};
		ResError(const std::string &strMsg)
			:m_strMsg(strMsg){};
		~ResError() throw(){};
		virtual std::string GetMessage() const
		{
			return m_strMsg;
		}
		virtual void SetMessage(const std::string &strMsg) 
		{
			m_strMsg = strMsg;
		}
		virtual const char* what() const throw()
		{
			return "Unknown Exception";
		}

	private:
		std::string m_strMsg;
	};
	//------------------------------------------------------------------------------------------------
	class RuntimeError: public ResError
	{

	};
};

#endif