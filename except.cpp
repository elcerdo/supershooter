#include "except.h"

#include <iostream>
using std::cerr;
using std::endl;

void Except::dump() const {
    switch (n) {
    case SS_INIT_ERR:
        cerr<<"initialization error"<<endl; break;
    case SS_SPRITE_LOADING_ERR:
        cerr<<"sprite loading error"<<endl; break;
    case SS_SPRITE_TOO_MANY_ERR:
        cerr<<"too many sprites"<<endl; break;
    case SS_SPRITE_CONVERSION_ERR:
        cerr<<"sprite conversion error"<<endl; break;
    case SS_SPRITE_UNKNOWN_ERR:
        cerr<<"sprite unknown "<<str<<endl; break;
    case SS_SPRITE_DUPLICATE_ERR:
        cerr<<"duplicate sprite"<<endl; break;
    case SS_SPRITE_MAPPING_ERR:
        cerr<<"text mapping error in '"<<str<<"'"<<endl; break;
    case SS_XML_PARSING_ERR:
        cerr<<"xml parsing error "<<str<<endl; break;
    case SS_XML_ID_UNKNOWN_ERR:
        cerr<<"xml unknown id "<<str<<endl; break;
    case SS_XML_ID_DUPLICATE_ERR:
        cerr<<"xml duplicate id "<<str<<endl; break;
    case SS_MUSIC_LOADING_ERR:
        cerr<<"music loading error"<<endl; break;
    case SS_MUSIC_DUPLICATE_ERR:
        cerr<<"music duplicate id"<<endl; break;
    case SS_MUSIC_UNKNOWN_ERR:
        cerr<<"music unknown "<<str<<endl; break;
    default:
        cerr<<"unknown error "<<n<<endl; break;
    }
}
