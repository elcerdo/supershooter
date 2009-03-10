#ifndef __EXCEPT_H__
#define __EXCEPT_H__

struct Except {
    enum ExceptType {
        SS_INIT_ERR,
        SS_SPRITE_LOADING_ERR,
        SS_SPRITE_TOO_MANY_ERR,
        SS_SPRITE_CONVERSION_ERR,
        SS_SPRITE_UNKNOWN_ERR,
        SS_SPRITE_DUPLICATE_ERR,
    };
    Except(ExceptType n) : n(n) {};
    ExceptType n;

    void dump() const;
};

#endif

