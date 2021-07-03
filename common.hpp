/*
    GPLv3 license
    Copyleft 2021 - Noël Martinon

    THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY
    APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT
    HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY
    OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM
    IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF
    ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
*/

#ifndef __COMMON_HPP
#define __COMMON_HPP

#include <string>
#include <iostream>
#include <vector>
#include <cstring>         //strerror
#include <sys/stat.h>
#include <dirent.h>     // dirent, opendir

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    #include <lm.h>
    #include <sstream>
    #include <tlhelp32.h>
#elif __linux__
    #include <unistd.h>
    #include <fstream>  //getProcIdByName
#endif

bool FileExists(const std::string& file);
bool DirectoryExists(const std::string& directory);
bool FindFiles(std::vector<std::string>& vList, const std::string directory, std::string search, bool bGetFiles=true, bool bGetDirectories=false, bool bRecursif=true, bool (*callback)(std::string)=NULL);
bool match(const char *first, const char * second);
std::string GetHomePath();
int getProcIdByName(std::string procName);
#ifdef _WIN32
bool GetWorkgroup(std::string &WorkGroup);
bool GetUserName(std::string &UserName);
bool GetFullName(std::string UserName, std::string Domain, std::string &FullName);
#elif __linux__
#endif

#endif
