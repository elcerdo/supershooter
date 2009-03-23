#include "engine.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <cassert>
#include <utility>
#include <boost/regex.hpp>
#include <fstream>
#include <cmath>
#include <dirent.h>
#include "except.h"
using std::endl;
using std::cerr;
using std::cout;

static SdlManager *mSdlManager=NULL;

SdlManager *SdlManager::get() { return mSdlManager; }
void SdlManager::free() { if (mSdlManager) { delete mSdlManager; mSdlManager=NULL; } }
void SdlManager::init(int w,int h,int d) {
    if (mSdlManager) throw Except(Except::SS_INIT_ERR);
    mSdlManager=new SdlManager(w,h,d);
}

SdlManager::SdlManager(int w,int h,int d) : in_main_loop(false), width(w), height(h), old_ticks(0) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_OPENGL)) { cerr<<"cannot initialize sdl..."<<endl; throw Except(Except::SS_INIT_ERR); }

    screen=SDL_SetVideoMode(width,height,d,SDL_OPENGL|SDL_DOUBLEBUF);
    if (not screen) { cerr<<"cannot create sdl screen..."<<endl; throw Except(Except::SS_INIT_ERR); }

    SDL_SetEventFilter(&event_filter);

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0,0,0,0);
	glEnable(GL_DEPTH_TEST);// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);// The Type Of Depth Testing To Do
	glEnable(GL_BLEND);//Alpha blending
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// Really Nice Perspective Calculations
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.05);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,w,h,0,-10.0,10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

const unsigned char *SdlManager::get_key_state() const { return SDL_GetKeyState(NULL); }
void SdlManager::get_mouse_position(float &x,float &y) const {
    int xx,yy;
    SDL_GetMouseState(&xx,&yy);
    x=xx; y=yy;
}
void SdlManager::set_background_color(float r,float g,float b) { glClearColor(r,g,b,0); }

SdlManager::~SdlManager() {
    SDL_FreeSurface(screen);
    SDL_Quit();
}

void SdlManager::register_listener(Listener *listener) { //FIXME duplicate
    if (in_main_loop) listener->register_self();
    listeners.insert(listener);
}

void SdlManager::unregister_listener(Listener *listener) { //FIXME existance
    listeners.erase(listener);
    if (in_main_loop) listener->unregister_self();
}

int SdlManager::event_filter(const SDL_Event *ev) {
    if (ev->type==SDL_MOUSEMOTION or ev->type==SDL_ACTIVEEVENT or ev->type==SDL_VIDEOEXPOSE) return 0;
    return 1;
}

void SdlManager::main_loop() {
    in_main_loop=true;

    for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end(); i++) (*i)->register_self();

    old_ticks=SDL_GetTicks();
    bool quit=false;
    while (not quit and not listeners.empty()) {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        SDL_Event event;
        while (SDL_PollEvent(&event) and not quit) {
            switch (event.type) {
            case SDL_KEYDOWN:
                for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end() and not quit; i++) quit=not (*i)->key_down(event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end() and not quit; i++) quit=not (*i)->key_up(event.key.keysym.sym);
                break;
            case SDL_MOUSEBUTTONDOWN:
                for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end() and not quit; i++) quit=not (*i)->mouse_down(event.button.button,event.button.x,event.button.y);
                break;
            case SDL_MOUSEBUTTONUP:
                for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end() and not quit; i++) quit=not (*i)->mouse_up(event.button.button,event.button.x,event.button.y);
                break;
            case SDL_QUIT:      
                quit=true;
                break;
            default:
                cout<<"unhandled event "<<static_cast<int>(event.type)<<endl;
                break;
            }
        }

        long int ticks=SDL_GetTicks();
        float t=ticks/1000.;
        float dt=(ticks-old_ticks)/1000.;
        old_ticks=ticks;
        for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end() and not quit; i++) quit=not (*i)->frame_entered(t,dt);

        SDL_GL_SwapBuffers();  
        SDL_Flip(screen);

        SDL_Delay(5);
    }

    for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end(); i++) (*i)->unregister_self();

    in_main_loop=false;
}

//***********************************************************
static long int nsprites_created=0;
static long int nsprites_destroyed=0;

Sprite::Sprite(unsigned int id,float w,float h,const std::string &name) : id(id), x(0), y(0), z(0), angle(0), factorx(1), factory(1), w(w), h(h), cx(0), cy(0), name(name), parent(NULL), alpha(1.) {}

Sprite::~Sprite() {
    nsprites_destroyed++;
    for (Children::const_iterator i=children.begin(); i!=children.end(); i++) delete *i;
}

Sprite *Sprite::create_child(const std::string &name) {
    Sprite *child=SpriteManager::get()->get_sprite(name);
    child->parent=this;
    children.insert(child);
    return child;
}

void Sprite::absolute_coordinates(float &ax,float &ay,float &aangle,float &afactorx, float &afactory) const {
    if (not parent) { ax=x; ay=y; aangle=angle; afactorx=factorx; afactory=factory; }
    else {
        parent->absolute_coordinates(ax,ay,aangle,afactorx,afactory);
        ax+=afactorx*x*cos(aangle)-afactory*y*sin(aangle);
        ay+=afactorx*x*sin(aangle)+afactory*y*cos(aangle);
        aangle+=angle;
        afactorx*=factorx;
        afactory*=factory;
    }
}

void Sprite::draw(float dt) const {
    glBindTexture(GL_TEXTURE_2D,id);
    glPushMatrix();
        glTranslatef(x,y,0.0);
        glRotatef(180./M_PI*angle,0.0,0.0,1.0);
        glNormal3f(0.0,0.0,1.0);
        glScalef(factorx,factory,1);
        glColor4f(1,1,1,alpha);
        glBegin(GL_QUADS);
        glTexCoord2f(1.0,1.0); glVertex3f(cx+w/2,cy+h/2,z);
        glTexCoord2f(1.0,0.0); glVertex3f(cx+w/2,cy-h/2,z);
        glTexCoord2f(0.0,0.0); glVertex3f(cx-w/2,cy-h/2,z);
        glTexCoord2f(0.0,1.0); glVertex3f(cx-w/2,cy+h/2,z);
        glEnd();
        for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->draw(dt);
    glPopMatrix();
}

void Sprite::dump(std::ostream &os,const std::string &indent) const {
    os<<indent<<name<<" ["<<x<<","<<y<<"] static"<<endl;
    for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->dump(os,indent+"--");
}

StateSprite::StateSprite(unsigned int id,float w,float h,const std::string &name,unsigned int nstate) : Sprite(id,w,h/nstate,name), nstate(nstate), rh(1./nstate), state(0) {}

void StateSprite::draw(float dt) const {
    float ya=rh*state;
    float yb=ya+rh;
    glBindTexture(GL_TEXTURE_2D,id);
    glPushMatrix();
        glTranslatef(x,y,0.0);
        glRotatef(180./M_PI*angle,0.0,0.0,1.0);
        glNormal3f(0.0,0.0,1.0);
        glScalef(factorx,factory,1);
        glColor4f(1,1,1,alpha);
        glBegin(GL_QUADS);
        glTexCoord2f(1.0,yb); glVertex3f(cx+w/2,cy+h/2,z);
        glTexCoord2f(1.0,ya); glVertex3f(cx+w/2,cy-h/2,z);
        glTexCoord2f(0.0,ya); glVertex3f(cx-w/2,cy-h/2,z);
        glTexCoord2f(0.0,yb); glVertex3f(cx-w/2,cy+h/2,z);
        glEnd();
        for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->draw(dt);
    glPopMatrix();
}

void StateSprite::dump(std::ostream &os,const std::string &indent) const {
    os<<indent<<name<<" ["<<x<<","<<y<<"]@"<<state<<" state"<<endl;
    for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->dump(os,indent+"--");
}

AnimatedSprite::AnimatedSprite(unsigned int id,float w,float h,const std::string &name,unsigned int nstate,unsigned int nframe) : StateSprite(id,w/nframe,h,name,nstate), rw(1./nframe), nframe(nframe),length(nframe), repeat(true), pos(0.), speed(10.) {}

void AnimatedSprite::draw(float dt) const {
    float ya=rh*state;
    float yb=ya+rh;
    float xa=rw*static_cast<int>(pos);
    float xb=xa+rw;
    glBindTexture(GL_TEXTURE_2D,id);
    glPushMatrix();
        glTranslatef(x,y,0.0);
        glRotatef(180./M_PI*angle,0.0,0.0,1.0);
        glNormal3f(0.0,0.0,1.0);
        glScalef(factorx,factory,1);
        glColor4f(1,1,1,alpha);
        glBegin(GL_QUADS);
        glTexCoord2f(xb,yb); glVertex3f(cx+w/2,cy+h/2,z);
        glTexCoord2f(xb,ya); glVertex3f(cx+w/2,cy-h/2,z);
        glTexCoord2f(xa,ya); glVertex3f(cx-w/2,cy-h/2,z);
        glTexCoord2f(xa,yb); glVertex3f(cx-w/2,cy+h/2,z);
        glEnd();
        for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->draw(dt);
    glPopMatrix();

    if (repeat) {
        const_cast<float&>(pos)+=speed*dt;
        while (pos>=length) const_cast<float&>(pos)-=length;
    } else {
        if (pos<length-speed*dt) const_cast<float&>(pos)+=speed*dt;
    }
}

void AnimatedSprite::dump(std::ostream &os,const std::string &indent) const {
    os<<indent<<name<<" ["<<x<<","<<y<<"]@"<<state<<","<<pos<<" animated"<<endl;
}

Text::Text(unsigned int id,float w,float h,const std::string &name,const std::string &str,const CharMap &mapping,Align align) : Sprite(id,w,h,name), mapping(mapping), align(align) {
    float x=0;
    for (std::string::const_iterator istr=str.begin(); istr!=str.end(); istr++) {
        CharMap::const_iterator istate=mapping.find(*istr);
        if (istate==mapping.end()) throw Except(Except::SS_SPRITE_MAPPING_ERR,str);

        StateSprite *current=dynamic_cast<StateSprite*>(create_child(name));
        current->state=istate->second;
        current->x=x;
        current->z=5.;
        current->alpha=alpha;
        x+=w-2.;
    }

    update_align();
}

void Text::draw(float dt) const {
    glPushMatrix();
        glTranslatef(x,y,0.0);
        glRotatef(180./M_PI*angle,0.0,0.0,1.0);
        glScalef(factorx,factory,1);
        glTranslatef(cx,cy,0.0);
        for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->draw(dt);
    glPopMatrix();
}

void Text::dump(std::ostream &os,const std::string &indent) const {
    os<<indent<<name<<" ["<<x<<","<<y<<"]"<<" text"<<endl;
    for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->dump(os,indent+"--");
}

void Text::update_align() {
    switch (align) {
    case RIGHT:
        cx=(w-2.)*(1.-children.size()); break;
    case CENTER:
        cx=-(w-2.)*children.size()/2.; break;
    case LEFT:
    default:
        break;
    }
}

double Text::width() const { return factorx*(w-2.)*children.size(); }

void Text::update_alpha() { for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->alpha=alpha; }

void Text::update(const std::string &str) {
    std::string::const_iterator istr=str.begin();
    Children::const_iterator ichild=children.begin();

    float x=0;
    while (istr!=str.end() and ichild!=children.end()) {
        CharMap::const_iterator istate=mapping.find(*istr);
        if (istate==mapping.end()) throw Except(Except::SS_SPRITE_MAPPING_ERR,str);

        StateSprite *current=dynamic_cast<StateSprite*>(*ichild);
        current->state=istate->second;
        current->x=x;
        current->z=5.;
        current->alpha=alpha;
        x+=w-2.;

        ichild++;
        istr++;
    }

    while (ichild!=children.end()) { delete *ichild; children.erase(ichild++); }

    while (istr!=str.end()) {
        CharMap::const_iterator istate=mapping.find(*istr);
        if (istate==mapping.end()) throw Except(Except::SS_SPRITE_MAPPING_ERR,str);

        StateSprite *current=dynamic_cast<StateSprite*>(create_child(name));
        current->state=istate->second;
        current->x=x;
        current->z=5.;
        current->alpha=alpha;
        x+=w-2.;

        istr++;
    }

    update_align();
}

//***********************************************************
static SpriteManager *mSpriteManager=NULL;

SpriteManager *SpriteManager::get() { return mSpriteManager; }
void SpriteManager::free() { if (mSpriteManager) { delete mSpriteManager; mSpriteManager=NULL; } }
void SpriteManager::init(size_t maxid) {
    if (mSpriteManager) throw Except(Except::SS_INIT_ERR);
    mSpriteManager=new SpriteManager(maxid);
}

SpriteManager::SpriteManager(size_t maxid) : maxid(maxid), currentid(0) {
    ids=new unsigned int[maxid];
    glGenTextures(maxid,ids);

    unsigned int k=0;
    for (char i='a'; i<='z'; i++) mapping[i]=k++;
    k=0;
    for (char i='A'; i<='Z'; i++) mapping[i]=k++;
    mapping[' ']=k++;
    for (char i='0'; i<='9'; i++) mapping[i]=k++;
    mapping['.']=k++;
    mapping['-']=k++;
    mapping['[']=k;
    mapping['(']=k++;
    mapping[']']=k;
    mapping[')']=k++;
    mapping['|']=k++;
    mapping['=']=k++;
}

SpriteManager::~SpriteManager() {
    delete [] ids;
    for (IdMap::const_iterator i=idmap.begin(); i!=idmap.end(); i++) SDL_FreeSurface(i->second.surface);
    cout<<nsprites_created<<" sprites created, "<<nsprites_destroyed<<" sprites destroyed: ";
    if (nsprites_created==nsprites_destroyed) cout<<"no leak detected"<<endl;
    else cout<<"leak detected"<<endl;
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
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    if (what[3].matched and what[5].matched) idmap[what[2]]=Record(ids[currentid],surf,atoi(std::string(what[4]).c_str()),atoi(std::string(what[6]).c_str())); //animated with state
    else if (what[3].matched) idmap[what[2]]=Record(ids[currentid],surf,atoi(std::string(what[4]).c_str())); //state
    else idmap[what[2]]=Record(ids[currentid],surf);
    currentid++;
}

Sprite *SpriteManager::get_sprite(const std::string &name) const {
    IdMap::const_iterator match=idmap.find(name);
    if (match==idmap.end()) throw Except(Except::SS_SPRITE_UNKNOWN_ERR,name);

    nsprites_created++;
    switch (match->second.type) {
    case Record::STATIC:
        return new Sprite(match->second.id,match->second.surface->w,match->second.surface->h,match->first); break;
    case Record::STATE:
        return new StateSprite(match->second.id,match->second.surface->w,match->second.surface->h,match->first,match->second.nstate); break;
    case Record::ANIMATED:
        return new AnimatedSprite(match->second.id,match->second.surface->w,match->second.surface->h,match->first,match->second.nstate,match->second.nframe); break;
    }

}

Text *SpriteManager::get_text(const std::string &str,const std::string &name,Text::Align align) const {
    IdMap::const_iterator match=idmap.find(name);
    if (match==idmap.end()) throw Except(Except::SS_SPRITE_UNKNOWN_ERR,name);
    if (match->second.type==Record::STATIC) throw Except(Except::SS_SPRITE_UNKNOWN_ERR,name);

    nsprites_created++;
    return new Text(match->second.id,match->second.surface->w,match->second.surface->h,match->first,str,mapping,align);
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

