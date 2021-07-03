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

#include "common.hpp"

//---------------------------------------------------------------------------------------------
/**
 *  FileExists()
 *  Check if a regular file exists
 *  Returns true if the file exists
 */
bool FileExists(const std::string& path) {
    struct stat path_stat;
    return (stat(path.c_str(), &path_stat) == 0 && S_ISREG(path_stat.st_mode));
}
//---------------------------------------------------------------------------------------------
/**
 *  DirectoryExists()
 *  Check if a directory exists
 *  Returns true if the directory exists
 */
bool DirectoryExists(const std::string& path) {
    struct stat path_stat;
    return (stat(path.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode));
}
//---------------------------------------------------------------------------------------------
/**
 *  FindFiles()
 *  Saerch files or directories and set result into a string vector
 *  Filters on files or directories can be apply. By default one files filter is enable.
 *  Recursif search is enabled by default.
 *  Return true if at least one file is found
 */
bool FindFiles(std::vector<std::string>& vList, const std::string directory, std::string search, bool bGetFiles, bool bGetDirectories, bool bRecursif, bool (*callback)(std::string)) {
    if (!bGetFiles && !bGetDirectories) return false;
    DIR *dir;
    struct dirent *ent;
    bool ret = false;

    if (DirectoryExists(directory) && (dir = opendir (directory.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            bool is_dir;
            bool is_file;
            if (strcmp(ent->d_name, ".") == 0 || // If file is "."
                strcmp(ent->d_name, "..") == 0) //  or ".."
                continue;

            #ifdef _DIRENT_HAVE_D_TYPE
                if (ent->d_type != DT_UNKNOWN && ent->d_type != DT_LNK) {
                   // don't have to stat if we have d_type info, unless it's a symlink (since we stat, not lstat)
                   is_dir = (ent->d_type == DT_DIR);
                   is_file = (ent->d_type == DT_REG);
                } else
            #endif
                // Cross platform code is used to avoid problems
                // (eg: bad value on get S_ISDIR or S_IFDIR from stat(ent->st_mode, &stbuf) on Windows)
                {
                   is_dir = DirectoryExists(directory+"/"+ent->d_name);
                   is_file = FileExists(directory+"/"+ent->d_name);
                }

            if ( ((bGetDirectories && is_dir) || (bGetFiles && is_file)) && match(search.c_str(), ent->d_name) )
            {
                vList.push_back(directory+"/"+ent->d_name);
                //~ if (*callback)
                if (callback) (*callback)(directory+"/"+ent->d_name);
                ret = true;
            }

            if (bRecursif && is_dir)
                if (FindFiles(vList, directory+"/"+ent->d_name, search, bGetFiles, bGetDirectories, bRecursif, callback))
                    ret = true;
        }

        closedir (dir);
        return ret;
    }

    return ret;
}
//---------------------------------------------------------------------------------------------
/**
 *  match()
 *  Checks if two given strings match.
 *  The wild string may contain wildcard characters :
 *  * --> Matches with 0 or more instances of any character or set of characters.
 *  ? --> Matches with any one character.
*/
bool match(const char *wild, const char * str) {
    // If we reach at the end of both strings, we are done
    if (*wild == '\0' && *str == '\0')
        return true;

    // Make sure that the characters after '*' are present
    // in str string. This function assumes that the wild
    // string will not contain two consecutive '*'
    if (*wild == '*' && *(wild+1) != '\0' && *str == '\0')
        return false;

    // If the wild string contains '?', or current characters
    // of both strings match
    if (*wild == '?' || *wild == *str)
        return match(wild+1, str+1);

    // If there is *, then there are two possibilities
    // a) We consider current character of str string
    // b) We ignore current character of str string.
    if (*wild == '*')
        return match(wild+1, str) || match(wild, str+1);
    return false;
}
//---------------------------------------------------------------------------------------------
/**
 *  GetUserProfilePath()
 *  Returns the current user profile folder
 */
std::string GetHomePath() {
    #ifdef __linux__
        std::string HomeDirectory = getenv("HOME");
    #elif defined(_WIN32)
        std::string HomeDirectory = getenv("HOMEDRIVE");
        std::string Homepath = getenv("HOMEPATH");
        HomeDirectory += Homepath;
    #endif
    return HomeDirectory;
}
//-----------------------------------------------

#ifdef _WIN32
//-----------------------------------------------
/**
 *  GetWorkgroup()
 *  Get workgroup or domain
 */
bool GetWorkgroup(std::string &WorkGroup) {
    DWORD dwLevel = 102;
    LPWKSTA_INFO_102 pBuf = NULL;
    NET_API_STATUS nStatus;
    LPTSTR pszServerName = NULL;
    bool bRes=false;

    nStatus = NetWkstaGetInfo(NULL, dwLevel, (LPBYTE *)&pBuf);
    if (nStatus == NERR_Success)
    {
        TCHAR szTemp[MAX_PATH]={'\0',};
        int BufSize=sizeof(szTemp)/sizeof(TCHAR);
        #ifdef UNICODE
        _tcscpy(szTemp, pBuf->wki102_langroup);
        #else
        ::WideCharToMultiByte(CP_ACP, 0, pBuf->wki102_langroup, -1, szTemp, BufSize, 0, 0);
        #endif
        WorkGroup = szTemp;
        bRes=TRUE;
    }

    if (pBuf != NULL)
        NetApiBufferFree(pBuf);
    return bRes;
}

//-----------------------------------------------
/**
 *  GetUserName()
 *  Get the current user name
 */
bool GetUserName(std::string &UserName) {
    char szTemp[MAX_PATH];
    DWORD dwSize = sizeof(szTemp);
    if (!GetUserName(szTemp, &dwSize))
        return false;
    UserName = szTemp;
    return true;
}

//-----------------------------------------------
/**
 *  GetFullName()
 *  Get the fullname information about a user account
 */
bool GetFullName(std::string UserName, std::string Domain, std::string &FullName) {
    char szTemp[MAX_PATH];
    WCHAR wszUserName[UNLEN+1]; // Unicode user name
    WCHAR wszDomain[MAX_PATH];
    LPBYTE ComputerName;
    NET_API_STATUS nStatus;
    LPUSER_INFO_2 ui = NULL;

    // Convert ANSI user name and domain to Unicode
    MultiByteToWideChar(CP_ACP, 0, UserName.c_str(), UserName.length()+1, wszUserName, sizeof(wszUserName)/sizeof(wszUserName[0]) );
    MultiByteToWideChar(CP_ACP, 0, Domain.c_str(), Domain.length()+1, wszDomain, sizeof(wszDomain)/sizeof(wszDomain[0]));

    // Get the computer name of a DC for the domain.
    // The NULL means locprintfal computer
    nStatus = NetGetDCName(NULL, wszDomain, &ComputerName);
    if(nStatus != NERR_Success)
        return false;

    nStatus = NetUserGetInfo((LPCWSTR)ComputerName, (LPCWSTR)&wszUserName, 2, (LPBYTE *)&ui);
    if(nStatus != NERR_Success)
        return false;

    if (!ui)
        return false;

    // Convert the Unicode full name to ANSI.
    WideCharToMultiByte(CP_ACP, 0, ui->usri2_full_name, -1, szTemp, MAX_PATH, NULL, NULL);
    FullName = szTemp;
    return true;
}

//-----------------------------------------------
/**
 *  EnableDebugPriv()
 *  Enable debug privilege
 */
void EnableDebugPriv() {
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;

    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);

    CloseHandle(hToken);
}

//-----------------------------------------------
/**
 *  getProcIdByName()
 *  Get PID of process name (only the first found in the list)
 *  Return -1 if process is not found
 */
int getProcIdByName(std::string procName) {
  int pid = -1;
  // Get the process list snapshot.
  HANDLE hProcessSnapShot = CreateToolhelp32Snapshot(
                                          TH32CS_SNAPALL,
                                          0 );

  // Initialize the process entry structure.
  PROCESSENTRY32 ProcessEntry = { 0 };
  ProcessEntry.dwSize = sizeof( ProcessEntry );

  if( !Process32First( hProcessSnapShot, &ProcessEntry ) )// Get the first process info failed.
  {
      return -1;
  }

  do
  {
      if (stricmp(procName.c_str(),ProcessEntry.szExeFile)==0)
      {
        pid = ProcessEntry.th32ProcessID;
        break;
      }
  }
  while( Process32Next( hProcessSnapShot, &ProcessEntry ));

  // Close the handle
  CloseHandle( hProcessSnapShot );
  return pid;
}
//-----------------------------------------------

#elif __linux__
/**
 *  getProcIdByName()
 *  Get PID of process name
 *  Return -1 if process is not found
 */
int getProcIdByName(std::string procName) {
    int pid = -1;

    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != NULL)
    {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp)))
        {
            // Skip non-numeric entries
            int id = atoi(dirp->d_name);
            if (id > 0)
            {
                // Read contents of virtual /proc/{pid}/cmdline file
                std::string cmdPath = std::string("/proc/") + dirp->d_name + "/cmdline";
                std::ifstream cmdFile(cmdPath.c_str());
                std::string cmdLine;
                getline(cmdFile, cmdLine);
                if (!cmdLine.empty())
                {
                    // Keep first cmdline item which contains the program path
                    size_t pos = cmdLine.find('\0');
                    if (pos != std::string::npos)
                        cmdLine = cmdLine.substr(0, pos);
                    // Keep program name only, removing the path
                    pos = cmdLine.rfind('/');
                    if (pos != std::string::npos)
                        cmdLine = cmdLine.substr(pos + 1);
                    // Compare against requested process name
                    if (procName == cmdLine)
                        pid = id;
                }
            }
        }
    }

    closedir(dp);
    return pid;
}
//-----------------------------------------------
#endif


