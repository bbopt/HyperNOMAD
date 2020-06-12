//
//  fileutils.hpp
//  HyperNomad
//
//  Created by Christophe Tribes on 19-05-28.
//  Copyright © 2019 GERAD. All rights reserved.
//

#ifndef __FILEUTILS__
#define __FILEUTILS__

#include <string>
#include <iostream>
#include <sstream>
#include <limits>
#include <limits.h>
#include <cstdlib>


// use of 'access' or '_access', and getpid() or _getpid():
#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#define PATH_MAX 260
#define getcwd(x,y) _getcwd(x,y)
#define isdigit(x) iswdigit(x)
#else
#include <unistd.h>
#endif

#ifdef _MSC_VER
const char dirSep[] = "\\";
#else
const char dirSep[] = "/";
#endif


using namespace std;

// Current directory
std::string curDir();

// Remove directory from the given path+filename.
std::string trimDir(const std::string &filename);

// Extract directory from the given path+filename.
// If there is no directory, return current directory using getcwd().
std::string extractDir(const std::string &filename);

// Check if a file exists and is readable
bool checkAccess(const std::string &filename);

#endif
