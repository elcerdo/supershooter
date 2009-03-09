#include "except.h"

#include <iostream>
using std::cerr;
using std::endl;

void Except::dump() const {
    switch (n) {
    case SS_INIT_ERR:
        cerr<<"initialization error"<<endl; break;
    case SS_LOADING_ERR:
        cerr<<"data loading error"<<endl; break;
    case SS_TOO_MANY_SPRITES_ERR:
        cerr<<"too many sprites"<<endl; break;
    case SS_CONVERSION_ERR:
        cerr<<"sprite conversion error"<<endl; break;
    case SS_SPRITE_ERR:
        cerr<<"unknown sprite"<<endl; break;
    }
}
