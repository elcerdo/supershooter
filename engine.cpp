#include "engine.h"

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <cassert>
#include <utility>
#include <boost/regex.hpp>
#include <fstream>
#include <dirent.h>
#include "except.h"
using std::cout;
using std::endl;
using std::cerr;

static SdlManager *mSdlManager=NULL;

SdlManager *SdlManager::get() {
    return mSdlManager;
}

void SdlManager::free() {
    if (mSdlManager) { delete mSdlManager; mSdlManager=NULL; }
}

void SdlManager::init(int w,int h,int d) {
    if (mSdlManager) throw Except(Except::SS_INIT_ERR);
    mSdlManager=new SdlManager(w,h,d);
}

SdlManager::SdlManager(int w,int h,int d) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_OPENGL)) { cerr<<"cannot initialize sdl..."<<endl; throw Except(Except::SS_INIT_ERR); }

    screen=SDL_SetVideoMode(800,600,32,SDL_OPENGL|SDL_DOUBLEBUF);
    if (not screen) { cerr<<"cannot create sdl screen..."<<endl; throw Except(Except::SS_INIT_ERR); }

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(1,1,0,1);
	glClearDepth(1);// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);// The Type Of Depth Testing To Do
	glEnable(GL_BLEND);//Alpha blending
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// Really Nice Perspective Calculations

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,w,h,0,1.0,-1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

SdlManager::~SdlManager() {
    SDL_FreeSurface(screen);
    SDL_Quit();
}

void SdlManager::clear() {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void SdlManager::swap() {
    SDL_GL_SwapBuffers();  
    SDL_Flip(screen);
}

void SdlManager::wait() {
    SDL_Delay(10);
}

//***********************************************************
Sprite::Sprite(unsigned int id,float w,float h,const std::string &name) : id(id), x(0), y(0), angle(0), factorx(1), factory(1), w(w), h(h), name(name) {}

void Sprite::draw() const {
    glBindTexture(GL_TEXTURE_2D,id);
    glPushMatrix();
        glTranslatef(x,y,0.0);
        glRotatef(angle,0.0,0.0,1.0);
        glNormal3f(0.0,0.0,1.0);
        glBegin(GL_QUADS);
        glTexCoord2f(1.0,1.0); glVertex3f(factorx*w/2,factory*h/2,0);
        glTexCoord2f(1.0,0.0); glVertex3f(factorx*w/2,-factory*h/2,0);
        glTexCoord2f(0.0,0.0); glVertex3f(-factorx*w/2,-factory*h/2,0);
        glTexCoord2f(0.0,1.0); glVertex3f(-factorx*w/2,factory*h/2,0);
    glEnd();
    glPopMatrix();
}

void Sprite::dump(std::ostream &os) const { os<<name<<" ["<<x<<","<<y<<"] static"<<endl; }

StateSprite::StateSprite(unsigned int id,float w,float h,const std::string &name,unsigned int nstate) : Sprite(id,w,h/nstate,name), nstate(nstate), rh(1./nstate), state(0) {}

void StateSprite::draw() const {
    float ya=rh*state;
    float yb=ya+rh;
    glBindTexture(GL_TEXTURE_2D,id);
    glPushMatrix();
        glTranslatef(x,y,0.0);
        glRotatef(angle,0.0,0.0,1.0);
        glNormal3f(0.0,0.0,1.0);
        glBegin(GL_QUADS);
        glTexCoord2f(1.0,yb); glVertex3f(factorx*w/2,factory*h/2,0);
        glTexCoord2f(1.0,ya); glVertex3f(factorx*w/2,-factory*h/2,0);
        glTexCoord2f(0.0,ya); glVertex3f(-factorx*w/2,-factory*h/2,0);
        glTexCoord2f(0.0,yb); glVertex3f(-factorx*w/2,factory*h/2,0);
    glEnd();
    glPopMatrix();
}

void StateSprite::dump(std::ostream &os) const { os<<name<<" ["<<x<<","<<y<<"]@"<<state<<" state"<<endl; }

AnimatedSprite::AnimatedSprite(unsigned int id,float w,float h,const std::string &name,unsigned int nstate,unsigned int nframe) : Sprite(id,w/nframe,h/nstate,name), nstate(nstate), rw(1./nframe), rh(1./nstate), state(0), nframe(nframe), repeat(nframe), pos(0.), speed(.1) {}

void AnimatedSprite::draw() const {
    float ya=rh*state;
    float yb=ya+rh;
    float xa=rw*int(pos);
    float xb=xa+rw;
    glBindTexture(GL_TEXTURE_2D,id);
    glPushMatrix();
        glTranslatef(x,y,0.0);
        glRotatef(angle,0.0,0.0,1.0);
        glNormal3f(0.0,0.0,1.0);
        glBegin(GL_QUADS);
        glTexCoord2f(xb,yb); glVertex3f(factorx*w/2,factory*h/2,0);
        glTexCoord2f(xb,ya); glVertex3f(factorx*w/2,-factory*h/2,0);
        glTexCoord2f(xa,ya); glVertex3f(-factorx*w/2,-factory*h/2,0);
        glTexCoord2f(xa,yb); glVertex3f(-factorx*w/2,factory*h/2,0);
    glEnd();
    glPopMatrix();

    const_cast<float&>(pos)+=speed;
    if (pos>=repeat) const_cast<float&>(pos)-=repeat;
}

void AnimatedSprite::dump(std::ostream &os) const { os<<name<<" ["<<x<<","<<y<<"]@"<<state<<","<<pos<<" animated"<<endl; }

//***********************************************************
static SpriteManager *mSpriteManager=NULL;

SpriteManager *SpriteManager::get() {
    return mSpriteManager;
}

void SpriteManager::free() {
    if (mSpriteManager) { delete mSpriteManager; mSpriteManager=NULL; }
}

void SpriteManager::init(size_t maxid) {
    if (mSpriteManager) throw Except(Except::SS_INIT_ERR);
    mSpriteManager=new SpriteManager(maxid);
}

SpriteManager::SpriteManager(size_t maxid) : maxid(maxid), currentid(0) {
    ids=new unsigned int[maxid];
    glGenTextures(maxid,ids);
}

SpriteManager::~SpriteManager() {
    delete [] ids;
    for (IdMap::const_iterator i=idmap.begin(); i!=idmap.end(); i++) SDL_FreeSurface(i->second.surface);
}

void SpriteManager::load_directory(const std::string &directory) {
    DIR *dir=opendir(directory.c_str());
    if (not dir) return;

    std::string prefix(directory);
    if (prefix.size()>0 and prefix[prefix.size()-1]!='/') prefix+='/';

    dirent *ent;
    while (ent=readdir(dir)) {
        std::string filename(ent->d_name);
        if (filename=="." or filename=="..") continue;

        try { load_image(prefix+filename);
        } catch (Except &e) {
            if (e.n==Except::SS_SPRITE_LOADING_ERR) cerr<<"can't load "<<prefix<<filename<<endl;
            else throw Except(e.n);
        }
    }

    closedir(dir);
}

void SpriteManager::load_image(const std::string &filename) {
    static const boost::regex e("(\\A|\\A.*/)(\\w+)(-(\\d+)(x(\\d+))?)?\\.(png|jpg)\\Z");
    boost::smatch what;
    if (not regex_match(filename,what,e)) throw Except(Except::SS_SPRITE_LOADING_ERR);
    if (idmap.find(what[2])!=idmap.end()) throw Except(Except::SS_SPRITE_DUPLICATE_ERR);

    if (currentid>=maxid-1) throw Except(Except::SS_SPRITE_TOO_MANY_ERR);

    SDL_Surface *surf=IMG_Load(filename.c_str());
    if (not surf) throw Except(Except::SS_SPRITE_LOADING_ERR);

    if (surf->format->BitsPerPixel!=32) { SDL_FreeSurface(surf); throw Except(Except::SS_SPRITE_CONVERSION_ERR); }
    glBindTexture(GL_TEXTURE_2D,ids[currentid]);
    glTexImage2D(GL_TEXTURE_2D,0,4,surf->w,surf->h,0,GL_RGBA,GL_UNSIGNED_BYTE,static_cast<unsigned char*>(surf->pixels));
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    if (what[3].matched and what[5].matched) idmap[what[2]]=Record(ids[currentid],surf,atoi(std::string(what[4]).c_str()),atoi(std::string(what[6]).c_str())); //animated with state
    else if (what[3].matched) idmap[what[2]]=Record(ids[currentid],surf,atoi(std::string(what[4]).c_str())); //state
    else idmap[what[2]]=Record(ids[currentid],surf);
    currentid++;
}

Sprite *SpriteManager::get_sprite(const std::string &name) const {
    IdMap::const_iterator match=idmap.find(name);
    if (match==idmap.end()) throw Except(Except::SS_SPRITE_UNKNOWN_ERR);

    switch (match->second.type) {
    case Record::STATIC:
        return new Sprite(match->second.id,match->second.surface->w,match->second.surface->h,match->first); break;
    case Record::STATE:
        return new StateSprite(match->second.id,match->second.surface->w,match->second.surface->h,match->first,match->second.nstate); break;
    case Record::ANIMATED:
        return new AnimatedSprite(match->second.id,match->second.surface->w,match->second.surface->h,match->first,match->second.nstate,match->second.nframe); break;
    }


}

void SpriteManager::dump(std::ostream &os) const {
    os<<currentid<<"/"<<maxid<<" sprites"<<endl;
    for (IdMap::const_iterator i=idmap.begin(); i!=idmap.end(); i++) {
        os<<"* "<<i->first<<" id="<<i->second.id<<" size="<<i->second.surface->w<<"x"<<i->second.surface->h<<" ";
        switch (i->second.type) {
        case Record::STATIC:
            os<<"static"; break;
        case Record::STATE:
            os<<"state "<<i->second.nstate; break;
        case Record::ANIMATED:
            os<<"animated "<<i->second.nstate<<"x"<<i->second.nframe; break;
        }
        os<<endl;
    }
}

