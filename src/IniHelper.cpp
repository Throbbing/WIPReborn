#include "IniHelper.h"

CSimpleIniA WIPIniHelper::_ini;
std::string WIPIniHelper::_filename = "";
//bool WIPIniHelper::_binit = false;

bool WIPIniHelper::reset_ini_file(const char* filename)
{
	SI_Error rc;
	_ini.Reset();
	rc = _ini.LoadFile(filename);
	
	if (rc>=0)
	{
		_filename = filename;
		//_binit = true;
		return true;
	}
	else
	{
		_filename = "";
		return false;
	}
	

}

bool WIPIniHelper::reset_ini_from_text(const char* content,int len)
{
	SI_Error rc;
	_ini.Reset();
	rc = _ini.LoadData(content,len);
	if (rc>=0)
	{
		_filename = "Memory";
		//_binit = true;
		return true;
	}
	else
	{
		_filename = "";
		return false;
	}
}

//read a spercific term to put to [des].return false if no this term
bool WIPIniHelper::get_int(char*section_name,char* name,int& des)
{
	if (_filename=="")
	{
		return false;
	}

	des=_ini.GetLongValue(section_name,name,NULL);
	return true;
	
}

bool WIPIniHelper::get_float(char*section_name,char* name,float& des)
{
	if (_filename=="")
	{
		return false;
	}

	des=(float)_ini.GetDoubleValue(section_name,name,NULL);
	return true;
}

bool  WIPIniHelper::get_string(char*section_name,char* name,std::string& des)
{
	if (_filename=="")
	{
		return false;
	}
	des=(std::string)(_ini.GetValue(section_name,name,NULL));
	return true;
}

void  WIPIniHelper::set_int(char* name,int val,char* section_name)
{
	if (_filename=="")
	{
		return;
	}
	if (section_name==NULL)
	{
		_ini.SetLongValue("NULL", name, val);
	}
	else
	{
		_ini.SetLongValue(section_name,name,val);
	}

	_ini.SaveFile(_filename.data());
}

void  WIPIniHelper::set_float(char* name,float val,char* section_name)
{
	if (_filename=="")
	{
		return;
	}
	if (section_name==NULL)
	{
		_ini.SetDoubleValue("NULL", name, val);
	}
	else
	{
		_ini.SetDoubleValue(section_name,name,val);
	}
	_ini.SaveFile(_filename.data());
}
void  WIPIniHelper::set_string(char* name,std::string val,char* section_name)
{
	if (_filename=="")
	{
		return;
	}

	if (section_name==NULL)
	{
		_ini.SetValue("NULL", name, val.c_str());
	}
	else
	{
		_ini.SetValue(section_name,name,val.c_str());
	}
		_ini.SaveFile(_filename.data());
}

//rename a section to new_name
//create a new section on the end of file if there is no such section
void  WIPIniHelper::rename_section(char* section_name,char* new_name)
{
	if (_filename=="")
	{
		return;
	}
	_ini.SetValue(section_name,new_name,NULL);
	_ini.SaveFile(_filename.data());
}

void WIPIniHelper::close()
{
	if (_filename=="")
	{
		return;
	}
	_ini.Reset();
}