#include "FileSystem.h"
#include "Logger.h"
#include <algorithm> 
#ifdef _WIN32
#include "windows.h"
#else
#include "unistd.h"
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <utime.h>
#include <sys/wait.h>
#endif

#define MAX_PATH 256
using std::string;

bool end_with (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) 
    {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } 
    else 
    {
        return false;
    }
}

void split(const std::string& s, const std::string& delim, std::vector< std::string >& ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		ret.push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		ret.push_back(s.substr(last, index - last));
	}
}

bool WIPFileSystem::set_current_dir(const std::string &path_name) 
{
#ifdef _WIN32
    if (SetCurrentDirectory(get_native_path(path_name).c_str()) == FALSE)
    {
		
		LOG_ERROR("Failed to change directory to %s[%d]",  path_name.c_str(),GetLastError());
        return false;
    }
#else
    if(chdir(get_native_path(path_name).c_str())!=0)
    {
		LOG_ERROR("Failed to change directory to %s",  path_name.c_str());
        return false;
    }
#endif
    return true;
}

std::string WIPFileSystem::get_current_dir() const
{
#ifdef _WIN32
    char path[MAX_PATH];
    path[0] = 0;
    GetCurrentDirectory(MAX_PATH, path);
    return add_trailing_slash(string(path));
#else
    char path[MAX_PATH];
    path[0] = 0;
    getcwd(path, MAX_PATH);
    return add_trailing_slash(string(path));
#endif
}

bool WIPFileSystem::file_exists(const std::string &file_name) const
{
std::string fixedName = get_native_path(
    remove_trailing_slash(file_name));

#ifdef _WIN32
	DWORD attributes = GetFileAttributes(fixedName.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY)
        return false;
#else
    struct stat st;
    if (stat(fixedName.c_str(), &st) || st.st_mode & S_IFDIR)
        return false;
#endif

    return true;
}

bool WIPFileSystem::dir_exists(const std::string &path_name) const
{
    std::string fixedName = get_native_path(
        remove_trailing_slash(path_name));
#ifdef _WIN32
	DWORD attributes = GetFileAttributes(fixedName.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        return false;
#else
    struct stat st;
    if (stat(fixedName.c_str(), &st) || !(st.st_mode & S_IFDIR))
        return false;
#endif

    return true;
}

void WIPFileSystem::scan_dir(std::vector<std::string>& result, const std::string& path, const std::string& filter, unsigned int flags, bool recursive) const
{
    result.clear();
    std::string init_path = add_trailing_slash(path);
    scan_dir_internal(result,init_path,init_path,filter,flags,recursive);

}

void WIPFileSystem::scan_dir_internal(std::vector<std::string>& result, std::string path, const std::string& startPath,const std::string& filter, unsigned int flags, bool recursive) const
{
    path = add_trailing_slash(path);
    std::string deltaPath;
    if (path.length() > startPath.length())
        deltaPath = path.substr(startPath.length());

    std::string filterExtension = filter.substr(filter.find('.'));
    if (filterExtension.find('*')!=std::string::npos)
        filterExtension.clear();

#ifdef _WIN32
    WIN32_FIND_DATA info;
    HANDLE handle = FindFirstFile(string(path + "*").c_str(), &info);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            string fileName(info.cFileName);
            if (!fileName.empty())
            {
                if (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN && !(flags & SCAN_HIDDEN))
                    continue;
                if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (flags & SCAN_DIRS)
						result.push_back(deltaPath + fileName);
                    if (recursive && fileName != "." && fileName != "..")
                        scan_dir_internal(result, path + fileName, startPath, filter, flags, recursive);
                }
                else if (flags & SCAN_FILES)
                {
                    if (filterExtension.empty() || end_with(fileName,filterExtension))
						result.push_back(deltaPath + fileName);
                }
            }
        }
        while (FindNextFile(handle, &info));

        FindClose(handle);
    }
#else
    DIR* dir;
    struct dirent* de;
    struct stat st;
    dir = opendir(get_native_path(path).c_str());
    if (dir)
    {
        while ((de = readdir(dir)))
        {
            /// \todo Filename may be unnormalized Unicode on Mac OS X. Re-normalize as necessary
            std::string fileName(de->d_name);
            bool normalEntry = fileName != "." && fileName != "..";
            if (normalEntry && !(flags & SCAN_HIDDEN) && (fileName[0]=='.'))
                continue;
            std::string pathAndName = path + fileName;
            if (!stat(pathAndName.c_str(), &st))
            {
                if (st.st_mode & S_IFDIR)
                {
                    if (flags & SCAN_DIRS)
                        result.push_back(deltaPath + fileName);
                    if (recursive && normalEntry)
                        scan_dir_internal(result, path + fileName, startPath, filter, flags, recursive);
                }
                else if (flags & SCAN_FILES)
                {
                    if (filterExtension.empty() || end_with(fileName,filterExtension))
                        result.push_back(deltaPath + fileName);
                }
            }
        }
        closedir(dir);
    }
#endif
}

void string_replace(string &strBase, string strSrc, string strDes)  
{  
    string::size_type pos = 0;  
    string::size_type srcLen = strSrc.size();  
    string::size_type desLen = strDes.size();  
    pos=strBase.find(strSrc, pos);   
    while ((pos != string::npos))  
    {  
        strBase.replace(pos, srcLen, strDes);  
        pos=strBase.find(strSrc, (pos+desLen));  
    }  
}

std::string trim(std::string &s)   
{  
    if (s.empty())
    {  
        return s;
    }  
  
    s.erase(0,s.find_first_not_of(" "));  
    s.erase(s.find_last_not_of(" ") + 1);
    return s;  
}

std::string WIPFileSystem::get_native_path(const std::string& path_name)
{
    return path_name;
}

bool WIPFileSystem::is_absolute_path(const std::string& path_name)
{
    if (path_name.empty())
        return false;

    std::string path = get_internal_path(path_name);

    if (path[0] == '/')
        return true;

#ifdef _WIN32
    if (path.length() > 1 && (
		(path[0] >= 0x61 && path[0] <= 0x7a) || (path[0]>=0x41&&path[0]<=0x5a)
		) && path[1] == ':')
        return true;
#endif

    return false;
}

std::string WIPFileSystem::get_relative_path(const std::string &path_name)
{
	if (!is_absolute_path(path_name))
		return path_name;
	string cur_path = remove_trailing_slash( get_current_dir());
	string path_name_c = remove_trailing_slash(path_name);

	if (cur_path[0] != path_name_c[0])
		return path_name;

	vector<string> ret_cur;
	vector<string> ret_path;

	split(cur_path, "/", ret_cur);
	split(path_name_c, "/", ret_path);

	bool in = false;
	if (ret_cur[0] == ret_path[0])
		in = true;
	int i = 0;
	vector<string>& use = ret_path.size() > ret_cur.size() ? ret_cur: ret_path;
	for (i = 0; i < use.size(); ++i)
	{
		if (ret_cur[i] != ret_path[i])
		{
			break;
		}
	}
	i--;
	string pre = "";
	int remain = ret_cur.size() - i - 1;
	if (remain > 0)
	{
		for (int k = 0; k < remain;k++)
			pre += "../";
	}

	for (int k = i + 1; k < ret_path.size();++k)
	{
		pre += ret_path[k];
		pre += '/';
	}



	return pre;
}

std::string WIPFileSystem::get_internal_path(const std::string& path_name)
{
    string path_copy = path_name;
    string_replace(path_copy,"\\","/");
    return path_copy;
}

std::string WIPFileSystem::get_parent_path(const std::string &path)
{
    std::string::size_type pos = remove_trailing_slash(path).find_last_of('/');
    if (pos != string::npos)
        return path.substr(0, pos + 1);
    else
        return string();
}

void WIPFileSystem::split_path(const std::string& fullPath, std::string& pathName, std::string& fileName, std::string& extension, bool lowercaseExtension)
{
    std::string full_path_copy = get_internal_path(fullPath);

    std::string::size_type extPos = full_path_copy.find_last_of('.');
    std::string::size_type pathPos = full_path_copy.find_last_of('/');

    if (extPos != std::string::npos && (pathPos == std::string::npos || extPos > pathPos))
    {
        extension = full_path_copy.substr(extPos+1);
        if (lowercaseExtension)
        {
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        }
        full_path_copy = full_path_copy.substr(0, extPos);
    }
    else
        extension.clear();

    pathPos = full_path_copy.find_last_of('/');
    if (pathPos != std::string::npos)
    {
        fileName = full_path_copy.substr(pathPos + 1);
        pathName = full_path_copy.substr(0, pathPos + 1);
    }
    else
    {
        fileName = full_path_copy;
        pathName.clear();
    }
}

std::string WIPFileSystem::get_path(const std::string& fullPath)
{
    std::string path, file, extension;
    split_path(fullPath, path, file, extension);
    return path;
}

std::string WIPFileSystem::get_filename(const std::string& fullPath)
{
    std::string path, file, extension;
    split_path(fullPath, path, file, extension);
    return file;
}

std::string WIPFileSystem::get_extension(const std::string& fullPath, bool lowercaseExtension)
{
    std::string path, file, extension;
    split_path(fullPath, path, file, extension, lowercaseExtension);
    return extension;
}

std::string WIPFileSystem::get_filename_and_extension(const std::string& fileName, bool lowercaseExtension)
{
    std::string path, file, extension;
    split_path(fileName, path, file, extension, lowercaseExtension);
    return file + extension;
}

std::string WIPFileSystem::replace_extension(const std::string& fullPath, const std::string& newExtension)
{
    std::string path, file, extension;
    split_path(fullPath, path, file, extension);
    return path + file + newExtension;
}

std::string WIPFileSystem::add_trailing_slash(const std::string& pathName)
{
    std::string pathcopy = pathName;
    std::string ret = trim(pathcopy);
    string_replace(ret,"\\","/");
    if (!ret.empty() && ret[ret.length()-1] != '/')
        ret += '/';
    return ret;
}

std::string WIPFileSystem::remove_trailing_slash(const std::string& pathName)
{
    std::string pathcopy = pathName;
    std::string ret = trim(pathcopy);
    string_replace(ret,"\\","/");
    if (!ret.empty() && ret[ret.length()-1] == '/')
        ret.resize(ret.length() - 1);
    return ret;
}

WIPFileSystem* g_filesystem = WIPFileSystem::get_instance();