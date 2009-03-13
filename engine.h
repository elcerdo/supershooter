#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <map>
#include <list>
#include <set>
#include <string>
#include <iostream>
#include <SDL/SDL_keysym.h>

//forward declaration
union SDL_Event;
struct SDL_Surface;

class Listener {
friend class SdlManager;
protected:
    virtual bool key_down(SDLKey key) { return true; };
    virtual bool key_up(SDLKey key) { return true; };
    virtual bool mouse_down(int button,float x,float y) { return true; };
    virtual bool mouse_up(int button,float x,float y) { return true; };
    virtual bool frame_entered(float t,float dt)=0;

    virtual void register_self() {};
    virtual void unregister_self() {};
};


//***********************************************************
class SdlManager {
public:
    static SdlManager *get();
    static void free();
    static void init(int w=1024,int h=768, int d=32);

    const unsigned char *get_key_state() const;

    void main_loop();
    void register_listener(Listener *listener);
    void unregister_listener(Listener *listener);
    void set_background_color(float r,float g,float b);
    const float width,height;
protected:
    typedef std::set<Listener*> Listeners;

    SdlManager(int w,int h,int d);
    ~SdlManager();

    static int event_filter(const SDL_Event *ev);
    SDL_Surface *screen;
    Listeners listeners;
    bool in_main_loop;
    long int old_ticks;
};

//***********************************************************
class Sprite {
friend class SpriteManager;
public:
    virtual void dump(std::ostream &os=std::cout,const std::string &indent="") const;
    virtual void draw(float dt) const;
    Sprite *create_child(const std::string &name);
    void absolute_coordinates(float &ax,float &ay,float &aangle,float &afactorx, float &afactory) const;

    virtual ~Sprite();

    float x,y,z,angle,factorx,factory,cx,cy;
    const float w,h;
protected:
    Sprite(unsigned int id,float w,float h,const std::string &name);

    typedef std::list<Sprite*> Children;
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

//***********************************************************
class SpriteManager {
public:
    static SpriteManager *get();
    static void free();
    static void init(size_t maxid=256);

    void dump(std::ostream &os=std::cout) const;
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
