#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include "../../RuntimeVFSExport.h"
#include <string>
#include <vector>

namespace NN::Runtime::VFS
{

class NN_RUNTIME_VFS_API StringUtils
{
public:
    static void Split(std::vector<std::string>& tokens, const std::string& text, char delimeter);
    static std::string Replace(std::string string, const std::string& from, const std::string& to);
    static bool EndsWith(const std::string& fullString, const std::string& ending);
    static bool StartsWith(const std::string& fullString, const std::string& starting);
};

} // namespace NN::Runtime::VFS

#endif // STRINGUTILS_H
