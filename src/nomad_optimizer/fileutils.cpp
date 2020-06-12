
#include "fileutils.hpp"




std::string curDir()
{
    char dirbuff[ PATH_MAX ];
    getcwd(dirbuff, PATH_MAX );
    std::string dir(dirbuff);
    
    return dir;
}


// Remove directory from the given path+filename.
std::string trimDir(const std::string &filename)
{
    std::string dir = filename;
    
    size_t k = filename.find_last_of ( dirSep );
    if ( k < filename.size() )
    {
       dir = filename.substr (k+1, filename.size() );
    }
    
    return dir;
}


// Extract directory from the given path+filename.
std::string extractDir(const std::string &filename)
{
    std::string dir = "";
    
    size_t k = filename.find_last_of ( dirSep );
    if ( k < filename.size() )
    {
        dir = filename.substr (0,k) + dirSep;
    }

    
    return dir;
}


// Check if a file exists and is readable
bool checkAccess(const std::string &filename)
{
#ifdef _MSC_VER
    return (_access ( filename.c_str() , 4 ) == 0);
#else
    return (access ( filename.c_str() , R_OK ) == 0);
#endif
}



