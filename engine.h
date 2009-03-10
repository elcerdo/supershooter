#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <map>
#include <list>
#include <string>
#include <iostream>
#include <SDL/SDL_image.h>

class SdlManager {
public:
    static SdlManager *get();
    static void free();
    static void init(int w=800,int h=600, int d=32);

    void clear();
    void swap();
    void wait();
protected:
    SdlManager(int w,int h,int d);
    ~SdlManager();

    SDL_Surface *screen;
};

//***********************************************************
class Sprite {
public:
    Sprite(unsigned int id,float w,float h,const std::string &name);

    virtual void dump(std::ostream &os) const;
    virtual void draw() const;
    float x,y,angle,factorx,factory;
protected:
    std::string name;
    unsigned int id;
    float w,h;
};

class StateSprite : public Sprite {
public:
    StateSprite(unsigned int id,float w,float h,const std::string &name,unsigned int nstate);

    virtual void dump(std::ostream &os) const;
    virtual void draw() const;

    const unsigned int nstate;
    unsigned int state;

    const float rh;
};

class AnimatedSprite : public Sprite {
public:
    AnimatedSprite(unsigned int id,float w,float h,const std::string &name,unsigned int nstate,unsigned int nframe);

    virtual void dump(std::ostream &os) const;
    virtual void draw() const;

    const unsigned int nstate;
    unsigned int state;

    const unsigned int nframe;
    unsigned int repeat;
    float pos,speed;

    const float rh,rw;
};

typedef std::list<Sprite*> Sprites;

//***********************************************************
class SpriteManager {
public:
    static SpriteManager *get();
    static void free();
    static void init(size_t maxid=256);

    void dump(std::ostream &os) const;
    void load_image(const std::string &filename);
    void load_directory(const std::string &directory);
    Sprite *get_sprite(const std::string &name) const;
protected:
    SpriteManager(size_t maxid);
    ~SpriteManager();

    struct Record {
        enum Type {STATIC,STATE,ANIMATED};

        Record() {}
        Record(unsigned int id,SDL_Surface *surf) : id(id), surface(surf), type(STATIC) {} //static
        Record(unsigned int id,SDL_Surface *surf,unsigned int nstate) : id(id), surface(surf), nstate(nstate), type(STATE) {} //state
        Record(unsigned int id,SDL_Surface *surf,unsigned int nstate,unsigned int nframe) : id(id), surface(surf), nstate(nstate), nframe(nframe), type(ANIMATED) {} //animated with state

        unsigned int id;
        SDL_Surface *surface;
        Type type;
        unsigned int nstate;
        unsigned int nframe;
    };

    typedef std::map<std::string,Record> IdMap;
    unsigned int *ids;
    size_t currentid,maxid;
    IdMap idmap;
};

#endif
