#ifndef __EXCEPT_H__
#define __EXCEPT_H__

struct Except {
    enum ExceptType {
        SS_INIT_ERR,
        SS_LOADING_ERR,
        SS_TOO_MANY_SPRITES_ERR,
        SS_CONVERSION_ERR,
        SS_SPRITE_ERR,
    };
    Except(ExceptType n) : n(n) {};
    ExceptType n;

    void dump() const;
};

#endif

