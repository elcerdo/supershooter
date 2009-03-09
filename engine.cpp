#include "engine.h"

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <iostream>
#include <cassert>
#include <utility>
#include <boost/regex.hpp>
#include "except.h"
using std::endl;
using std::cerr;
using std::cout;

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
Sprite::Sprite(unsigned int id,double w,double h) : id(id), x(0), y(0), angle(0), factorx(1), factory(1), w(w), h(h) {}
void Sprite::draw() const {
    glBindTexture(GL_TEXTURE_2D,id);
    glPushMatrix();
        glTranslatef(x+w/2,y+h/2,0.0);
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
    for (IdMap::const_iterator i=idmap.begin(); i!=idmap.end(); i++) SDL_FreeSurface(i->second.second);
}

void SpriteManager::load_image(const std::string &filename) {
    static const boost::regex e("(\\A|\\A.*/)(\\w+)\\.(png|jpg)\\Z");
    boost::smatch what;
    if (not regex_match(filename,what,e)) throw Except(Except::SS_SPRITE_ERR);
    if (idmap.find(what[2])!=idmap.end()) throw Except(Except::SS_SPRITE_ERR);

    if (currentid>=maxid-1) throw Except(Except::SS_TOO_MANY_SPRITES_ERR);

    SDL_Surface *surf=IMG_Load(filename.c_str());
    if (not surf) { cerr<<"error loading '"<<filename<<"': "<<IMG_GetError()<<endl; throw Except(Except::SS_LOADING_ERR); }

    if (surf->format->BitsPerPixel!=32) { SDL_FreeSurface(surf); throw Except(Except::SS_CONVERSION_ERR); }
    glBindTexture(GL_TEXTURE_2D,ids[currentid]);
    glTexImage2D(GL_TEXTURE_2D,0,4,surf->w,surf->h,0,GL_RGBA,GL_UNSIGNED_BYTE,static_cast<unsigned char*>(surf->pixels));
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    idmap[what[2]]=std::make_pair(ids[currentid],surf);
    currentid++;
}

Sprite *SpriteManager::get_sprite(const std::string &name) const {
    IdMap::const_iterator match=idmap.find(name);
    if (match==idmap.end()) throw Except(Except::SS_SPRITE_ERR);
    return new Sprite(match->second.first,match->second.second->w,match->second.second->h);
}

void SpriteManager::dump() const {
    cout<<currentid<<"/"<<maxid<<" sprites"<<endl;
    for (IdMap::const_iterator i=idmap.begin(); i!=idmap.end(); i++) cout<<i->first<<" "<<i->second.first<<" "<<i->second.second->w<<"x"<<i->second.second->h<<endl;
}
