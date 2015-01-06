#ifndef LOGGING_H
#define LOGGING_H

enum LogTypes
{
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR
};

class SimpleLogger
{
private:
	void init(const char* filename);
	void print(const char* message, LogTypes type);
	void print(LogTypes type);
	std::fstream output;
public:
	SimpleLogger();
	SimpleLogger(const char* filename);

	void i(const char* message);
	void w(const char* message);
	void e(const char* message);

	friend std::fstream & operator << (SimpleLogger &logger, const char *message)
	{
		logger.output << message;
		return logger.output;
	}

	friend SimpleLogger& operator << (SimpleLogger &logger, const LogTypes type)
	{
		logger.print(type);
		return logger;
	}
	
	~SimpleLogger();
};

void SimpleLogger::init(const char* filename)
{
	output.open(filename, std::fstream::out | std::fstream::app);
}

SimpleLogger::SimpleLogger()
{
	init("exScript.log");
}

SimpleLogger::SimpleLogger(const char* filename)
{
	init(filename);
}
void SimpleLogger::print(LogTypes type)
{
	switch (type)
	{
	case LOG_ERROR:
		output << "[ERROR]";
		break;
	case LOG_WARN:
		output << "[WARN]";
		break;
	case LOG_INFO:
	default:
		output << "[INFO]";
		break;
	}
	
}
void SimpleLogger::print(const char* message, LogTypes type)
{
	print(type);
	output << message << std::endl;
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

SimpleLogger::~SimpleLogger()
{
	if (output.is_open()) 
	{		
		output.close();
	}
}


SimpleLogger MainLog;
#endif