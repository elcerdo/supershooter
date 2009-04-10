#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <iostream>
#include <map>
#include <set>
struct SDL_Surface;

class Sprite {
friend class SpriteManager;
public:
    virtual void dump(std::ostream &os=std::cout,const std::string &indent="") const;
    virtual void draw(float dt) const;
    Sprite *create_child(const std::string &name);
    void absolute_coordinates(float &ax,float &ay,float &aangle,float &afactorx, float &afactory) const;

    virtual ~Sprite();

    float x,y,z,alpha,angle,factorx,factory,cx,cy;
    const float w,h;
protected:
    Sprite(unsigned int id,float w,float h,const std::string &name);

    typedef std::set<Sprite*> Children;
    Sprite *parent;
    Children children;
    
    std::string name;
    unsigned int id;
};

class StateSprite : public Sprite {
friend class SpriteManager;
public:
    virtual void dump(std::ostream &os=std::cout,const std::string &indent="") const;
    virtual void draw(float dt) const;

    const unsigned int nstate;
    unsigned int state;

    const float rh;
protected:
    StateSprite(unsigned int id,float w,float h,const std::string &name,unsigned int nstate);
};

class AnimatedSprite : public StateSprite {
friend class SpriteManager;
public:
    virtual void dump(std::ostream &os=std::cout,const std::string &indent="") const;
    virtual void draw(float dt) const;

    const unsigned int nframe;
    unsigned int length;
    bool repeat;
    float pos,speed;

    const float rw;
protected:
    AnimatedSprite(unsigned int id,float w,float h,const std::string &name,unsigned int nstate,unsigned int nframe);
};

class Text : public Sprite {
friend class SpriteManager;
public:
    virtual void draw(float dt) const;
    virtual void dump(std::ostream &os=std::cout,const std::string &indent="") const;
    void update(const std::string &str);
    void update_alpha();
    enum Align { LEFT,CENTER,RIGHT };
    Align align;
protected:
    typedef std::map<char,unsigned int> CharMap;
    Text(unsigned int id,float w,float h,const std::string &name,const std::string &str,const CharMap &mapping,Align align);
    void update_align();
    const CharMap mapping;
};

//***********************************************************
class SpriteManager {
public:
    static SpriteManager *get();
    static void free();
    static void init(size_t maxid=256);

    void dump(std::ostream &os=std::cout) const;
    void load_image(const std::string &filename);
    bool load_directory(const std::string &directory);
    Sprite *get_sprite(const std::string &name) const;
    Text *get_text(const std::string &str,const std::string &name,Text::Align align) const;
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
    Text::CharMap mapping;
};


#endif
