#include "except.h"
#include "utils.h"
#include "message.h"
#include "sound.h"
#include <ode/ode.h>
#include <cassert>
#include <list>
#include <sstream>
using std::cout;
using std::cerr;
using std::endl;

struct Body {
    Body() {}
    virtual void update(float dt) =0;
    virtual void draw(float dt) = 0;
    virtual ~Body() {
        dBodyDestroy(body);
    }
protected:
    void restrict_2d() {
        //restrict to 2D
        const dReal *pos = dBodyGetPosition(body);
        //printf("posi: %.2f %.2f %.2f\n",pos[0],pos[1],pos[2]);
        dBodySetPosition(body,pos[0],pos[1],0);


        const dReal *avel = dBodyGetAngularVel(body);
        //printf("avel: %.2f %.2f %.2f\n",avel[0],avel[1],avel[2]);
        dBodySetAngularVel(body,0,0,avel[2]);

        const dReal *quat = dBodyGetQuaternion(body);
        float norm = sqrt(quat[0]*quat[0]+quat[3]*quat[3]);
        dReal nquat[4] = {quat[0]/norm,0,0,quat[3]/norm};
        dBodySetQuaternion(body,nquat);
    }
    dBodyID body;
};

struct Crate : public Body {
    Crate(dWorldID world, dSpaceID space, float x, float y) {
        body = dBodyCreate(world);
        dBodySetPosition(body,x,y,0);
        dBodySetAngularVel(body,0,0,.5);

        dMass mass;
        dMassSetBoxTotal(&mass,75,32,32,20);
        dBodySetMass(body,&mass);

        dGeomID geom = dCreateBox(space,32,32,20);
        dGeomSetBody(geom,body);

        sprite = SpriteManager::get()->get_sprite("crate");
        assert(sprite);
        sprite->x = x;
        sprite->y = y;

        dBodySetAngularDamping(body,.1);
        dBodySetLinearDamping(body,.02);
    }
    virtual void update(float dt) {}
    virtual void draw(float dt) {
        restrict_2d();

        const dReal *pos = dBodyGetPosition(body);
        sprite->x = pos[0];
        sprite->y = pos[1];

        const dReal *quat = dBodyGetQuaternion(body);
        float angle = 2*atan2(quat[3],quat[0]);
        sprite->angle = angle;
        sprite->draw(dt);
    }
    virtual ~Crate() {
        delete sprite;
    }
protected:
    Sprite *sprite;
};

static const float taudirection = .2;
static const float drealdirectionmax = 4e-3;

struct Wheel {
    Wheel(float x,float y,Sprite *parent,float drift=50,float brake=10e4,float forward=2e5,float reverse=-10e4) : x(x), y(y), brake(brake), drift(drift), state(FREE), forward(forward), reverse(reverse), direction(0), realdirection(0) {
        sprite = dynamic_cast<StateSprite*>(parent->create_child("onoff"));
        sprite->x = x;
        sprite->y = y;
        sprite->factorx = .5;
        sprite->factory = .5;
        sprite->z = 1;
        arrow = parent->create_child("arrow");
        arrow->x = x;
        arrow->y = y;
        arrow->z = 1.1;
        f[0] = 0;
        f[1] = 0;
    }
    void add_force(const dBodyID &body,float dt) {
        float drealdirection = dt/taudirection * (realdirection-direction);
        if (drealdirection > drealdirectionmax)  drealdirection = drealdirectionmax;
        if (drealdirection < -drealdirectionmax) drealdirection = -drealdirectionmax;
        realdirection -= drealdirection;

        const dReal *vel = dBodyGetLinearVel(body);
        const dReal *avel = dBodyGetAngularVel(body);
        const dReal *quat = dBodyGetQuaternion(body);
        float angle = 2*atan2(quat[3],quat[0]);

        float brelvel[2] = {vel[0]*cos(angle)+vel[1]*sin(angle)-y*avel[2],-vel[0]*sin(angle)+vel[1]*cos(angle)+x*avel[2]};
        float relvel[2]  = {brelvel[0]*cos(realdirection)+brelvel[1]*sin(realdirection),-brelvel[0]*sin(realdirection)+brelvel[1]*cos(realdirection)};
        //printf("%.2f %.2f\n",brelvel[0],brelvel[1]);

        float force;
        switch (state) {
        case FREE:
            force = 0;
            break;
        case BRAKE:
            if (relvel[0]>0) force = -brake;
            else force = brake;
            break;
        case FORWARD:
            force = forward;
            break;
        case REVERSE:
            force = reverse;
            break;
        }
        force -= relvel[0]*40;

        sprite->state = 0;
        arrow->angle = realdirection;
        if (relvel[1]>drift)  { relvel[1] = drift;  sprite->state = 1; }
        if (relvel[1]<-drift) { relvel[1] = -drift; sprite->state = 1; }
        f[0] = force*cos(realdirection)+relvel[1]*6e3*sin(realdirection);
        f[1] = force*sin(realdirection)-relvel[1]*6e3*cos(realdirection);
    }
    enum State {BRAKE,FREE,FORWARD,REVERSE};
    State state;
    float direction;
    const float x,y;
    float f[2];
protected:
    float realdirection;
    const float reverse;
    const float forward;
    const float drift;
    const float brake;
    StateSprite *sprite;
    Sprite *arrow;
};
    
struct Truck : public Body {
    Truck(dWorldID world, dSpaceID space, float x, float y) : state(Wheel::FREE), direction(0) {
        sprite = SpriteManager::get()->get_sprite("pickup");
        assert(sprite);
        FR = new Wheel(50,22,sprite);
        FL = new Wheel(50,-22,sprite);
        BR = new Wheel(-35,22,sprite);
        BL = new Wheel(-35,-22,sprite);
        
        body = dBodyCreate(world);
        dBodySetPosition(body,x,y,0);

        dMass mass;
        dMassSetBoxTotal(&mass,600,150,54,20);
        dBodySetMass(body,&mass);

        dGeomID geom = dCreateBox(space,150,54,20);
        dGeomSetBody(geom,body);

        sprite->x = x;
        sprite->y = y;

        relvel[0] = 0;
        relvel[1] = 0;
    }
    virtual void update(float dt) {
        FR->direction = atan(85.*tan(direction)/(85.-22.*tan(direction)));
        FL->direction = atan(85.*tan(direction)/(85.+22.*tan(direction)));
        switch (state) {
        case Wheel::FREE:
            BR->state = Wheel::FREE;
            BL->state = Wheel::FREE;
            FR->state = Wheel::FREE;
            FL->state = Wheel::FREE;
            break;
        case Wheel::BRAKE:
            BR->state = Wheel::BRAKE;
            BL->state = Wheel::BRAKE;
            FR->state = Wheel::BRAKE;
            FL->state = Wheel::BRAKE;
            break;
        case Wheel::FORWARD:
            BR->state = Wheel::FREE;
            BL->state = Wheel::FREE;
            FR->state = Wheel::FORWARD;
            FL->state = Wheel::FORWARD;
            break;
        case Wheel::REVERSE:
            BR->state = Wheel::FREE;
            BL->state = Wheel::FREE;
            FR->state = Wheel::REVERSE;
            FL->state = Wheel::REVERSE;
            break;
        }
        //printf("%d %d %d %d\n",FR->state,FL->state,BR->state,BL->state);

        FR->add_force(body,dt);
        FL->add_force(body,dt);
        BR->add_force(body,dt);
        BL->add_force(body,dt);
        dBodyAddRelForceAtRelPos(body,FL->f[0],FL->f[1],0,FL->x,FL->y,0);
        dBodyAddRelForceAtRelPos(body,FR->f[0],FR->f[1],0,FR->x,FR->y,0);
        dBodyAddRelForceAtRelPos(body,BL->f[0],BL->f[1],0,BL->x,BL->y,0);
        dBodyAddRelForceAtRelPos(body,BR->f[0],BR->f[1],0,BR->x,BR->y,0);

        const dReal *vel = dBodyGetLinearVel(body);
        const dReal *quat = dBodyGetQuaternion(body);
        float angle = 2*atan2(quat[3],quat[0]);
        relvel[0] =  vel[0]*cos(angle)+vel[1]*sin(angle);
        relvel[1] = -vel[0]*sin(angle)+vel[1]*cos(angle);
        //printf("%.2f %.2f\n",relvel[0],relvel[1]);
    }
    virtual void draw(float dt) {
        restrict_2d();

        const dReal *pos = dBodyGetPosition(body);
        sprite->x = pos[0];
        sprite->y = pos[1];

        const dReal *quat = dBodyGetQuaternion(body);
        float angle = 2*atan2(quat[3],quat[0]);
        sprite->angle = angle;
        sprite->draw(dt);
    }
    void set_angle(float angle) {
        dReal quat[4] = {cos(angle/2),0,0,sin(angle/2)};
        dBodySetQuaternion(body,quat);
    }
    float get_x() const { return sprite->x; }
    float get_y() const { return sprite->y; }

    virtual ~Truck() {
        delete sprite;
        delete FL,FR,BL,BR;
    }
    Wheel::State state;
    float direction;
    float relvel[2];
protected:
    Wheel *FL,*FR,*BL,*BR;
    Sprite *sprite;
};

struct Roads {
    Roads() {
        float accumy = 0;
        for (int k=0; k<NROADS; k++) {
            Sprite *leftroad = SpriteManager::get()->get_sprite("road");
            leftroad->z = -.1;
            leftroad->cy = -leftroad->h/2.;
            leftroad->x = LEFTX;
            leftroad->y = accumy;

            std::stringstream ss;
            ss << "section " << k;
            Text *leftlabel = SpriteManager::get()->get_text(ss.str(),"font03",Text::LEFT);
            leftlabel->cy = -leftlabel->h/2.;
            leftlabel->x = -leftroad->w/2.;
            leftroad->z = -.05;
            leftroad->register_child(leftlabel);

            sections.push_back(std::make_pair(leftroad,leftlabel));
            accumy += leftroad->h;
        }
    }
    ~Roads() {
        while (not sections.empty()) {
            delete sections.back().first;
            sections.pop_back();
        }
    }
    void update(float dt,float cy) {
        int csection = ceil(cy/sections.back().first->h);
        int k=4;
        while (k--) {
            int msection = get_middle_section();
            //printf("%d %d\n",csection,msection);

            if (msection < csection) {
                Section section = sections.front();
                sections.pop_front();
                section.first->y += NROADS*section.first->h;
                sections.push_back(section);
            }
            if (msection > csection) {
                Section section = sections.back();
                sections.pop_back();
                section.first->y -= NROADS*section.first->h;
                sections.push_front(section);
            }
            if (msection == csection) break;
        }
        for (Sections::const_iterator i=sections.begin(); i!=sections.end(); i++) { i->first->draw(dt); }
    }

    static const int NROADS = 5;
    static const float LEFTX = 300;
protected:
    int get_middle_section() const {
        Sections::const_iterator i=sections.begin();
        for (int k=0; k<NROADS/2.; k++) { i++; }
        return i->first->y/i->first->h - 1;
    }
    typedef std::pair<Sprite*,Text*> Section;
    typedef std::list<Section> Sections;
    Sections sections;
};

class MainApp : public Listener {
public:
    MainApp() {
        dInitODE();
        world = dWorldCreate();
        space = dHashSpaceCreate(0);
        contacts = dJointGroupCreate(0);

        //dWorldSetGravity(world,0,300.,0);

        //dGeomID top = dCreatePlane(space,0,1,0,0);
        //dGeomID bot = dCreatePlane(space,0,-1,0,-SdlManager::get()->height);
        dGeomID lef = dCreatePlane(space,1,0,0,0);
        dGeomID rig = dCreatePlane(space,-1,0,0,-SdlManager::get()->width);
        //dGeomSetCategoryBits(top,1);
        //dGeomSetCategoryBits(bot,1);
        //dGeomSetCollideBits(top,~2);
        //dGeomSetCollideBits(bot,~2);
        dGeomSetCategoryBits(lef,2);
        dGeomSetCategoryBits(rig,2);
        dGeomSetCollideBits(lef,~1);
        dGeomSetCollideBits(rig,~1);

        truck = new Truck(world,space,200,200);
        truck->set_angle(M_PI/3);
        bodies.push_back(truck);

        cursor = SpriteManager::get()->get_sprite("cursor");
        cursor->cx = cursor->w/2.;
        cursor->cy = cursor->h/2.;
        cursor->z  = 4;
    }
    ~MainApp() {
        unregister_self();

        dSpaceDestroy(space);
        dJointGroupDestroy(contacts);
        dWorldDestroy(world);
        dCloseODE();

        delete cursor;
    }
    Body *add_crate(float x, float y) {
        bodies.push_back(new Crate(world,space,x,y));
    }
protected:
    static void collide_callback(void *data, dGeomID a, dGeomID b) {
        //if (dGeomGetClass(a)==dPlaneClass and dGeomGetClass(b)==dPlaneClass) return; //plane plane collision
        MainApp *app = static_cast<MainApp*>(data);

        const int MAXCONTACT = 5;
        dBodyID ab = dGeomGetBody(a);
        dBodyID bb = dGeomGetBody(b);
        dContact contacts[MAXCONTACT];
        if (not ab or not bb) {
            for (int k=0; k<MAXCONTACT; k++) {
                contacts[k].surface.mode = dContactBounce;
                contacts[k].surface.mu = 24;
                contacts[k].surface.bounce = .1;
            }
        } else {
            for (int k=0; k<MAXCONTACT; k++) {
                contacts[k].surface.mode = dContactBounce;
                contacts[k].surface.mu = 2;
                contacts[k].surface.bounce = .8;
            }
        }

        int numc = dCollide(a,b,MAXCONTACT,&contacts[0].geom,sizeof(dContact));
        for (int k=0; k<numc; k++) {
            dJointID contact = dJointCreateContact(app->world,app->contacts,&contacts[k]);
            dJointAttach(contact,ab,bb);
        }

        //printf("collision %d %d %d %d %d\n",dGeomGetClass(a),dGeomGetClass(b),numc,ab,bb);
    }
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_UP:
            truck->state = Wheel::FORWARD;
            break;
        case SDLK_DOWN:
            if (abs(truck->relvel[0])<100) truck->state = Wheel::REVERSE;
            else truck->state = Wheel::BRAKE;
            break;
        case SDLK_LEFT:
            truck->direction = -M_PI/5;
            break;
        case SDLK_RIGHT:
            truck->direction = M_PI/5;
            break;
        case SDLK_d:
            SdlManager::get()->toggle_fullscreen();
            break;
        case SDLK_ESCAPE:
            return false;
            break;
        default:
            break;
        }
        return true;
    }
    virtual bool key_up(SDLKey key) {
        switch (key) {
        case SDLK_UP:
        case SDLK_DOWN:
            truck->state = Wheel::FREE;
            break;
        case SDLK_LEFT:
        case SDLK_RIGHT:
            truck->direction = 0;
            break;
        default:
            break;
        }
        return true;
    }
    virtual bool mouse_down(int button,float x,float y) {
        add_crate(x,y);
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        SdlManager::get()->get_overlay_mouse_position(cursor->x,cursor->y);
        cursor->draw_overlay(dt);

        if (dt<=0) return true;

        SdlManager::get()->set_screen_center(SdlManager::get()->width/2.,truck->get_y());
        for (Bodies::iterator i=bodies.begin(); i!=bodies.end(); i++) { (*i)->draw(dt); }
        roads.update(dt,truck->get_y());

        for (Bodies::iterator i=bodies.begin(); i!=bodies.end(); i++) { (*i)->update(dt); }

        dSpaceCollide(space,static_cast<void*>(this),collide_callback);
        dWorldQuickStep(world,dt);
        dJointGroupEmpty(contacts);

        return true;
    }
    virtual void unregister_self() {
        while (not bodies.empty()) {
            delete bodies.back();
            bodies.pop_back();
        }
    }

    dWorldID world;
    dSpaceID space;
    dJointGroupID contacts;
    typedef std::list<Body*> Bodies;
    Bodies bodies;
    Truck *truck;
    Roads roads;
    Sprite *cursor;
};

int main() {
    try {
        srand(time(NULL));

        SdlManager::init();
        SdlManager::get()->set_background_color(0.2,0.4,0.2);

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

        MessageManager::init();
        SdlManager::get()->register_listener(MessageManager::get());
        MessageManager::get()->set_display(true);
        {
            Fps fps;
            fps.set_display(true);
            MainApp mainapp;

            SdlManager::get()->register_listener(&fps);
            SdlManager::get()->register_listener(&mainapp);

            SdlManager::get()->main_loop();
        }
        SoundManager::free();
        MessageManager::free();
        SpriteManager::free();
        SdlManager::free();
    } catch (Except e) {
        e.dump();
        return 1;
    }
}

