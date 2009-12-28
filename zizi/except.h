#ifndef __EXCEPT_H__
#define __EXCEPT_H__

#include <string>

//TODO change names to be lib zizi exceptio
struct Except {
    enum ExceptType {
        ZIZI_INIT_ERR,
        ZIZI_SPRITE_LOADING_ERR,
        ZIZI_SPRITE_TOO_MANY_ERR,
        ZIZI_SPRITE_CONVERSION_ERR,
        ZIZI_SPRITE_UNKNOWN_ERR,
        ZIZI_SPRITE_DUPLICATE_ERR,
        ZIZI_SPRITE_MAPPING_ERR,
        ZIZI_SOUND_LOADING_ERR,
        ZIZI_SOUND_DUPLICATE_ERR,
        ZIZI_SOUND_UNKNOWN_ERR,
        SS_XML_PARSING_ERR,
        SS_XML_ID_UNKNOWN_ERR,
        SS_XML_ID_DUPLICATE_ERR,
    };
    Except(ExceptType n,const std::string &stri) : n(n), str(stri) {};
    ExceptType n;
    std::string str;

    void dump() const;
};

#endif

