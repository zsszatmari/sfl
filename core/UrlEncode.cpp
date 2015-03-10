//
//  UrlEncode.cpp
//  G-Ear Player
//
//  Created by Zsolt Szatmári on 9/5/13.
//
//

#include "UrlEncode.h"

namespace Gear
{
#define method UrlEncode::
    
    string method encode(const string &original)
    {
        string str = original;
        
        static const char hex[] = "0123456789ABCDEF";
        size_t index = 0;
        
        while (true) {
            if (index >= str.length()) {
                break;
            }
            unsigned char c = str[index];
            
            if (!isalnum(c) && c != '-') {
                char encoded[3];
                encoded[0] = '%';
                encoded[1] = hex[c >> 4];
                encoded[2] = hex[c & 0xF];
                
                str.replace(index, 1, encoded, sizeof(encoded));
                index += sizeof(encoded);
            } else {
                ++index;
            }
        }
        return str;
    }
    
    static inline int numberFromChar(char c)
    {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }
        if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        return 0;
    }
    
    string method decode(const string &str)
    {
        string decoded;
        size_t index = 0;
        while (true) {
            if (index >= str.length()) {
                break;
            }
            char c = str[index];
            
            if (c == '%') {
                if ((index+2) >= str.length()) {
                    break;
                }
                char newChar = (numberFromChar(str[index+1]) << 4) + numberFromChar(str[index+2]);
                decoded.push_back(newChar);
                index += 3;
            } else if (c == '\\' && (index + 5) < str.length()  && str[index+1] == 'u' && str[index+2] == '0' && str[index+3] == '0') {
            	char newChar = (numberFromChar(str[index+4]) << 4) + numberFromChar(str[index+5]);
            	decoded.push_back(newChar);
            	index += 6;
            } else {
                decoded.push_back(c);
                ++index;
            }
        }
        return decoded;
    }
}
