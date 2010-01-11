#include "except.h"
#include "utils.h"
#include "message.h"
#include "sound.h"
#include <ode/ode.h>
#include <cassert>
#include <list>
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

struct Wheel {
    Wheel(float x,float y,float drift) : x(x), y(y), brake(0), drift(drift) {
        f[0] = 0;
        f[1] = 0;
        f[2] = 0;
    }
    void add_force(const dBodyID &body) {
        const dReal *vel = dBodyGetLinearVel(body);
        const dReal *avel = dBodyGetAngularVel(body);
        const dReal *quat = dBodyGetQuaternion(body);
        float angle = 2*atan2(quat[3],quat[0]);

        float brelvel[2] = {vel[0]*cos(angle)+vel[1]*sin(angle)-y*avel[2],-vel[0]*sin(angle)+vel[1]*cos(angle)+x*avel[2]};
        float relvel[2]  = {brelvel[0]*cos(brake)+brelvel[1]*sin(brake),-brelvel[0]*sin(brake)+brelvel[1]*cos(brake)};
        printf("%.2f %.2f\n",brelvel[0],brelvel[1]);

        if (relvel[1]>drift) relvel[1] = drift;
        f[0] = accel*cos(brake)+relvel[1]*6000*sin(brake);
        f[1] = accel*sin(brake)-relvel[1]*6000*cos(brake);
    }
    float accel;
    float brake;
    float f[3];
    const float x,y;
    const float drift;
};
    
struct Truck : public Body {
    Truck(dWorldID world, dSpaceID space, float x, float y) : FR(45,75,40), FL(45,-75,40), BR(-20,75,40), BL(-20,-75,40) {
        body = dBodyCreate(world);
        dBodySetPosition(body,x,y,0);

        dMass mass;
        dMassSetBoxTotal(&mass,600,150,54,20);
        dBodySetMass(body,&mass);

        dGeomID geom = dCreateBox(space,150,54,20);
        dGeomSetBody(geom,body);

        sprite = SpriteManager::get()->get_sprite("pickup");
        assert(sprite);
        sprite->x = x;
        sprite->y = y;
        sprite->factorx = .5;
        sprite->factory = .5;

    }
    virtual void update(float dt) {
        cout << "-----------------------------" << endl;
        FR.brake = brake;
        FL.brake = brake;
        const dReal *vel = dBodyGetLinearVel(body);
        if (accel>0) {
            BR.accel = 10*accel;
            BL.accel = 10*accel;
            FR.accel = 0;
            FL.accel = 0;
        } else {
            BR.accel = 0;
            BL.accel = 0;
            FR.accel = 10*accel;
            FL.accel = 10*accel;
        }
        FR.add_force(body);
        FL.add_force(body);
        dBodyAddRelForceAtRelPos(body,FL.f[0],FL.f[1],FL.f[2],FL.x,FL.y,0);
        dBodyAddRelForceAtRelPos(body,FR.f[0],FR.f[1],FR.f[2],FR.x,FR.y,0);
        BR.add_force(body);
        BL.add_force(body);
        dBodyAddRelForceAtRelPos(body,BL.f[0],BL.f[1],BL.f[2],BL.x,BL.y,0);
        dBodyAddRelForceAtRelPos(body,BR.f[0],BR.f[1],BR.f[2],BR.x,BR.y,0);
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

    virtual ~Truck() {
        delete sprite;
    }
    float accel;
    float brake;
protected:
    Sprite *sprite;
    Wheel FL,FR,BL,BR;
};

class MainApp : public Listener {
public:
    MainApp() {
        dInitODE();
        world = dWorldCreate();
        space = dHashSpaceCreate(0);
        contacts = dJointGroupCreate(0);

        //dWorldSetGravity(world,0,300.,0);

        dGeomID top = dCreatePlane(space,0,1,0,0);
        dGeomID bot = dCreatePlane(space,0,-1,0,-SdlManager::get()->height);
        dGeomID lef = dCreatePlane(space,1,0,0,0);
        dGeomID rig = dCreatePlane(space,-1,0,0,-SdlManager::get()->width);
        dGeomSetCategoryBits(top,1);
        dGeomSetCategoryBits(bot,1);
        dGeomSetCollideBits(top,~2);
        dGeomSetCollideBits(bot,~2);
        dGeomSetCategoryBits(lef,2);
        dGeomSetCategoryBits(rig,2);
        dGeomSetCollideBits(lef,~1);
        dGeomSetCollideBits(rig,~1);

        truck = new Truck(world,space,200,200);
        truck->set_angle(M_PI/3);
        bodies.push_back(truck);
    }
    ~MainApp() {
        unregister_self();

        dSpaceDestroy(space);
        dJointGroupDestroy(contacts);
        dWorldDestroy(world);
        dCloseODE();
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
                contacts[k].surface.mu = dInfinity;
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
            truck->accel = 20000;
            break;
        case SDLK_DOWN:
            truck->accel = -20000;
            break;
        case SDLK_LEFT:
            truck->brake = -M_PI/5;
            break;
        case SDLK_RIGHT:
            truck->brake = M_PI/5;
            break;
        case SDLK_f:
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
            truck->accel = 0;
            break;
        case SDLK_LEFT:
        case SDLK_RIGHT:
            truck->brake = 0;
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
        if (dt<=0) return true;

        for (Bodies::iterator i=bodies.begin(); i!=bodies.end(); i++) { (*i)->update(dt); }

        dSpaceCollide(space,static_cast<void*>(this),collide_callback);
        dWorldQuickStep(world,dt);
        dJointGroupEmpty(contacts);

        for (Bodies::iterator i=bodies.begin(); i!=bodies.end(); i++) { (*i)->draw(dt); }
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
};

int main() {
    try {
        srand(time(NULL));

        SdlManager::init();
        SdlManager::get()->set_background_color(0.2,0.4,0.4);

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

