//
//  StringUtil.h
//  GEngine
//
//  Created by Clark Kromenaker on 12/30/17.
//
#pragma once
#include <string>
#include <algorithm>
#include <cctype>

namespace StringUtil
{
    inline void ToUpper(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }
    
    inline std::string ToUpperCopy(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }
    
    inline void ToLower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }
    
    inline std::string ToLowerCopy(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
    
    inline void Trim(std::string& str)
    {
        // Find first non-space character index. If we can't (str is all spaces), just return.
        size_t first = str.find_first_not_of(' ');
        if(first == std::string::npos) { return; }
        
        // Find first non-space in the back.
        size_t last = str.find_last_not_of(' ');
        
        // Finally, trim off the front and back whitespace.
        str = str.substr(first, (last - first + 1));
    }
    
    // Struct that encapsulates a case-insensitive character comparison.
    struct iequal
    {
        bool operator()(int c1, int c2) const
        {
            return std::toupper(c1) == std::toupper(c2);
        }
    };
    
    inline bool EqualsIgnoreCase(const std::string& str1, const std::string& str2)
    {
        return std::equal(str1.begin(), str1.end(), str2.begin(), iequal());
    }
}