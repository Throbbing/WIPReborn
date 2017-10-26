#ifndef _WIP_LOGGER_H_
#define _WIP_LOGGER_H_

#define WIP_NOTE	1
#define WIP_WARNING	2
#define WIP_ERROR	3
#define WIP_INFO 4

const int MAX_DEBUG_LINE_LEN = 1024;
const int MAX_SINGLE_LINE_CHAR_NUM = 1024;
#include <fstream> 
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stdarg.h>


using namespace std;

//this class will not export to lua
//to add mutex!
class WIPLogger
{
public:
	static WIPLogger*get_instance();
	
	/*open log system and set output directory*/
	void startup(const char* log_path);
	/*close system */
	void shutdown();

	void debug_log(unsigned int flags,const char* content,...);
	void debug_print(unsigned int flags,const char* content,...);
	void debug(unsigned int flags,const char* content,...);
	void new_log();
	void flush();

	bool isInitialized() { return m_initialized; }
	
protected:
	WIPLogger();
	~WIPLogger();
	
private:
	void parse_flags(unsigned int flag);
	static WIPLogger* _instance;
	ofstream outfile; 
	bool            m_initialized;
	string _buf;
	string _default_path;
	string _current_line;
	int _current_lines;
	

};

extern WIPLogger* g_logger;

//add "##" to hold that there is noly one paramter.
//LOG_NOTE("foobar") g_logger-debug(WIP_NOTE,"foobar",) ERROR!!
#define LOG_NOTE(fmt,...) g_logger->debug(WIP_NOTE,fmt,##__VA_ARGS__)
#define LOG_WARN(fmt,...) (g_logger->debug(WIP_WARNING,fmt,##__VA_ARGS__ ))
#define LOG_ERROR(fmt,...) (g_logger->debug(WIP_ERROR,fmt,##__VA_ARGS__ ))
#define LOG_INFO(fmt,...) (g_logger->debug(WIP_INFO,fmt,##__VA_ARGS__ ))


#endif // !_WIP_LOGGER_H_