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

struct Crate {
    Crate(dWorldID world, dSpaceID space, float x, float y) {
        body = dBodyCreate(world);
        dBodySetPosition(body,x,y,0);
        dBodySetAngularVel(body,0,0,.5);

        dMass mass;
        dMassSetBoxTotal(&mass,1,32,32,20);
        dBodySetMass(body,&mass);

        geom = dCreateBox(space,32,32,20);
        dGeomSetBody(geom,body);

        sprite = SpriteManager::get()->get_sprite("crate");
        assert(sprite);
        sprite->x = x;
        sprite->y = y;
    }
    void draw(float dt) {
        const dReal *pos = dBodyGetPosition(body);
        //printf("posi: %.2f %.2f %.2f\n",pos[0],pos[1],pos[2]);
        dBodySetPosition(body,pos[0],pos[1],0);
        sprite->x = pos[0];
        sprite->y = pos[1];

        const dReal *avel = dBodyGetAngularVel(body);
        //printf("avel: %.2f %.2f %.2f\n",avel[0],avel[1],avel[2]);
        dBodySetAngularVel(body,0,0,avel[2]);

        const dReal *quat = dBodyGetQuaternion(body);
        float norm = sqrt(quat[0]*quat[0]+quat[3]*quat[3]);
        dReal nquat[4] = {quat[0]/norm,0,0,quat[3]/norm};
        dBodySetQuaternion(body,nquat);

        float angle = 2*atan2(quat[3],quat[0]);
        sprite->angle = angle;
        sprite->draw(dt);
    }
    ~Crate() {
        delete sprite;
    }
protected:
    dBodyID body;
    dGeomID geom;
    Sprite *sprite;
};



class MainApp : public Listener {
public:
    MainApp() {
        dInitODE();
        world = dWorldCreate();
        space = dHashSpaceCreate(0);
        contacts = dJointGroupCreate(0);

        dWorldSetGravity(world,0,300.,0);

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
    }
    ~MainApp() {
        unregister_self();

        dSpaceDestroy(space);
        dJointGroupDestroy(contacts);
        dWorldDestroy(world);
        dCloseODE();
    }
    Crate *add_crate(float x, float y) {
        crates.push_back(new Crate(world,space,x,y));
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
    virtual bool mouse_down(int button,float x,float y) {
        add_crate(x,y);
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        if (dt>0) {
            dSpaceCollide(space,static_cast<void*>(this),collide_callback);
            dWorldQuickStep(world,dt);
            dJointGroupEmpty(contacts);
        }

        for (Crates::iterator i=crates.begin(); i!=crates.end(); i++) { (*i)->draw(dt); }
        return true;
    }
    virtual void unregister_self() {
        while (not crates.empty()) {
            delete crates.back();
            crates.pop_back();
        }
    }

    dWorldID world;
    dSpaceID space;
    dJointGroupID contacts;
    typedef std::list<Crate*> Crates;
    Crates crates;
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
            mainapp.add_crate(100,30);

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

