#include "shoot.h"
#include "except.h"
#include "utils.h"
#include "message.h"
#include <pwd.h>
#include <cmath>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
using std::cout;
using std::endl;

class BigShip : public Ship, public Listener {
public:
    BigShip() : Ship(100,10000), shooting(false), reload(0), reload2(0) {
        body=SpriteManager::get()->get_sprite("bigship01");
        Sprite *lburst=body->create_child("burst00");
        dynamic_cast<AnimatedSprite*>(lburst)->speed=24;
        lburst->z+=.1;
        lburst->cx=-40;
        lburst->cy=-10;
        Sprite *rburst=body->create_child("burst00");
        dynamic_cast<AnimatedSprite*>(rburst)->speed=24;
        rburst->z+=.1;
        rburst->cx=-40;
        rburst->cy=9;
        body->angle=-M_PI/2.;
    }

    virtual bool move(float dt) {

        //body->x+=dt*speed*cos(angle);
        //body->y+=dt*speed*sin(angle);

        if (reload>0) reload-=dt;
        if (reload2>0) reload2-=dt;

        if (shooting and reload2<=0) {
            reload2+=0.005;
            BulletManager::get()->shoot_from_sprite(body,0,2000,0,"bullet05",7)->sprite;
        }
        if (shooting and reload<=0) {
            reload+=0.1;
            ShipManager::get()->score+=7;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(body,M_PI/180.*15.,600,0,"bullet00",5)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(body,M_PI/180.*25.,600,0,"bullet00",5)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(body,M_PI/180.*35.,600,0,"bullet00",5)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(body,-M_PI/180.*15.,600,0,"bullet00",5)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(body,-M_PI/180.*25.,600,0,"bullet00",5)->sprite)->speed=20.;
            dynamic_cast<AnimatedSprite*>(BulletManager::get()->shoot_from_sprite(body,-M_PI/180.*35.,600,0,"bullet00",5)->sprite)->speed=20.;
        }

        return true;
    }


protected:
    //virtual bool key_down(SDLKey key) {
    //    switch (key) {
    //    case SDLK_SPACE:
    //        shooting=not shooting; break;
    //    }
    //    return true;
    //}

    virtual bool mouse_down(int button, float x,float y) {
        if (button==1) shooting=true;
        return true;
    }
    virtual bool mouse_up(int button,float x,float y) {
        if (button==1) shooting=false;
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        SdlManager::get()->get_mouse_position(body->x,body->y);
        move(dt);
        draw(dt);

        if (health<0) SdlManager::get()->unregister_listener(this);

        return true;
    }

    virtual void register_self() {
        CollisionManager::get()->spaces[1].second.insert(this);
    }

    virtual void unregister_self() {
        CollisionManager::get()->spaces[1].second.erase(this);
    }

    bool shooting;
    float reload;
    float reload2;
};
    

class MainMenu : public Listener {
public:
    MainMenu() : state(IN_MENU), ship(NULL), title_start(NULL), title_name(NULL), wavename("ohyeah"), final_score(NULL), final_text(NULL), final_score_text(NULL), current_hiscore_text(NULL) {
        flogo=SpriteManager::get()->get_sprite("fronttitle");
        blogo=SpriteManager::get()->get_sprite("backtitle");
        flogo->x=SdlManager::get()->width*.5;
        flogo->y=100;
        flogo->z=9;
        blogo->x=SdlManager::get()->width*.5;
        blogo->y=100;
        blogo->z=8.5;

        ship_health=SpriteManager::get()->get_text("life","font00",Text::LEFT);
        ship_health->x=16;
        ship_health->y=SdlManager::get()->height-48;
        ship_score=SpriteManager::get()->get_text("score","font00",Text::LEFT);
        ship_score->x=16;
        ship_score->y=SdlManager::get()->height-16;

        for (size_t k=0; k<255; k++) stars.insert(new Star(true));

        main_menu=new Menu(flogo->y+200,50);
        main_menu->append_item("start");
        main_menu->append_item("fullscreen");
        main_menu->append_item("quit");

        cursor=SpriteManager::get()->get_sprite("cursor");
        cursor->cx=cursor->w/2.;
        cursor->cy=cursor->h/2.;
        cursor->z=8;
    }
    ~MainMenu() {
        delete cursor;
        delete main_menu;

        if (ship) delete ship;
        if (title_name) delete title_name;
        if (title_start) delete title_start;
        if (final_text) delete final_text;
        if (final_score) delete final_score;
        if (final_score_text) delete final_score_text;

        for (Stars::const_iterator i=stars.begin(); i!=stars.end(); i++) delete *i;
        while (not hiscore_texts.empty()) { delete hiscore_texts.front(); hiscore_texts.pop_front(); }

        delete ship_health;
        delete ship_score;
        delete flogo;
        delete blogo;
    }
protected:
    virtual bool mouse_down(int button, float x,float y) {
        if (button!=1) return true;
        main_menu->update(x,y);
        if (state==IN_MENU and main_menu->is_selected("quit")) { return false;
        } else if (state==IN_MENU and main_menu->is_selected("fullscreen")) { SdlManager::get()->toogle_fullscreen();
        } else if (state==IN_MENU and main_menu->is_selected("start")) {
            state=GAME_START;
            ShipManager::get()->flush_ships();
            BulletManager::get()->flush_bullets();
            ship=new BigShip;
            SdlManager::get()->register_listener(ship);
            ShipManager::get()->score=0;
            title_start=new Drifting("GAME START","font01",2.5,SdlManager::get()->height/3.,true);
            title_name=new Drifting(wavename,"font00",1.,title_start->text->y+50,false);
        } else if (state==GAME_OVER) game_over_to_menu();
        return true;
    }
    virtual bool key_down(SDLKey key) {
        if (key==SDLK_ESCAPE and state==IN_MENU) {
            return false;
        } else if (key==SDLK_ESCAPE and state==IN_GAME) {
            state=IN_MENU;
            SdlManager::get()->unregister_listener(ship);
            ShipManager::get()->flush_waves();
            delete ship;
            ship=NULL;
        } else if (key==SDLK_ESCAPE and state==GAME_OVER) game_over_to_menu();
        return true;
    }
    void game_over_to_menu() {
        state=IN_MENU;
        SdlManager::get()->unregister_listener(ship);
        delete ship;
        ship=NULL;
        delete final_score_text;
        delete final_score;
        delete final_text;
        final_text=final_score_text=final_score=NULL;
        while (not hiscore_texts.empty()) { delete hiscore_texts.front(); hiscore_texts.pop_front(); }
        current_hiscore_text=NULL;
    }
    void display_health_and_score(float dt) {
        {
        std::stringstream ss;
        ss<<std::fixed<<std::setprecision(0)<<ship->health;
        ship_health->update(ss.str());
        } {
        std::stringstream ss;
        ss<<std::setw(10)<<std::setfill('0')<<ShipManager::get()->score;
        ship_score->update(ss.str());
        }

        ship_health->draw(dt);
        ship_score->draw(dt);
    }

    virtual bool frame_entered(float t,float dt) {
        if (state==IN_MENU) {
            blogo->alpha=(.4+.2*cos(2*M_PI*t/4.))/2.;
            blogo->draw(dt);
            flogo->draw(dt);
            main_menu->draw(dt);
            SdlManager::get()->get_mouse_position(cursor->x,cursor->y);
            cursor->draw(dt);
        } else if (state==GAME_START) {
            display_health_and_score(dt);

            if (not (title_start->draw(dt) and title_name->draw(dt))) {
                delete title_name;
                delete title_start;
                title_start=title_name=NULL;
                ShipManager::get()->schedule_wave(wavename);
                state=IN_GAME;
            }
        } else if (state==IN_GAME) {
            display_health_and_score(dt);

            if (ship->health<0) update_hiscores("GAME OVER",ShipManager::get()->score);
            else if (ShipManager::get()->wave_finished()) update_hiscores("WAVE COMPLETED",ShipManager::get()->score+1000*ship->health);
        } else if (state==GAME_OVER) {
            final_score_text->draw(dt);
            final_text->draw(dt);
            final_score->draw(dt);
            blink-=dt;
            if (blink<0 and current_hiscore_text) { blink+=.5; } //current_hiscore_text->alpha=1.-current_hiscore_text->alpha; current_hiscore_text->update_alpha(); }
            for (HiScoreTexts::const_iterator ii=hiscore_texts.begin(); ii!=hiscore_texts.end(); ii++) (*ii)->draw(dt);
        }
                
        for (Stars::const_iterator i=stars.begin(); i!=stars.end(); i++) {
            Star *current=*i;
            current->sprite->y+=current->v*dt;
            if (current->sprite->y>SdlManager::get()->height) {
                stars.erase(i);
                delete current;
                current=new Star;
                stars.insert(current);
            }
            current->sprite->draw(dt);
        }
        return true;
    }
    Sprite *flogo,*blogo;

    typedef std::list<Text*> HiScoreTexts;
    HiScoreTexts hiscore_texts;
    Text *current_hiscore_text;
    float blink;
    typedef std::multimap<int,std::string> HiScore;
    typedef std::map<std::string,HiScore> HiScores;
    void update_hiscores(const std::string &title,long int myscore) {
        ShipManager::get()->flush_waves();
        state=GAME_OVER;
        
        final_text=SpriteManager::get()->get_text(title,"font01",Text::CENTER);
        final_text->factorx=final_text->factory=2.5;
        final_text->x=SdlManager::get()->width/2.;
        final_text->y=SdlManager::get()->height/3.;

        final_score_text=SpriteManager::get()->get_text("final score","font00",Text::CENTER);
        final_score_text->x=final_text->x;
        final_score_text->y=final_text->y+100;

        std::stringstream ss;
        ss<<std::setw(10)<<std::setfill('0')<<ShipManager::get()->score;
        ship_score->update(ss.str());
        final_score=SpriteManager::get()->get_text(ss.str(),"font00",Text::CENTER);
        final_score->x=final_score_text->x;
        final_score->factorx=final_score->factory=1.5;
        final_score->y=final_score_text->y+40;

        const passwd *pass=getpwuid(getuid());
        std::string hiscorefile(pass->pw_dir);
        hiscorefile+="/.supershooter";
        HiScores hi;

        {
        std::ofstream ff(hiscorefile.c_str(),std::ios::app);
        ff<<wavename<<" "<<pass->pw_name<<" "<<myscore<<endl;
        }
        
        {
        std::ifstream ff(hiscorefile.c_str());
        while (not ff.eof()) {
            char user[32];
            char name[32];
            long int score;

            ff>>name>>std::ws>>user>>std::ws>>score;
            if (ff.fail()) break;

            hi[std::string(name)].insert(std::make_pair(score,user));
        }
        }

        HiScores::const_iterator ii=hi.find(wavename);
        if (ii==hi.end()) return;

        size_t k=5;
        float base_y=final_score->y+70;
        current_hiscore_text=NULL;
        HiScore::const_reverse_iterator jj=ii->second.rbegin();
        while (jj!=ii->second.rend() and k>0) {
            std::stringstream ss;
            ss<<std::setw(10)<<std::setfill('0')<<jj->first;
            Text *text=SpriteManager::get()->get_text(ss.str(),"font00",Text::CENTER);
            text->x=SdlManager::get()->width/2.;
            text->y=base_y;
            hiscore_texts.push_back(text);

            if (jj->second==pass->pw_name and jj->first==myscore) current_hiscore_text=text;

            k--;
            base_y+=32;
            jj++;
        }

        blink=0.5;
    }

    enum State {
        IN_MENU,
        GAME_START,
        IN_GAME,
        GAME_OVER
    };
    State state;
    
    struct Star {
        Star(bool initial=false) : v(5.+100.*rand()/(RAND_MAX+1.)) {
            sprite=dynamic_cast<StateSprite*>(SpriteManager::get()->get_sprite("star"));
            sprite->state=rand()%sprite->nstate;
            sprite->y=-SdlManager::get()->height*rand()/(RAND_MAX+1.);
            if (initial) sprite->y*=-1;
            sprite->x=SdlManager::get()->width*rand()/(RAND_MAX+1.);
            sprite->z=-9.5;
            sprite->alpha=.5;
            sprite->factorx=.25+.75*rand()/(RAND_MAX+1.);
            sprite->factory=sprite->factorx;
        }
        ~Star() { delete sprite; }
        StateSprite *sprite;
        float v;
    };

    typedef std::set<Star*> Stars;
    Stars stars;
    BigShip *ship;
    Text *ship_health;
    Text *ship_score;
    Text *final_score_text;
    Text *final_score;
    Text *final_text;

    struct Drifting {
        Drifting(const std::string &str,const std::string &font,float scale,float y,bool ltr) : text(SpriteManager::get()->get_text(str,font,Text::CENTER)), ltr(ltr), border(500), slow(SdlManager::get()->width/2.-100.), fast(SdlManager::get()->width/2.+100.) {
            text->factorx=text->factory=scale;
            text->y=y;
            if (ltr) { text->x=-border; v=1200; }
            else { text->x=SdlManager::get()->width+border; v=-1200; }
        }
        ~Drifting() { delete text; }
        bool draw(float dt) {
            if (text->x>SdlManager::get()->width+border or text->x<-border) return false;
            text->draw(dt);
            if (text->x>slow and text->x<fast) text->x+=.2*v*dt;
            else text->x+=v*dt;
            return true;
        }

        Text *text;
        float v;
        const float border,slow,fast;
        bool ltr;
    };
    Drifting *title_start;
    Drifting *title_name;
    std::string wavename;

    struct Menu {
        Menu(float base_y,float inc_y) : base_y(base_y), inc_y(inc_y), selected(NULL) {}
        ~Menu() { for (MenuItems::const_iterator i=items.begin(); i!=items.end(); i++) delete *i; }
        void append_item(const std::string &str) { items.insert(new MenuItem(str,base_y)); base_y+=inc_y; }
        void draw(float dt) { for (MenuItems::const_iterator i=items.begin(); i!=items.end(); i++) (*i)->text->draw(dt); }
        void reset() { selected=NULL; }
        bool is_selected(const std::string &title) const { return selected and selected->str==title; }
        void update(float x,float y)  {
            reset();
            for (MenuItems::const_iterator i=items.begin(); i!=items.end() and y>(*i)->top; i++) if ((*i)->bottom>y and x>(*i)->left and x<(*i)->right) {
                selected=*i;
                return;
            }
        }
        struct MenuItem {
            MenuItem(const std::string &str,float base_y) : text(SpriteManager::get()->get_text(str,"font00",Text::CENTER)), str(str) {
                text->x=SdlManager::get()->width/2.;
                text->y=base_y;
                top=text->y-text->h/2.;
                bottom=text->y+text->h/2.;
                left=text->x-text->w/2.;
                right=text->x+text->w/2.;
            }
            ~MenuItem() { delete text; }
            Text *text;
            std::string str;
            float top,bottom,left,right;
        };

        struct MenuItemOrder { bool operator()(const MenuItem *a,const MenuItem *b) { return a->top < b->top; } };
        typedef std::set<MenuItem*,MenuItemOrder> MenuItems;
        MenuItems items;
        MenuItem *selected;

        float base_y,inc_y;
    };
    Menu *main_menu;
    Sprite *cursor;
};




int main() {
    try {
        SdlManager::init();
        SdlManager::get()->set_background_color(0,0,0);

        SpriteManager::init();
        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->dump();

        CollisionManager::init();

        MessageManager::init();
        SdlManager::get()->register_listener(MessageManager::get());

        BulletManager::init();
        SdlManager::get()->register_listener(BulletManager::get());

        ShipManager::init();
        SdlManager::get()->register_listener(ShipManager::get());
        ShipManager::get()->dump();
        {
            //ShipManager::get()->launch_enemy_ship("bigship","main",400,0,M_PI/2.);
            //ShipManager::get()->launch_enemy_ship("bigship","main",600,0,M_PI/2.);
            //XmlShip *aa;
            //for (float y=200; y<=800; y+=50) ShipManager::get()->launch_enemy_ship("basicship","left",y,-100,M_PI/2.);
            //for (float y=200; y<=800; y+=50) ShipManager::get()->launch_enemy_ship("basicship","right",y,-50,M_PI/2.);

            //Killer killer;
            //BigShip bigship;
            //Pusher pusher;
            //SdlManager::get()->register_listener(&killer);
            //SdlManager::get()->register_listener(&pusher);
            //SdlManager::get()->register_listener(&bigship);

            Fps fps;
            MainMenu mainmenu;
            SdlManager::get()->register_listener(&mainmenu);
            SdlManager::get()->register_listener(&fps);
            SdlManager::get()->main_loop();

        }
        ShipManager::free();
        BulletManager::free();
        MessageManager::free();
        CollisionManager::free();
        SpriteManager::free();
        SdlManager::free();
    } catch (Except e) {
        e.dump();
        return 1;
    }
}


