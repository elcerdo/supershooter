#ifndef __EXCEPT_H__
#define __EXCEPT_H__

#include <string>

struct Except {
    enum ExceptType {
        SS_INIT_ERR,
        SS_SPRITE_LOADING_ERR,
        SS_SPRITE_TOO_MANY_ERR,
        SS_SPRITE_CONVERSION_ERR,
        SS_SPRITE_UNKNOWN_ERR,
        SS_SPRITE_DUPLICATE_ERR,
        SS_SPRITE_MAPPING_ERR,
        SS_XML_PARSING_ERR,
        SS_XML_ID_UNKNOWN_ERR,
        SS_XML_ID_DUPLICATE_ERR,
        SS_MUSIC_LOADING_ERR,
        SS_MUSIC_DUPLICATE_ERR,
        SS_MUSIC_UNKNOWN_ERR,
    };
    Except(ExceptType n,const std::string &str="") : n(n), str(str) {};
    ExceptType n;
    std::string str;

    void dump() const;
};

#endif

