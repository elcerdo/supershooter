#include "shoot.h"

#include <cmath>
#include <sstream>
#include "collision.h"
#include "message.h"
#include "except.h"
#include <iostream>
#include <algorithm>
#include <typeinfo>
using std::cout;
using std::endl;

std::ostream &operator<<(std::ostream &os, const TiXmlElement *elem) {
    if (elem) {
        os<<"["<<elem->Value();
        TiXmlAttribute *att=const_cast<TiXmlElement*>(elem)->FirstAttribute();
        if (att) { os<<"|"<<att->Name(); if (att->Value()) os<<"="<<att->Value(); att=att->Next(); }
        while (att) { os<<" "<<att->Name(); if (att->Value()) os<<"="<<att->Value(); att=att->Next(); }
        return os<<"]";
    }
    return os<<"[NULL]";
}

//***********************************************************
Ship::Ship(float health,long int score) : body(NULL), health(health), score(score) {}
Ship::Ship(Sprite *body,float health,long int score) : body(body), health(health), score(score) {}
Ship::~Ship() { delete body; }
void Ship::draw(float dt) const { body->draw(dt); }
float Ship::get_x() const { return body->x; }
float Ship::get_y() const { return body->y; }
float Ship::get_left() const { 
    float dx0=(body->w*body->factorx*cos(body->angle)+body->h*body->factory*sin(body->angle))/2.;
    float dx1=(body->w*body->factorx*cos(body->angle)-body->h*body->factory*sin(body->angle))/2.;
    if (dx0<0) dx0=-dx0;
    if (dx1<0) dx1=-dx1;
    return dx0<dx1 ? body->x-dx1 : body->x-dx0;
}
float Ship::get_right() const { 
    float dx0=(body->w*body->factorx*cos(body->angle)-body->h*body->factory*sin(body->angle))/2.;
    float dx1=(body->w*body->factorx*cos(body->angle)+body->h*body->factory*sin(body->angle))/2.;
    if (dx0<0) dx0=-dx0;
    if (dx1<0) dx1=-dx1;
    return dx0<dx1 ? body->x+dx1 : body->x+dx0;
}
float Ship::get_top() const {
    float dy0=(body->w*body->factorx*sin(body->angle)+body->h*body->factory*cos(body->angle))/2.;
    float dy1=(-body->w*body->factorx*sin(body->angle)+body->h*body->factory*cos(body->angle))/2.;
    if (dy0<0) dy0=-dy0;
    if (dy1<0) dy1=-dy1;
    return dy0<dy1 ? body->y-dy1 : body->y-dy0;
}
float Ship::get_bottom() const {
    float dy0=(body->w*body->factorx*sin(body->angle)+body->h*body->factory*cos(body->angle))/2.;
    float dy1=(-body->w*body->factorx*sin(body->angle)+body->h*body->factory*cos(body->angle))/2.;
    if (dy0<0) dy0=-dy0;
    if (dy1<0) dy1=-dy1;
    return dy0<dy1 ? body->y+dy1 : body->y+dy0;
}
bool Ship::collide_with(const Point *point) const {
    float dx=(point->get_x()-body->x);
    float dy=(point->get_y()-body->y);
    float lx=(dx*cos(body->angle)+dy*sin(body->angle))*2./body->factorx;
    if (lx<0) lx=-lx;
    if (lx>body->w) return false;
    float ly=(-dx*sin(body->angle)+dy*cos(body->angle))*2./body->factory;
    if (ly<0) ly=-ly;
    if (ly>body->h) return false;
    return true;
}

XmlShip::XmlShip(Sprite *aa,const Sprites &sprites,TiXmlElement *main,float health,long int score) : Ship(aa,health,score), sprites(sprites), current(main), t(0), speed(0), wait(0) {}

bool XmlShip::move(float dt) {
    if (body->x>SdlManager::get()->width+512 or body->x<-512 or body->y>SdlManager::get()->height+512 or body->y<-512) return false;

    body->x+=dt*speed*cos(body->angle);
    body->y+=dt*speed*sin(body->angle);

    exec();

    if (wait>0) wait-=dt;
    if (wait<0) wait=0;

    return true;
}

void XmlShip::exec() {
    if (wait>0) return;

    if (current) {
        if (current->ValueStr()=="program") {
            int repeat=1;
            current->QueryValueAttribute("repeat",&repeat);
            stack.push(std::make_pair(current,repeat));
            current=current->FirstChildElement();
        } else if (current->ValueStr()=="wait") {
            float time=1.;
            current->QueryValueAttribute("time",&time);
            wait=time;
            current=current->NextSiblingElement();
        } else if (current->ValueStr()=="position") {

            std::string id;
            if (current->QueryValueAttribute("id",&id)==TIXML_SUCCESS) {
                Sprites::iterator foo=sprites.find(id);
                if (foo==sprites.end()) throw Except(Except::SS_XML_ID_UNKNOWN_ERR,id);
                Sprite *select=foo->second;
                current->QueryValueAttribute("x",&select->x);
                current->QueryValueAttribute("y",&select->y);
                if (current->QueryValueAttribute("angle",&select->angle)==TIXML_SUCCESS) select->angle*=M_PI/180.;
            } else {
                current->QueryValueAttribute("x",&body->x);
                current->QueryValueAttribute("y",&body->y);
                if (current->QueryValueAttribute("angle",&body->angle)==TIXML_SUCCESS) body->angle*=M_PI/180.;
            }

            current=current->NextSiblingElement();
        } else if (current->ValueStr()=="positionrel") {
            float _x=0,_y=0,_angle=0;
            current->QueryValueAttribute("x",&_x);
            current->QueryValueAttribute("y",&_y);
            if (current->QueryValueAttribute("angle",&_angle)==TIXML_SUCCESS) _angle*=M_PI/180.;

            std::string id;
            if (current->QueryValueAttribute("id",&id)==TIXML_SUCCESS) {
                Sprites::iterator foo=sprites.find(id);
                if (foo==sprites.end()) throw Except(Except::SS_XML_ID_UNKNOWN_ERR,id);
                Sprite *select=foo->second;
                select->x+=_x;
                select->y+=_y;
                select->angle+=_angle;
            } else {
                body->x+=_x;
                body->y+=_y;
                body->angle+=_angle;
            }

            current=current->NextSiblingElement();
        } else if (current->ValueStr()=="shoot") {
            float _rangle=0,_speed=300.;
            std::string _name="bullet01";
            current->QueryValueAttribute("speed",&_speed);
            current->QueryValueAttribute("name",&_name);
            if (current->QueryValueAttribute("anglerel",&_rangle)==TIXML_SUCCESS) _rangle*=M_PI/180.;

            std::string id;
            Bullet *bullet;
            if (current->QueryValueAttribute("id",&id)==TIXML_SUCCESS) {
                Sprites::iterator foo=sprites.find(id);
                if (foo==sprites.end()) throw Except(Except::SS_XML_ID_UNKNOWN_ERR,id);
                Sprite *select=foo->second;
                bullet=BulletManager::get()->shoot_from_sprite(select,_rangle,_speed,1,_name);
            } else bullet=BulletManager::get()->shoot_from_sprite(body,_rangle,_speed,1,_name);

            if (StateSprite *cast=dynamic_cast<StateSprite*>(bullet->sprite)) current->QueryValueAttribute("sprstate",&cast->state);
            if (AnimatedSprite *cast=dynamic_cast<AnimatedSprite*>(bullet->sprite)) {
                current->QueryValueAttribute("sprspeed",&cast->speed);
                current->QueryValueAttribute("sprrepeat",&cast->repeat);
                current->QueryValueAttribute("sprlength",&cast->length);
            }

            current=current->NextSiblingElement();
        } else if (current->ValueStr()=="speed") {
            current->QueryValueAttribute("value",&speed);
            current=current->NextSiblingElement();
        } else {
            std::cerr<<"unknow ship order "<<current<<endl;
            current=current->NextSiblingElement();
        }
    } else if (not stack.empty()) {
        ExecutionStack::value_type top=stack.top();
        stack.pop();

        top.second--;
        if (top.second) {
            current=top.first->FirstChildElement();
            stack.push(top);
        } else {
            if (not stack.empty()) current=top.first->NextSiblingElement();
            else current=NULL;
        }

    } 

    if (current or not stack.empty()) exec();
}

//***********************************************************
static ShipManager *mShipManager=NULL;

ShipManager *ShipManager::get() { return mShipManager; }
void ShipManager::free() { if (mShipManager) { delete mShipManager; mShipManager=NULL; } }
void ShipManager::init(size_t nspace,const std::string &configfile) {
    if (mShipManager) throw Except(Except::SS_INIT_ERR);
    mShipManager=new ShipManager(nspace,configfile);
}

ShipManager::ShipManager(size_t nspace,const std::string &configfile) : spaces(nspace), ncreated(0), ndestroyed(0), current(NULL), wait(0), score(0) {
    xml_assert(config.LoadFile(configfile));

    TiXmlElement *root=config.FirstChildElement("config");
    xml_assert(root);

    for (TiXmlElement *ships=root->FirstChildElement("ships"); ships; ships=ships->NextSiblingElement("ships"))
    for (TiXmlElement *ship=ships->FirstChildElement("ship"); ship; ship=ship->NextSiblingElement("ship")) {
        std::string id;
        xml_assert(ship->QueryValueAttribute("id",&id)==TIXML_SUCCESS);
        if (shipnodes.find(id)!=shipnodes.end()) throw Except(Except::SS_XML_ID_DUPLICATE_ERR,id);
        shipnodes[id]=ship;
    }

    for (TiXmlElement *waves=root->FirstChildElement("waves"); waves; waves=waves->NextSiblingElement("waves"))
    for (TiXmlElement *wave=waves->FirstChildElement("wave"); wave; wave=wave->NextSiblingElement("wave")) {
        std::string id;
        xml_assert(wave->QueryValueAttribute("id",&id)==TIXML_SUCCESS);
        if (wavenodes.find(id)!=wavenodes.end()) throw Except(Except::SS_XML_ID_DUPLICATE_ERR,id);
        wavenodes[id]=wave;
    }
}

ShipManager::~ShipManager() {
    unregister_self();
    cout<<ncreated<<" ships created, "<<ndestroyed<<" ships destroyed: ";
    if (ncreated==ndestroyed) cout<<"no leak detected"<<endl;
    else cout<<"leak detected"<<endl;
}

void ShipManager::unregister_self() {
    size_t kspace=0;
    for (Spaces::iterator ships=spaces.begin(); ships!=spaces.end(); ships++) {
        for (Ships::const_iterator i=ships->begin(); i!=ships->end(); i++) {
            CollisionManager::get()->spaces[kspace].second.erase(*i);
            delete *i;
        }
        ndestroyed+=ships->size();
        ships->clear();
        kspace++;
    }
    
    for (Explosions::const_iterator i=explosions.begin(); i!=explosions.end(); i++) delete i->second;
    explosions.clear();
}

void ShipManager::flush_waves() { MessageManager::get()->add_message("flushing waves"); current=NULL; while (not stack.empty()) stack.pop_front(); }
void ShipManager::flush_ships() { unregister_self(); }

bool ShipManager::frame_entered(float t,float dt) {
    for (Explosions::iterator i=explosions.begin(); i!=explosions.end() and t>i->first+1.;) { //explosions last one second and are ordered
        Explosions::iterator ii=i++;
        delete ii->second; explosions.erase(ii);
    }
    for (Explosions::const_iterator i=explosions.begin(); i!=explosions.end(); i++) i->second->draw(dt);

    size_t kspace=0;
    for (Spaces::iterator ships=spaces.begin(); ships!=spaces.end(); ships++) {
        for (Ships::iterator ii=ships->begin(); ii!=ships->end();) {
            Ships::iterator i=ii++;
            if ((*i)->health<0 or not (*i)->move(dt)) {
                Sprite *sprite=SpriteManager::get()->get_sprite("boom00");
                sprite->x=(*i)->get_x();
                sprite->y=(*i)->get_y();
                sprite->z=7.;
                if (AnimatedSprite *cast=dynamic_cast<AnimatedSprite*>(sprite)) { cast->speed=15.; cast->repeat=false; };
                explosions.insert(std::make_pair(t,sprite));

                score+=(*i)->score;
                delete *i;
                CollisionManager::get()->spaces[kspace].second.erase(*i);
                ships->erase(i);
                ndestroyed++;
                continue;
            }
            (*i)->draw(dt);
        }
        kspace++;
    }

    if (wait>0) wait-=dt;
    if (wait<0) wait=0;

    exec();
    return true;
}

void ShipManager::exec() {
    if (wait>0) return;

    if (current) {
        if (current->ValueStr()=="wave") {
            int repeat=1;
            current->QueryValueAttribute("repeat",&repeat);
            stack.push_front(std::make_pair(current,repeat));
            current=current->FirstChildElement();
        } else if (current->ValueStr()=="spawn") {
            std::string shipid,prgid="main";
            float x=.5,y=-.05,angle=90;
            current->QueryValueAttribute("id",&shipid);
            current->QueryValueAttribute("program",&prgid);
            current->QueryValueAttribute("x",&x);
            current->QueryValueAttribute("y",&y);
            current->QueryValueAttribute("angle",&angle);
            x*=SdlManager::get()->width;
            y*=SdlManager::get()->height;
            angle*=M_PI/180.;
            launch_enemy_ship(shipid,prgid,x,y,angle);

            current=current->NextSiblingElement();
        } else if (current->ValueStr()=="line") {
            std::string shipid,prgid="main";
            float sx=.25,sy=-.05;
            float ex=.25,ey=-.05;
            float angle=90;
            unsigned int n=10;
            current->QueryValueAttribute("id",&shipid);
            current->QueryValueAttribute("program",&prgid);
            current->QueryValueAttribute("startx",&sx);
            current->QueryValueAttribute("starty",&sy);
            current->QueryValueAttribute("endx",&ex);
            current->QueryValueAttribute("endy",&ey);
            current->QueryValueAttribute("n",&n);
            current->QueryValueAttribute("angle",&angle);
            sx*=SdlManager::get()->width;
            sy*=SdlManager::get()->height;
            ex*=SdlManager::get()->width;
            ey*=SdlManager::get()->height;
            angle*=M_PI/180.;
            for (unsigned int k=0; k<n; k++) {
                float x=sx+(ex-sx)*static_cast<float>(k)/(n-1.);
                float y=sy+(ey-sy)*static_cast<float>(k)/(n-1.);
                launch_enemy_ship(shipid,prgid,x,y,angle);
            }

            current=current->NextSiblingElement();
        } else if (current->ValueStr()=="wait") {
            float time=1.;
            current->QueryValueAttribute("time",&time);
            wait=time;
            current=current->NextSiblingElement();
        } else {
            std::cerr<<"unknow wave order "<<current<<endl;
            current=current->NextSiblingElement();
        }
    } else if (not stack.empty()) {
        ExecutionStack::value_type top=stack.front();
        stack.pop_front();

        top.second--;
        if (top.second) {
            std::stringstream message;
            std::string prg;
            top.first->QueryValueAttribute("id",&prg);
            if (not prg.empty()) {
                message<<"entering "<<prg<<" ("<<top.second-1<<" remaining)";
                MessageManager::get()->add_message(message.str());
            }

            current=top.first->FirstChildElement();
            stack.push_front(top);
        } else {
            if (not stack.empty()) current=top.first->NextSiblingElement();
            else current=NULL;
            if (not current) MessageManager::get()->add_message("end of wave");
        }

    } 

    if (current or not stack.empty()) exec();
};

bool ShipManager::wave_finished() const { return (not current and stack.empty() and spaces[0].empty()); }

void ShipManager::add_ship(Ship *ship,size_t kspace) {
    spaces[kspace].insert(ship);
    CollisionManager::get()->spaces[kspace].second.insert(ship);
    
    ncreated++;
}

void ShipManager::schedule_wave(const std::string &id) {
    NodeMap::iterator foo=wavenodes.find(id);
    if (foo==wavenodes.end()) throw Except(Except::SS_XML_ID_UNKNOWN_ERR,id);

    int repeat=1;
    foo->second->QueryValueAttribute("repeat",&repeat);
    repeat++; //wave is popped one time before being executed
    stack.push_back(std::make_pair(foo->second,repeat));
}



XmlShip *ShipManager::launch_enemy_ship(const std::string &id,const std::string &prgid,float x,float y,float angle) {
    NodeMap::iterator foo=shipnodes.find(id);
    if (foo==shipnodes.end()) throw Except(Except::SS_XML_ID_UNKNOWN_ERR,id);

    float health;
    long int score;
    xml_assert(foo->second->QueryValueAttribute("health",&health)==TIXML_SUCCESS);
    xml_assert(foo->second->QueryValueAttribute("score",&score)==TIXML_SUCCESS);

    XmlShip::Sprites sprites;
    Sprite *body=parse_sprite(foo->second->FirstChildElement("sprite"),sprites);

    TiXmlElement *program=NULL;
    for (TiXmlElement *i=foo->second->FirstChildElement("program"); i; i=i->NextSiblingElement("program")) {
        std::string current_prgid;
        i->QueryValueAttribute("id",&current_prgid);
        if (prgid==current_prgid) { program=i; break; }
    }
    if (not program) throw Except(Except::SS_XML_ID_UNKNOWN_ERR,prgid);

    XmlShip *ship=new XmlShip(body,sprites,program,health,score);
    ship->body->x=x;
    ship->body->y=y;
    ship->body->angle=angle;
    add_ship(ship,0); //add as enemy
    return ship;
}

Sprite *ShipManager::parse_sprite(TiXmlElement *node,XmlShip::Sprites &sprites,Sprite *parent) const {
    std::string name;
    xml_assert(node->QueryValueAttribute("name",&name)==TIXML_SUCCESS);

    Sprite *body;
    if (parent) body=parent->create_child(name);
    else body=SpriteManager::get()->get_sprite(name);

    std::string id;
    node->QueryValueAttribute("id",&id);
    if (not id.empty()) {
        if (sprites.find(id)!=sprites.end()) throw Except(Except::SS_XML_ID_DUPLICATE_ERR);
        sprites[id]=body;
    }

    node->QueryValueAttribute("z",&body->z);
    node->QueryValueAttribute("x",&body->x);
    node->QueryValueAttribute("y",&body->y);
    node->QueryValueAttribute("cx",&body->cx);
    node->QueryValueAttribute("cy",&body->cy);
    if (node->QueryValueAttribute("angle",&body->angle)==TIXML_SUCCESS) body->angle*=M_PI/180.;
    if (StateSprite *cast=dynamic_cast<StateSprite*>(body)) node->QueryValueAttribute("state",&cast->state);
    if (AnimatedSprite *cast=dynamic_cast<AnimatedSprite*>(body)) {
        node->QueryValueAttribute("speed",&cast->speed);
        node->QueryValueAttribute("repeat",&cast->repeat);
        node->QueryValueAttribute("length",&cast->length);
    }

    for (TiXmlElement *child=node->FirstChildElement("sprite"); child; child=child->NextSiblingElement("sprite"))
        parse_sprite(child,sprites,body);

    if (parent) return NULL;
    return body;
}

void ShipManager::dump(std::ostream &os) const {
    os<<shipnodes.size()<<" ships, "<<wavenodes.size()<<" waves"<<endl;
}

void ShipManager::xml_assert(bool v) const {
    if (v) return;
    std::stringstream ss;
    ss<<config.ErrorDesc()<<" col="<<config.ErrorCol()<<" row="<<config.ErrorRow();
    throw Except(Except::SS_XML_PARSING_ERR,ss.str());
}

//***********************************************************
Bullet::Bullet(Sprite *sprite,float angle,float speed,float damage) : sprite(sprite), vx(speed*cos(angle)), vy(speed*sin(angle)), damage(damage) { sprite->angle=angle;}
Bullet::~Bullet() { delete sprite; }
float Bullet::get_x() const { return sprite->x; }
float Bullet::get_y() const { return sprite->y; }
void Bullet::move(float dt) { sprite->x+=dt*vx; sprite->y+=dt*vy; }

static BulletManager *mBulletManager=NULL;

BulletManager *BulletManager::get() { return mBulletManager; }
void BulletManager::free() { if (mBulletManager) { delete mBulletManager; mBulletManager=NULL; } }
void BulletManager::init(size_t nspace) {
    if (mBulletManager) throw Except(Except::SS_INIT_ERR);
    mBulletManager=new BulletManager(nspace);
}

BulletManager::BulletManager(size_t nspace) : spaces(nspace), ncreated(0), ndestroyed(0) {}
BulletManager::~BulletManager() {
    unregister_self();
    cout<<ncreated<<" bullets created, "<<ndestroyed<<" bullets destroyed: ";
    if (ncreated==ndestroyed) cout<<"no leak detected"<<endl;
    else cout<<"leak detected"<<endl;
}

void BulletManager::flush_bullets() { unregister_self(); }
void BulletManager::unregister_self() {
    size_t kspace=0;
    for (Spaces::iterator bullets=spaces.begin(); bullets!=spaces.end(); bullets++) {
        for (Bullets::const_iterator i=bullets->begin(); i!=bullets->end(); i++) {
            delete *i;
            CollisionManager::get()->spaces[kspace].first.erase(*i);
        }
        ndestroyed+=bullets->size();
        bullets->clear();
        kspace++;
    }
}

bool BulletManager::frame_entered(float t,float dt) {
    move(dt);

    CollisionManager::get()->resolve_collision();
    //CollisionManager::get()->dump();
    for (size_t k=0; k<spaces.size(); k++) {
        CollisionManager::Space &space=CollisionManager::get()->spaces[k];
        Bullets &bullets=spaces[k];

        Area::Points foo;
        for (CollisionManager::Areas::const_iterator i=space.second.begin(); i!=space.second.end(); i++) {
            const Area *area=*i;
            std::set_union(area->colliding.begin(),area->colliding.end(),foo.begin(),foo.end(),std::inserter(foo,foo.begin()));
            //cout<<(dynamic_cast<const Ship*>(area)!=NULL)<<" "<<typeid(*area).before(typeid(Ship))<<endl;
            if (const Ship *ship=dynamic_cast<const Ship*>(area)) { //area is a ship so deal the damage //FIXME use of typeid is time constant
                for (Area::Points::const_iterator j=ship->colliding.begin(); j!=ship->colliding.end(); j++) {
                    if (const Bullet *bullet=dynamic_cast<const Bullet*>(*j)) {
                        const_cast<Ship*>(ship)->health-=bullet->damage;
                    }
                }
            }


        }
        //cout<<"---------"<<endl;

        for (Area::Points::const_iterator j=foo.begin(); j!=foo.end(); j++) {
           space.first.erase(*j);
           bullets.erase(dynamic_cast<Bullet*>(*j));
           delete *j; 
           ndestroyed++;
        }
    }

    draw(dt);
    return true;
}

Bullet *BulletManager::shoot(float x,float y,float angle,float speed,size_t kspace,const std::string &name,float damage) {
    Sprite *sprite=SpriteManager::get()->get_sprite(name);
    sprite->x=x;
    sprite->y=y;
    Bullet *bullet=new Bullet(sprite,angle,speed,damage);
    spaces[kspace].insert(bullet);
    CollisionManager::get()->spaces[kspace].first.insert(bullet);
    ncreated++;
    return bullet;
}

Bullet *BulletManager::shoot_from_sprite(const Sprite *sprite,float rangle,float speed,size_t kspace,const std::string &name,float damage) {
    float ax,ay,aangle,afactorx,afactory;
    sprite->absolute_coordinates(ax,ay,aangle,afactorx,afactory);
    return shoot(ax,ay,aangle+rangle,speed,kspace,name,damage);
}

void BulletManager::move(float dt) { 
    size_t kspace=0;
    for (Spaces::iterator j=spaces.begin(); j!=spaces.end(); j++) {
        Bullets &bullets=*j;
        for (Bullets::iterator ii=bullets.begin(); ii!=bullets.end();) {
            Bullets::iterator i=ii++;
            Bullet *bullet=*i;
            bullet->move(dt);
            if (bullet->sprite->x<-10 or bullet->sprite->x>SdlManager::get()->width+10 or bullet->sprite->y<-10 or bullet->sprite->y>SdlManager::get()->height+10) {
                delete bullet;
                CollisionManager::get()->spaces[kspace].first.erase(bullet);
                bullets.erase(i);
                ndestroyed++;
            }
        }
        kspace++;
    }
}

void BulletManager::draw(float dt) const {
    for (Spaces::const_iterator bullets=spaces.begin(); bullets!=spaces.end(); bullets++)
    for (Bullets::const_iterator i=bullets->begin(); i!=bullets->end(); i++)
        (*i)->sprite->draw(dt);
}

