#pragma once

#include <string>
#include <vector>

//from Urho3D

/// Return files.
static const unsigned SCAN_FILES = 0x1;
/// Return directories.
static const unsigned SCAN_DIRS = 0x2;
/// Return also hidden files.
static const unsigned SCAN_HIDDEN = 0x4;

class WIPFileSystem 
{
public:
  static WIPFileSystem* get_instance()
  {
    static WIPFileSystem* _instance = 0;
    if(!_instance)
		  _instance = new WIPFileSystem();
	  return _instance;
  }
  
protected:
  WIPFileSystem(){}
  ~WIPFileSystem(){}

public:
  // set current working directory
  bool set_current_dir(const std::string &path_name);

  // return the absolute current working dir
  std::string get_current_dir() const;
  // Check if a file exists.absolute path
  bool file_exists(const std::string &file_name) const;
  // Check if a directory exists.absolute path
  bool dir_exists(const std::string &path_name) const;
  // Scan a directory for specified files.
  void scan_dir(std::vector<std::string>& result, const std::string& path, const std::string& filter, unsigned int flags, bool recursive) const;
private:
  void scan_dir_internal(std::vector<std::string>& result, std::string path, const std::string& startPath,const std::string& filter, unsigned int flags, bool recursive) const;

public:
  // Split a full path to path, filename and extension. The extension will be
  // converted to lowercase by default.
  static void split_path(const std::string &fullPath, std::string &pathName,
                        std::string &fileName, std::string &extension,
                        bool lowercaseExtension = true);
  //Return the path from a full path.
  static std::string get_path(const std::string &fullPath);
  //Return the filename from a full path.
  static std::string get_filename(const std::string &fullPath);
  //Return the extension from a full path, converted to lowercase by default.
  static std::string get_extension(const std::string &fullPath,
                                  bool lowercaseExtension = true);
  //Return the filename and extension from a full path. The case of the
  //extension is preserved by default, so that the file can be opened in
  //case-sensitive operating systems.
  static std::string get_filename_and_extension(const std::string &fullPath,
                                             bool lowercaseExtension = false);
  //Replace the extension of a file name with another.
  static std::string replace_extension(const std::string &fullPath,
                                      const std::string &newExtension);
  //Add a slash at the end of the path if missing and convert to internal
  //format (use slashes.)
  static std::string add_trailing_slash(const std::string &pathName);
  //Remove the slash from the end of a path if exists and convert to internal
  //format (use slashes.)
  static std::string remove_trailing_slash(const std::string &pathName);
  //Return the parent path, or the path itself if not available.
  static std::string get_parent_path(const std::string &pathName);
  //Convert a path to internal format (use slashes.)
  static std::string get_internal_path(const std::string &pathName);
  //Convert a path to the format required by the operating system.
  static std::string get_native_path(const std::string &pathName);
  //Convert a path to the format required by the operating system in wide
  //characters.
  //static WString GetWideNativePath(const String& pathName);
  //Return whether a path is absolute.
  static bool is_absolute_path(const std::string &pathName);

  std::string get_relative_path(const std::string &pathName);
};


extern WIPFileSystem* g_filesystem;