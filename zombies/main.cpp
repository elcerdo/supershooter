#include "except.h"
#include "collision.h"
#include "utils.h"
//#include "png.h"
#include "message.h"
#include "sound.h"
#include <cmath>
#include <list>
#include <cassert>
#include <queue>
#include <SDL/SDL_image.h>
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


#define ELEM_NEVER_WALKED -2
#define ELEM_IN_QUEUE     -1

struct Elem {
    Elem(int i,int j,float d,const SDL_Surface *surf,float *distance) : i(i), j(j), d(d), surf(surf), distance(distance) {
        assert(not (i<0 or i>=surf->w or j<0 or j>=surf->h));
        assert(distance[j*surf->w+i] == ELEM_NEVER_WALKED);
        distance[j*surf->w+i] = ELEM_IN_QUEUE;
    }
    ~Elem() {
        //printf("----- %d %d %f\n",this->i,this->j,this->d);
        distance[j*surf->w+i] = d;
    }
    Elem *get_neighbor(int di,int dj) const {
        if (i+di<0 or i+di>=surf->w or j+dj<0 or j+dj>=surf->h) return NULL; //invalid coords
        if (distance[(j+dj)*surf->w+(i+di)] != ELEM_NEVER_WALKED) return NULL; //already in queue or updated
        if (static_cast<const uint8_t*>(surf->pixels)[4*((j+dj)*surf->w+(i+di))+3] != 0) return NULL; //inside walls

        return new Elem(i+di,j+dj,d+sqrt(di*di+dj*dj),surf,distance);
    }
    int i,j;
    float d;
    const SDL_Surface *surf;
    float *distance;
};

struct ElemCompare {
    bool operator()(const Elem* a,const Elem *b) { return a->d > b->d; }
};

typedef std::priority_queue<Elem*,std::vector<Elem*>,ElemCompare> ElemQueue;

struct Buildings: public Area {
    Buildings() : w(SdlManager::get()->width), h(SdlManager::get()->height) {
        map_ground = SpriteManager::get()->get_sprite("map_ground");
        map_ground->factorx=w/map_ground->w;
        map_ground->factory=h/map_ground->h;
        map_ground->x=SdlManager::get()->width/2;
        map_ground->y=SdlManager::get()->height/2;
        map_ground->z=-1;
        surf_wall=IMG_Load("../data/map_wall.png");
        surf_bullet=IMG_Load("../data/map_bullet.png");
        assert(surf_wall);
        assert(surf_bullet);
        distance = new float[surf_wall->w*surf_wall->h];
    }
    virtual ~Buildings() {
        delete map_ground;
        delete [] distance;
        SDL_FreeSurface(surf_bullet);
        SDL_FreeSurface(surf_wall);
    }

    void update_distance(float x,float y) {
        for (int k=0; k<surf_wall->w*surf_wall->h; k++) { distance[k] = ELEM_NEVER_WALKED; }

        //int k=0;
        ElemQueue queue;
        queue.push(new Elem(x*surf_wall->w/w,y*surf_wall->h/h,0,surf_wall,distance));
        while (not queue.empty()) {
            Elem *neighbor = NULL;
            Elem *current = queue.top();
            queue.pop();
            neighbor = current->get_neighbor(-1,0);  if (neighbor) queue.push(neighbor);
            neighbor = current->get_neighbor(1,0);   if (neighbor) queue.push(neighbor);
            neighbor = current->get_neighbor(0,-1);  if (neighbor) queue.push(neighbor);
            neighbor = current->get_neighbor(0,1);   if (neighbor) queue.push(neighbor);
            neighbor = current->get_neighbor(-1,1);  if (neighbor) queue.push(neighbor);
            neighbor = current->get_neighbor(-1,-1); if (neighbor) queue.push(neighbor);
            neighbor = current->get_neighbor(1,1);   if (neighbor) queue.push(neighbor);
            neighbor = current->get_neighbor(1,-1);  if (neighbor) queue.push(neighbor);
            delete current;
            //char filename[256];
            //if (k%50 == 0) {
            //    sprintf(filename,"distance%03d.png",k/50);
            //    write_frame_float_scale(filename,distance,surf_wall->h,surf_wall->w,128,0);
            //}
            //k++;
        }

    }

    void get_gradient(float x,float y,float &dx,float &dy) const {
        assert(not is_inside(x,y,surf_wall));
        int i=x*surf_wall->w/w;
        int j=y*surf_wall->h/h;

        int kdx=0;
        int kdy=0;
        dx=0;
        dy=0;
        if (i>0 and distance[j*surf_wall->w+i-1] != ELEM_NEVER_WALKED) { kdx++; dx-=distance[j*surf_wall->w+i]-distance[j*surf_wall->w+i-1]; }
        if (i<w-1 and distance[j*surf_wall->w+i+1] != ELEM_NEVER_WALKED) { kdx++; dx-=distance[j*surf_wall->w+i+1]-distance[j*surf_wall->w+i]; }
        if (j>0 and distance[(j-1)*surf_wall->w+i] != ELEM_NEVER_WALKED) { kdy++; dy-=distance[j*surf_wall->w+i]-distance[(j-1)*surf_wall->w+i]; }
        if (j<h-1 and distance[(j+1)*surf_wall->w+i] != ELEM_NEVER_WALKED) { kdy++; dy-=distance[(j+1)*surf_wall->w+i]-distance[j*surf_wall->w+i]; }
        assert(kdx !=0);
        assert(kdy !=0);
        dx /= kdx;
        dy /= kdy;
    }

    void draw(float dt) {
        map_ground->draw(dt);
    }
    bool avoid_walls(float &x,float &y,float &dx,float &dy) const {
        assert(not is_inside(x,y,surf_wall));
        bool xx = not is_inside(x+dx,y,surf_wall);
        bool yy = not is_inside(x,y+dy,surf_wall);
        bool xy = not is_inside(x+dx,y+dy,surf_wall);
        if (xy) { x += dx; y += dy; }
        else if (xx) { x += dx; dy = 0; }
        else if (yy) { y += dy; dx = 0; }

        if (x < 0) { x=0; dx=0; }
        if (x >= w) { x=w-1; dx=0; }
        if (y < 0) { y=0; dy=0; }
        if (y >= h) { y=h-1; dy=0; }

        //bool luxy = not is_inside(x+dx-size,y+dy-size);
        //bool lbxy = not is_inside(x+dx-size,y+dy+size);
        //bool ruxy = not is_inside(x+dx+size,y+dy-size);
        //bool rbxy = not is_inside(x+dx+size,y+dy+size);
        //bool luxx = not is_inside(x+dx-size,y-size);
        //bool lbxx = not is_inside(x+dx-size,y+size);
        //bool ruxx = not is_inside(x+dx+size,y-size);
        //bool rbxx = not is_inside(x+dx+size,y+size);
        //bool luyy = not is_inside(x-size,y+dy-size);
        //bool lbyy = not is_inside(x-size,y+dy+size);
        //bool ruyy = not is_inside(x+size,y+dy-size);
        //bool rbyy = not is_inside(x+size,y+dy+size);
        //if (((dx<0 and luxy and lbxy) or (dx>0 and ruxy and rbxy)) and ((dy<0 and luxy and ruxy) or (dy>0 and lbxy and rbxy))) { y += dy; x += dx; }
        //else if ((dx<0 and luxx and lbxx) or (dx>0 and ruxx and rbxx)) { x += dx; }
        //else if ((dy<0 and luyy and lbyy) or (dy>0 and ruyy and rbyy)) { y += dy; }
    }
    virtual float get_x() const { assert(false); return 0; }
    virtual float get_y() const { assert(false); return 0; }
    virtual float get_left() const { return 0; }
    virtual float get_right() const { return w; }
    virtual float get_bottom() const { return h; }
    virtual float get_top() const { return 0; }
    virtual bool  collide_with(const Point* p) const {
        assert(dynamic_cast<const Bullet*>(p));
        return is_inside(p->get_x(),p->get_y(),surf_bullet);
    }
protected:
    bool is_inside(float x,float y,const SDL_Surface *surf) const {
        const uint8_t *pixels = static_cast<const uint8_t*>(surf->pixels);
        int i=x*surf->w/w;
        int j=y*surf->h/h;
        const uint8_t *pixel=&pixels[4*(j*surf->w+i)];
        return pixel[3]!=0;
    }
    Sprite *map_upper;
    Sprite *map_ground;
    SDL_Surface *surf_bullet;
    SDL_Surface *surf_wall;
    float *distance;
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
    void update(float dt,const Buildings *buildings) {
        float dx;
        float dy;
        buildings->get_gradient(sprite->x,sprite->y,dx,dy);
        float norm;
        norm = sqrt(dx*dx+dy*dy);
        if (norm != 0) {
            dx *= dt*ZOMBIE_SPEED/norm;
            dy *= dt*ZOMBIE_SPEED/norm;
            buildings->avoid_walls(sprite->x,sprite->y,dx,dy);
        }
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
        buildings = new Buildings;
        space.second.insert(buildings);

    }
    ~Spawner() {
        unregister_self();
        for(Corpses::iterator i=corpses.begin(); i!=corpses.end(); i++) delete *i;
        delete cross;
        delete guy;
        delete bullet;
        delete buildings;
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
        buildings->draw(dt);

        float dx=0;
        float dy=0;
        const unsigned char *keys=SdlManager::get()->get_key_state();
        if (keys[SDLK_DOWN]  or keys[SDLK_s]) dy=GUY_SPEED;
        if (keys[SDLK_UP]    or keys[SDLK_w]) dy-=GUY_SPEED;
        if (keys[SDLK_RIGHT] or keys[SDLK_d]) dx=GUY_SPEED;
        if (keys[SDLK_LEFT]  or keys[SDLK_a]) dx=-GUY_SPEED;
        dx *= dt;
        dy *= dt;
        buildings->avoid_walls(guy->x,guy->y,dx,dy);
        guy->angle=atan2(cross->y-guy->y,cross->x-guy->x);
        guy->draw(dt);
        buildings->update_distance(guy->x,guy->y);

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
                if (zombie->colliding.empty()) zombie->update(dt,buildings);
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
    Sprite *map;
    Buildings *buildings;
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


