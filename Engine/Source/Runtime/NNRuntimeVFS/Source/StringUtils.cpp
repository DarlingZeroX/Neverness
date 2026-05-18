#include "VFS/StringUtils.h"

namespace NN::Runtime::VFS
{

void StringUtils::Split(std::vector<std::string>& tokens, const std::string& text, char delimeter)
{
    size_t start = 0;
    size_t end = 0;
    while ((end = text.find(delimeter, start)) != std::string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));
}

std::string StringUtils::Replace(std::string string, const std::string& from, const std::string& to)
{
    size_t pos = 0;
    while ((pos = string.find(from, pos)) != std::string::npos) {
        string.replace(pos, from.length(), to);
        pos += to.length();
    }
    return string;
}

bool StringUtils::EndsWith(const std::string& fullString, const std::string& ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    return false;
}

bool StringUtils::StartsWith(const std::string& fullString, const std::string& starting)
{
    if (fullString.length() >= starting.length()) {
        return (0 == fullString.compare(0, starting.length(), starting));
    }
    return false;
}

} // namespace NN::Runtime::VFS
