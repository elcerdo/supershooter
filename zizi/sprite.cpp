#include "sprite.h"

#include <boost/regex.hpp>
#include <dirent.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <cmath>
#include "except.h"
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
    os<<indent<<name<<" ["<<x<<","<<y<<","<<cx<<","<<cy<<"] static"<<std::endl;
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
    os<<indent<<name<<" ["<<x<<","<<y<<","<<cx<<","<<cy<<"]@"<<state<<" state"<<std::endl;
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
    os<<indent<<name<<" ["<<x<<","<<y<<","<<cx<<","<<cy<<"]@"<<state<<","<<pos<<" animated"<<std::endl;
}

Text::Text(unsigned int id,float w,float h,const std::string &name,const std::string &str,const CharMap &mapping,Align align) : Sprite(id,w,h,name), mapping(mapping), align(align) {
    float tx=0;
    for (std::string::const_iterator istr=str.begin(); istr!=str.end(); istr++) {
        CharMap::const_iterator istate=mapping.find(*istr);
        if (istate==mapping.end()) throw Except(Except::ZIZI_SPRITE_MAPPING_ERR,str);

        StateSprite *current=dynamic_cast<StateSprite*>(create_child(name));
        current->state=istate->second;
        current->x=tx;
        current->z=5.;
        current->alpha=alpha;
        tx+=current->w-2.;
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
    os<<indent<<name<<" ["<<x<<","<<y<<","<<cx<<","<<cy<<"]"<<" text"<<std::endl;
    for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->dump(os,indent+"--");
}

void Text::update_align() {
    if (not children.empty()) {
        const_cast<float&>(w)=factorx*((*children.begin())->w-2.)*(children.size()-1.);
        const_cast<float&>(h)=(*children.begin())->h;
    } else {
        const_cast<float&>(w)=0;
        const_cast<float&>(h)=0;
    }

    switch (align) {
    case RIGHT:
        this->cx=-w;
        break;
    case CENTER:
        this->cx=-w/2.;
        break;
    case LEFT:
    default:
        break;
    }

}

void Text::update_alpha() { for (Children::const_iterator i=children.begin(); i!=children.end(); i++) (*i)->alpha=alpha; }

void Text::update(const std::string &str) {
    std::string::const_iterator istr=str.begin();
    Children::const_iterator ichild=children.begin();

    float x=0;
    while (istr!=str.end() and ichild!=children.end()) {
        CharMap::const_iterator istate=mapping.find(*istr);
        if (istate==mapping.end()) throw Except(Except::ZIZI_SPRITE_MAPPING_ERR,str);

        StateSprite *current=dynamic_cast<StateSprite*>(*ichild);
        current->state=istate->second;
        current->x=x;
        current->z=5.;
        current->alpha=alpha;
        x+=current->w-2.;

        ichild++;
        istr++;
    }

    while (ichild!=children.end()) { delete *ichild; children.erase(ichild++); }

    while (istr!=str.end()) {
        CharMap::const_iterator istate=mapping.find(*istr);
        if (istate==mapping.end()) throw Except(Except::ZIZI_SPRITE_MAPPING_ERR,str);

        StateSprite *current=dynamic_cast<StateSprite*>(create_child(name));
        current->state=istate->second;
        current->x=x;
        current->z=5.;
        current->alpha=alpha;
        x+=current->w-2.;

        istr++;
    }

    update_align();
}

//***********************************************************
static SpriteManager *mSpriteManager=NULL;

SpriteManager *SpriteManager::get() { return mSpriteManager; }
void SpriteManager::free() { if (mSpriteManager) { delete mSpriteManager; mSpriteManager=NULL; } }
void SpriteManager::init(size_t maxid) {
    if (mSpriteManager) throw Except(Except::ZIZI_INIT_ERR,"spritemanager already exists");
    mSpriteManager=new SpriteManager(maxid);
}

SpriteManager::SpriteManager(size_t maxid) : maxid(maxid), currentid(0) {
    ids=new unsigned int[maxid];
    glGenTextures(maxid,ids);

    unsigned int k=0;
    for (char i='a'; i<='z'; i++) mapping[i]=k++;
    k=0;
    for (char i='A'; i<='Z'; i++) mapping[i]=k++;
    mapping['_']=k;
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
    std::cout<<nsprites_created<<" sprites created, "<<nsprites_destroyed<<" sprites destroyed: ";
    if (nsprites_created==nsprites_destroyed) std::cout<<"no leak detected"<<std::endl;
    else std::cout<<"leak detected"<<std::endl;
}

bool SpriteManager::load_directory(const std::string &directory) {
    DIR *dir=opendir(directory.c_str());
    if (not dir) return false;

    std::string prefix(directory);
    if (prefix.size()>0 and prefix[prefix.size()-1]!='/') prefix+='/';

    dirent *ent;
    while (ent=readdir(dir)) {
        std::string filename(ent->d_name);
        if (filename=="." or filename=="..") continue;

        try { load_image(prefix+filename);
        } catch (Except &e) {
            if (e.n!=Except::ZIZI_SPRITE_LOADING_ERR and e.n!=Except::ZIZI_SPRITE_CONVERSION_ERR) throw e;
            else e.dump();
        }
    }

    closedir(dir);
    return true;
}

void SpriteManager::load_image(const std::string &filename) {
    static const boost::regex e("(\\A|\\A.*/)(\\w+)(-(\\d+)(x(\\d+))?)?\\.(png|jpg)\\Z");
    boost::smatch what;
    if (not regex_match(filename,what,e)) throw Except(Except::ZIZI_SPRITE_LOADING_ERR,filename);
    if (idmap.find(what[2])!=idmap.end()) throw Except(Except::ZIZI_SPRITE_DUPLICATE_ERR,filename);

    if (currentid>=maxid-1) throw Except(Except::ZIZI_SPRITE_TOO_MANY_ERR,"");

    SDL_Surface *surf=IMG_Load(filename.c_str());
    if (not surf) throw Except(Except::ZIZI_SPRITE_LOADING_ERR,filename);

    if (surf->format->BitsPerPixel!=32) { SDL_FreeSurface(surf); throw Except(Except::ZIZI_SPRITE_CONVERSION_ERR,filename); }
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
    if (match==idmap.end()) throw Except(Except::ZIZI_SPRITE_UNKNOWN_ERR,name);

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
    if (match==idmap.end()) throw Except(Except::ZIZI_SPRITE_UNKNOWN_ERR,name);
    if (match->second.type==Record::STATIC) throw Except(Except::ZIZI_SPRITE_UNKNOWN_ERR,name);

    nsprites_created++;
    return new Text(match->second.id,match->second.surface->w,match->second.surface->h,match->first,str,mapping,align);
}

void SpriteManager::dump(std::ostream &os) const {
    os<<currentid<<"/"<<maxid<<" sprites"<<std::endl;
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
        os<<std::endl;
    }
}

