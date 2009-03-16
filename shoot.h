#ifndef __SHOOT_H__
#define __SHOOT_H__

#include "engine.h"
#include "collision.h"
#include "tinyxml/tinyxml.h"
#include <vector>
#include <set>
#include <stack>


//***********************************************************
struct Ship : public Area {
    Ship(float health);
    Ship(Sprite *body,float health);
    virtual ~Ship();
    virtual bool move(float dt)=0;
    virtual void draw(float dt) const;
    virtual float get_x() const;
    virtual float get_y() const;
    virtual float get_left() const;
    virtual float get_right() const;
    virtual float get_top() const;
    virtual float get_bottom() const;
    virtual bool collide_with(const Point *point) const;
    Sprite *body;
    float health;
};

class XmlShip : public Ship {
public:
    typedef std::map<std::string,Sprite*> Sprites;
    XmlShip(Sprite *aa,const Sprites &sprites,TiXmlElement *main,float health,bool debug=false);

    virtual bool move(float dt);
    bool debug;
protected:
    void exec();
    float speed,t,wait;

    Sprites sprites;
    TiXmlElement *current;
    typedef std::stack<std::pair<TiXmlElement*,int> > ExecutionStack;
    ExecutionStack stack;
};

class ShipManager : public Listener {
public:
    static ShipManager *get();
    static void free();
    static void init(size_t nspace=2,const std::string &configfile="config.xml");

    void add_ship(Ship *ship,size_t kspace);
    XmlShip *launch_enemy_ship(const std::string &id,const std::string &prgid,float x,float y,float angle);
    void dump(std::ostream &os=std::cout) const;
protected:
    ShipManager(size_t nspace,const std::string &configfile);
    ~ShipManager();

    virtual bool frame_entered(float t,float dt);
    virtual void unregister_self();

    typedef std::set<Ship*> Ships;
    typedef std::vector<Ships> Spaces;
    Spaces spaces;

    long int ncreated;
    long int ndestroyed;

    void xml_assert(bool v) const;
    Sprite *parse_sprite(TiXmlElement *node,XmlShip::Sprites &sprites,Sprite *parent=NULL) const;
    typedef std::map<std::string,TiXmlElement*> ShipNodes;
    ShipNodes shipnodes;

    TiXmlDocument config;
};

//***********************************************************
struct Bullet : public Point {
    Bullet(Sprite *sprite,float angle,float speed,float damage);
    virtual ~Bullet();
    virtual void move(float dt);
    virtual float get_x() const;
    virtual float get_y() const;
    Sprite *sprite;
    float vx,vy;
    float damage;
};

class BulletManager : public Listener {
public:
    static BulletManager *get();
    static void free();
    static void init(size_t nspace=2);

    Bullet *shoot(float x,float y,float angle, float speed, size_t kspace, const std::string &name="bullet00",float damage=2.);
    Bullet *shoot_from_sprite(const Sprite *sprite,float rangle, float speed, size_t kspace, const std::string &name="bullet00",float damage=2.);
protected:
    BulletManager(size_t nspace);
    ~BulletManager();

    virtual bool frame_entered(float t,float dt);
    virtual void unregister_self();
    void move(float dt);
    void draw(float dt) const;

    typedef std::set<Bullet*> Bullets;
    typedef std::vector<Bullets> Spaces;
    Spaces spaces;

    long int ncreated;
    long int ndestroyed;
};

#endif
