#include "except.h"
#include "collision.h"
#include "utils.h"
#include "message.h"
#include "sound.h"
#include <cmath>
#include <list>
#include <cassert>
using std::cerr;
using std::cout;
using std::endl;

#define GUY_SPEED 300.
#define ZOMBIE_SPEED 80.
#define BULLET_SPEED 800.
#define BULLET_RELOAD .1

struct Bullet: public Point {
    Bullet(const Sprite *guy) : x(guy->x), y(guy->y), vx(BULLET_SPEED*cos(guy->angle)), vy(BULLET_SPEED*sin(guy->angle)) {}
    virtual float get_x() const { return x; }
    virtual float get_y() const { return y; }
    float x,y;
    float vx,vy;
};

//FIXME dirty hack
struct OuterBox: public Area {
    OuterBox() : w(SdlManager::get()->width), h(SdlManager::get()->height) {}
    virtual float get_x() const { assert(false); return 0; }
    virtual float get_y() const { assert(false); return 0; }
    virtual float get_left() const { return -100000; }
    virtual float get_right() const { return 100000; }
    virtual float get_bottom() const { return 100000; }
    virtual float get_top() const { return -100000; }
    virtual bool  collide_with(const Point* p) const { return p->get_x()<0 or p->get_x()>w or p->get_y()>h; }
protected:
    float w,h;
};

class Zombie: public Area {
public:
    Zombie(const Sprite * guy,float x,float y) {
        sprite = SpriteManager::get()->get_sprite("zombie");
        sprite->x = x;
        sprite->y = y;
        target = guy;
    }
    void update(float dt) {
        float dx = target->x - sprite->x;
        float dy = target->y - sprite->y;
        float norm = sqrt(dx*dx+dy*dy);
        sprite->x += dt*ZOMBIE_SPEED*dx/norm;
        sprite->y += dt*ZOMBIE_SPEED*dy/norm;
        sprite->angle = atan2(dy,dx);
        sprite->draw(dt);
    }
    virtual float get_x() const { return sprite->x; }
    virtual float get_y() const { return sprite->y; }
    virtual float get_left() const { return sprite->x-16; }
    virtual float get_right() const { return sprite->x+16; }
    virtual float get_bottom() const { return sprite->y+16; }
    virtual float get_top() const {return sprite->y-16; }
    virtual bool  collide_with(const Point* p) const { return true; }
    virtual ~Zombie() {
        delete sprite;
    }
    Sprite *sprite;
protected:
    const Sprite *target;
};

//static int aaaa = 0;

class Spawner : public Listener {
public:
    Spawner(): space(CollisionManager::get()->spaces[0]) {
        cross=SpriteManager::get()->get_sprite("cross");
        cross->z=3;
        guy=SpriteManager::get()->get_sprite("guy");
        guy->x=SdlManager::get()->width/2;
        guy->y=SdlManager::get()->height/2;
        bullet=SpriteManager::get()->get_sprite("bullet06");
        reload=0;

        space.second.insert(new OuterBox);
    }
    ~Spawner() {
        unregister_self();
        delete cross;
        delete guy;
        delete bullet;
    }
    typedef std::list<Sprite*> Corpses;
    typedef std::set<Zombie*> Zombies;
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_q:
            CollisionManager::get()->dump();
            break;
        case SDLK_SPACE:
            space.second.insert(new Zombie(guy,50,50));
            break;
        case SDLK_ESCAPE:
            return false; break;
        }
        return true;
    }
    virtual bool mouse_down(int button,float x,float y) {
        if (button == 1) shooting = true;
        return true;
    }
    virtual bool mouse_up(int button,float x,float y) {
        if (button == 1) shooting = false;
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        SdlManager::get()->get_mouse_position(cross->x,cross->y);
        cross->draw(dt);

        float dx=0;
        float dy=0;
        const unsigned char *keys=SdlManager::get()->get_key_state();
        if (keys[SDLK_DOWN]  or keys[SDLK_s]) dy=GUY_SPEED;
        if (keys[SDLK_UP]    or keys[SDLK_w]) dy-=GUY_SPEED;
        if (keys[SDLK_RIGHT] or keys[SDLK_d]) dx=GUY_SPEED;
        if (keys[SDLK_LEFT]  or keys[SDLK_a]) dx=-GUY_SPEED;
        guy->x += dx*dt;
        guy->y += dy*dt;
        if (guy->x < 0) guy->x=0;
        if (guy->x > SdlManager::get()->width) guy->x=SdlManager::get()->width;
        if (guy->y < 0) guy->y=0;
        if (guy->y > SdlManager::get()->height) guy->y=SdlManager::get()->height;
        guy->angle=atan2(cross->y-guy->y,cross->x-guy->x);
        guy->draw(dt);

        if (shooting and reload<=0) {
            space.first.insert(new Bullet(guy));
            //aaaa++;
            reload = BULLET_RELOAD;
        }
        reload -= dt;

        for (CollisionManager::Points::iterator i=space.first.begin(); i!=space.first.end(); i++) {
            Bullet *current = static_cast<Bullet*>(*i);
            current->x += current->vx * dt;
            current->y += current->vy * dt;
        }
        CollisionManager::get()->resolve_collision();

        CollisionManager::Points bullet_garbage;
        Zombies zombie_garbage;
        int k=0;
        for (CollisionManager::Areas::iterator i=space.second.begin(); i!=space.second.end(); i++) {
            //cout<<*i<<": ";
            for (Area::Points::iterator j=(*i)->colliding.begin(); j!=(*i)->colliding.end(); j++) {
                Area::Points::iterator jj=j;
                bullet_garbage.insert(*jj);
                space.first.erase(*jj);
            }

            //cout<<" count="<<aaaa<<endl;
            Zombie *zombie = dynamic_cast<Zombie*>(*i);
            if (zombie) {
                if (zombie->colliding.empty()) zombie->update(dt);
                else {
                    CollisionManager::Areas::iterator ii=i;
                    zombie_garbage.insert(zombie);
                    space.second.erase(*ii);
                }
            }
            k++;
        }

        for (Zombies::iterator i=zombie_garbage.begin(); i!=zombie_garbage.end(); i++) {
            Sprite *corpse = SpriteManager::get()->get_sprite("zombie_dead");
            corpse->x = (*i)->sprite->x;
            corpse->y = (*i)->sprite->y;
            corpse->z = -1;
            corpse->angle = (*i)->sprite->angle;
            corpses.push_back(corpse);
            delete *i;
        }

        for (CollisionManager::Points::iterator i=bullet_garbage.begin(); i!=bullet_garbage.end(); i++) {
            delete *i;
            //aaaa--;
        }

        for (CollisionManager::Points::iterator i=space.first.begin(); i!=space.first.end(); i++) {
            bullet->x = (*i)->get_x();
            bullet->y = (*i)->get_y();
            bullet->draw(dt);
        }

        for(Corpses::iterator i=corpses.begin(); i!=corpses.end(); i++) {
            (*i)->draw(dt);
        }

        return true;
    }
    virtual void unregister_self() {
        //FIXME cleaning....
    }
    Corpses corpses;
    Sprite *cross;
    Sprite *bullet;
    Sprite *guy;
    float reload;
    bool shooting;
    CollisionManager::Space &space;
};

int main() {
    try {
        SdlManager::init();
        SdlManager::get()->set_background_color(0,.8,0);

        SoundManager::init();
        if (not SoundManager::get()->load_directory("data"))
        if (not SoundManager::get()->load_directory("../data"))
        if (not SoundManager::get()->load_directory("../../data")) {
            cerr<<"can't locate sound data..."<<endl;
            return 1;
        }
        SoundManager::get()->dump();

        SpriteManager::init();
        if (not SpriteManager::get()->load_directory("data"))
        if (not SpriteManager::get()->load_directory("../data"))
        if (not SpriteManager::get()->load_directory("../../data")) {
            cerr<<"can't locate sprite data..."<<endl;
            return 1;
        }
        SpriteManager::get()->dump();

        CollisionManager::init(1);

        MessageManager::init();
        SdlManager::get()->register_listener(MessageManager::get());
        MessageManager::get()->set_display(true);
        {
            Fps fps;
            fps.set_display(true);
            Spawner spawner;
            SdlManager::get()->register_listener(&fps);
            SdlManager::get()->register_listener(&spawner);

            MessageManager::get()->add_message("lets get started");
            SdlManager::get()->main_loop();
        }
        SoundManager::free();
        MessageManager::free();
        CollisionManager::free();
        SpriteManager::free();
        SdlManager::free();
        //cout<<"aaaa="<<aaaa<<endl;
    } catch (Except e) {
        e.dump();
        return 1;
    }
}


