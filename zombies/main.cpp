#include "except.h"
#include "collision.h"
#include "utils.h"
#include "message.h"
#include "sound.h"
#include <cmath>
#include <cassert>
using std::cerr;
using std::cout;
using std::endl;

#define GUY_SPEED 300.
#define BULLET_SPEED 800.
#define BULLET_RELOAD .1

struct Bullet: public Point {
    Bullet(const Sprite *guy) : x(guy->x), y(guy->y), vx(BULLET_SPEED*cos(guy->angle)), vy(BULLET_SPEED*sin(guy->angle)) {}
    virtual float get_x() const { return x; }
    virtual float get_y() const { return y; }
    float x,y;
    float vx,vy;
};

struct OuterBox: public Area {
    OuterBox() : w(SdlManager::get()->width), h(SdlManager::get()->height) {}
    virtual float get_x() const { assert(false); return 0; }
    virtual float get_y() const { assert(false); return 0; }
    virtual float get_left() const { return -300; }
    virtual float get_right() const { return w+300; }
    virtual float get_bottom() const {return h+300; }
    virtual float get_top() const {return -300; }
    virtual bool  collide_with(const Point* p) const { return p->get_x()<0 or p->get_x()>w or p->get_y()>h; }
protected:
    float w,h;
};


static int aaaa = 0;

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
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_m:
            SoundManager::get()->toogle_musics();
            break;
        case SDLK_s:
            SoundManager::get()->toogle_sfxs();
            break;
        case SDLK_SPACE:
            CollisionManager::get()->dump();
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
        if (keys[SDLK_DOWN]) dy=GUY_SPEED;
        if (keys[SDLK_UP]) dy-=GUY_SPEED;
        if (keys[SDLK_RIGHT]) dx=GUY_SPEED;
        if (keys[SDLK_LEFT]) dx=-GUY_SPEED;
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
            aaaa++;
            reload = BULLET_RELOAD;
        }
        reload -= dt;

        for (CollisionManager::Points::iterator i=space.first.begin(); i!=space.first.end(); i++) {
            Bullet *current = static_cast<Bullet*>(*i);
            current->x += current->vx * dt;
            current->y += current->vy * dt;
        }
        CollisionManager::get()->resolve_collision();

        for (CollisionManager::Areas::iterator i=space.second.begin(); i!=space.second.end(); i++) {
            if ((*i)->colliding.empty()) continue;

            //cout<<*i<<": ";
            for (Area::Points::iterator j=(*i)->colliding.begin(); j!=(*i)->colliding.end(); j++) {
                aaaa --;
                //cout<<*j<<" ";
                space.first.erase(*j);
                delete *j;
            }
            //cout<<" count="<<aaaa<<endl;
        }

        for (CollisionManager::Points::iterator i=space.first.begin(); i!=space.first.end(); i++) {
            bullet->x = (*i)->get_x();
            bullet->y = (*i)->get_y();
            bullet->draw(dt);
        }

        return true;
    }
    virtual void unregister_self() {
    }
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
        cout<<"aaaa="<<aaaa<<endl;
    } catch (Except e) {
        e.dump();
        return 1;
    }
}


