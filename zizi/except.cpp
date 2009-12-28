#include "except.h"

#include <iostream>
using std::cerr;
using std::endl;

void Except::dump() const {
    switch (n) {
    case SS_INIT_ERR:
        cerr<<"initialization error"; break;
    case SS_SPRITE_LOADING_ERR:
        cerr<<"sprite loading error"; break;
    case SS_SPRITE_TOO_MANY_ERR:
        cerr<<"too many sprites"; break;
    case SS_SPRITE_CONVERSION_ERR:
        cerr<<"sprite conversion error"; break;
    case SS_SPRITE_UNKNOWN_ERR:
        cerr<<"sprite unknown"; break;
    case SS_SPRITE_DUPLICATE_ERR:
        cerr<<"duplicate sprite"; break;
    case SS_SPRITE_MAPPING_ERR:
        cerr<<"text mapping error"; break;
    case SS_XML_PARSING_ERR:
        cerr<<"xml parsing error"; break;
    case SS_XML_ID_UNKNOWN_ERR:
        cerr<<"xml unknown id"; break;
    case SS_XML_ID_DUPLICATE_ERR:
        cerr<<"xml duplicate id"; break;
    case SS_SOUND_LOADING_ERR:
        cerr<<"sound loading error"; break;
    case SS_SOUND_DUPLICATE_ERR:
        cerr<<"sound duplicate id"; break;
    case SS_SOUND_UNKNOWN_ERR:
        cerr<<"sound unknown"; break;
    default:
        cerr<<"unknown error"; break;
    }
    cerr<<" ** "<<str<<endl;
}
