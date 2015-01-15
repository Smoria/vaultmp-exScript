#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <fstream>

enum LogTypes
{
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR
};

class SimpleLogger
{
private:
	void init(const char* filename, int logLevel);
	void print(const char* message, LogTypes type);
	void print(LogTypes type);
	std::fstream output;
	int logLevel;
public:
	static int MaxConsoleLogLevel;

	SimpleLogger();
	SimpleLogger(int logLevel);
	SimpleLogger(const char* filename);
	SimpleLogger(const char* filename, int logLevel);

	void i(const char* message);
	void w(const char* message);
	void e(const char* message);
	void end();

	friend SimpleLogger & operator << (SimpleLogger &logger, const char *message)
	{
		logger.output << message;
		if (logger.logLevel <= MaxConsoleLogLevel)
			std::cout << message;
		return logger;
	}

	friend SimpleLogger & operator << (SimpleLogger &logger, const int val)
	{
		logger.output << val;
		if (logger.logLevel <= MaxConsoleLogLevel)
			std::cout << val;
		return logger;
	}

	friend SimpleLogger & operator << (SimpleLogger &logger, const vaultmp::String val)
	{
		logger.output << val;
		if (logger.logLevel <= MaxConsoleLogLevel)
			std::cout << val;
		return logger;
	}

	friend SimpleLogger& operator << (SimpleLogger &logger, const LogTypes type)
	{
		logger.print(type);
		return logger;
	}
	
	~SimpleLogger();
};

int SimpleLogger::MaxConsoleLogLevel = 1;

void SimpleLogger::init(const char* filename, int logLevel)
{
	output.open(filename, std::fstream::out | std::fstream::app);
}

SimpleLogger::SimpleLogger()
{
	init("exScript.log", 1);
}

SimpleLogger::SimpleLogger(int logLevel)
{
	init("exScript.log", logLevel);
}

SimpleLogger::SimpleLogger(const char* filename)
{
	init(filename, 1);
}

SimpleLogger::SimpleLogger(const char* filename, int logLevel)
{
	init(filename, logLevel);
}

void SimpleLogger::print(LogTypes type)
{
	std::string outtext;
	switch (type)
	{
	case LOG_ERROR:
		outtext = "[ERROR] ";
		break;
	case LOG_WARN:
		outtext = "[WARN] ";
		break;
	case LOG_INFO:
	default:
		outtext = "[INFO] ";
		break;
	}
	output << outtext;
	if (logLevel <= MaxConsoleLogLevel)
		std::cout << outtext;
}

void SimpleLogger::print(const char* message, LogTypes type)
{
	print(type);
	output << message << std::endl;
	if (logLevel <= MaxConsoleLogLevel)
		std::cout << message << std::endl;
}
void SimpleLogger::i(const char* message)
{
	print(message,LOG_INFO);
}
void SimpleLogger::w(const char* message)
{
	print(message, LOG_WARN);
}
void SimpleLogger::e(const char* message)
{
	print(message, LOG_ERROR);
}
void SimpleLogger::end()
{
	output << std::endl;
	if (logLevel <= MaxConsoleLogLevel)
		std::cout << std::endl;
}
SimpleLogger::~SimpleLogger()
{
	if (output.is_open()) 
	{		
		output.close();
	}
}


SimpleLogger MainLog;
SimpleLogger ChatLog("exChat.log",3);
#endif