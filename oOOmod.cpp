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

/*
 * Complilation intructions:
 * linux: g++ -std=c++17 oOOmod.cpp pugixml.cpp common.cpp -o oOOmod
 * windows 64bits: i686-w64-mingw32-g++ -static -Os -s -std=c++17 oOOmod.cpp pugixml.cpp common.cpp -o oOOmod.exe -lws2_32 -lnetapi32 -Wl,--subsystem,windows
*/

#include "oOOmod.hpp"

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main(int argc, char** argv)
#endif
{
    #ifdef _WIN32
        g_argc = __argc;
        g_argv = __argv;
    #else
        g_argc = argc;
        g_argv = argv;
    #endif

    std::string userpath;
    std::vector<std::string> vList;

    if (g_argc<2) {
        std::cout << "oOOmod v1.0 - 2021 Noel MARTINON" << std::endl;
        std::cout << "Usage : oOOmod config.xcu" << std::endl;
        return 1;
    }

    if (getProcIdByName("soffice.bin")>-1) {
        std::cout << "Execution abord: Process soffice.bin is running !" << std::endl;
        return 1;
    }

    #ifdef _WIN32
        char Path[MAX_PATH];
        if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, Path) == S_OK)
            userpath = Path;
        userpath +="/libreoffice/4";
    #elif __linux
        userpath = GetHomePath()+"/.config/libreoffice/4";
    #endif

    if (FindFiles(vList, userpath, "registrymodifications.xcu", true, false, true, Process_xcu))
        std::cout << "Process ended" << std::endl;
    else
        std::cout << "Process ended without having found a 'xcu' file" << std::endl;

    return 0;

}
