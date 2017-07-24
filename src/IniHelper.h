#include "./thirdpart/SimpleIni.h"

class WIPIniHelper 
{
public:
  //重新设定当前影响的ini文件
  static bool reset_ini_file(const char *filename);
  static bool reset_ini_from_text(const char* content,int len);
  //读取一个指定名字name的值存放到des，没有此项目返回false.
  static bool get_int(char *section_name, char *name, int &des);
  static bool get_float(char *section_name, char *name, float &des);
  static bool get_string(char *section_name, char *name, std::string &des);

  /*写入一个指定名字那么的值，如果此项目存在重写那个值，如果不存在:
   *section_name==NULL
   *则创建一个section名为[NULL],在那一section后面新建此值并写入
   *否则 则在指定section_name后面新建此值并写入
  */
  static void set_int(char *name, int val, char *section_name = NULL);
  static void set_float(char *name, float val, char *section_name = NULL);
  static void set_string(char *name, std::string val,
                         char *section_name = NULL);
//rename a section to new_name
//create a new section on the end of file if there is no such section
  static void rename_section(char *section_name, char *new_name);

  static void close();

private:
  static CSimpleIniA _ini;
  static std::string _filename;
  // static bool _binit;
};