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
    }
}
