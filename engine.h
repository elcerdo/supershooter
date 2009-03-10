#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <map>
#include <list>
#include <string>
#include <utility>
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
    Sprite(unsigned int id,double w,double h,const std::string &name);

    virtual void dump(std::ostream &os) const;
    virtual void draw() const;
    float x,y,angle,factorx,factory;
protected:
    std::string name;
    unsigned int id;
    float w,h;
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

    typedef std::map<std::string,std::pair<unsigned int,SDL_Surface*> > IdMap;
    unsigned int *ids;
    size_t currentid,maxid;
    IdMap idmap;
};

#endif
