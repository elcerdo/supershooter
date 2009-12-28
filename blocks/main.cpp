#include "except.h"
#include "utils.h"
#include "message.h"
#include "sound.h"
#include <cassert>
#include <set>
#include <queue>
using std::cerr;
using std::cout;
using std::endl;
using std::make_pair;

#define NCOLORS 6
#define MAGNIFYFACTOR 1.2
#define DEBUGMSG(format, ...) printf(format,##__VA_ARGS__)
#define SCORESEPARATION 40

struct Pixel {
    Pixel(float x,float y,unsigned int state) : magnified(false), playable(false), inqueue(false), left(NULL), right(NULL), top(NULL), bottom(NULL) {
        sprite = dynamic_cast<StateSprite*>(SpriteManager::get()->get_sprite("blocks"));
        assert(sprite);
        sprite->x = x;
        sprite->y = y;
        assert(state < sprite->nstate);
        sprite->state = state;
    }
    ~Pixel() {
        delete sprite;
    }
    void draw(float dt) {
        if (magnified) {
            sprite->factorx *= MAGNIFYFACTOR;
            sprite->factory *= MAGNIFYFACTOR;
        }
        sprite->draw(dt);
        if (magnified) {
            sprite->factorx /= MAGNIFYFACTOR;
            sprite->factory /= MAGNIFYFACTOR;
        }
    }

    bool magnified;
    bool playable;
    bool inqueue;
    Pixel *left;
    Pixel *right;
    Pixel *top;
    Pixel *bottom;
    StateSprite *sprite;
};

typedef std::set<Pixel*> PixelsSet;
typedef std::pair<int,Pixel*> Seed;

struct SeedGreater {
    bool operator()(const Seed &a, const Seed &b) { return a.first > b.first; }
};

typedef std::priority_queue<Seed,std::vector<Seed>,SeedGreater> SeedsQueue;

class MainApp : public Listener {
public:
    MainApp() : nw(21), nh(14), size(nw*nh), spacing(35) { 
        cursor = SpriteManager::get()->get_sprite("cursor");
        cursor->cx = cursor->w/2.;
        cursor->cy = cursor->h/2.;
        cursor->z = 8;

        click = SoundManager::get()->get_sfx("click");

        endsfx = SoundManager::get()->get_sfx("boom");

        p1score = SpriteManager::get()->get_text("0","font00",Text::RIGHT);
        p1score->x = SdlManager::get()->width/2.-SCORESEPARATION;
        p1score->y = SdlManager::get()->height-100;

        p2score = SpriteManager::get()->get_text("0","font00",Text::LEFT);
        p2score->x = SdlManager::get()->width/2.+SCORESEPARATION;
        p2score->y = SdlManager::get()->height-100;

        status = SpriteManager::get()->get_text("status","font00",Text::CENTER);
        status->x = SdlManager::get()->width/2.;
        status->y = 100;

        pixels = new Pixel*[size];
        const float cx = (SdlManager::get()->width-spacing*(nw-1))/2.;
        const float cy = (SdlManager::get()->height-spacing*(nh-1))/2.;
        for (int i=0; i<nh; i++) for (int j=0; j<nw; j++) {
            get_pixel(i,j) = new Pixel(cx+spacing*j,cy+spacing*i,rand()%NCOLORS);
        }
        for (int i=0; i<nh; i++) for (int j=0; j<nw; j++) {
            Pixel *current = get_pixel(i,j);
            if (i != 0)    current->top    = get_pixel(i-1,j);
            if (i != nh-1) current->bottom = get_pixel(i+1,j);
            if (j != 0)    current->left   = get_pixel(i,j-1);
            if (j != nw-1) current->right  = get_pixel(i,j+1);
        }

        { // player 1 start
            Pixel *start = get_pixel(nh-1,0);
            start->sprite->state = 18;
            p1pixels.insert(start);
        }

        { // player 2 start
            Pixel *start = get_pixel(0,nw-1);
            start->sprite->state = 19;
            p2pixels.insert(start);
        }

        state = P1PLAYING;
        last_played = -1;
        update_playable();
        MessageManager::get()->add_message("p1 starts");
        status->update("p1 starts");
    }
    ~MainApp() {
        unregister_self();
        for (int k=0; k<size; k++) { delete pixels[k]; }
        delete pixels;
        delete cursor;
        delete p1score;
        delete p2score;
        delete status;
        delete click;
        delete endsfx;
    }
protected:
    Pixel*& get_pixel(int i,int j) { return pixels[i*nw+j]; }
    Pixel*  get_hoovered_pixel(float x,float y) {
        const float cj = (2*x-SdlManager::get()->width+spacing*nw)/(2*spacing);
        const float ci = (2*y-SdlManager::get()->height+spacing*nh)/(2*spacing);
        if (ci>=0 and cj>=0 and cj<nw and ci<nh) { return get_pixel(ci,cj); }
        else { return NULL; }
    }
    bool update_playable() {
        assert(state==P1PLAYING or state==P2PLAYING);

        for (int k=0; k<size; k++) { pixels[k]->playable = false; }

        //update playable pixels
        PixelsSet *playerpixels = (state == P1PLAYING) ? &p1pixels : &p2pixels;
        for (PixelsSet::const_iterator i=playerpixels->begin(); i!= playerpixels->end(); i++) {
            const Pixel *current = *i;
            if (current->top and current->top->sprite->state<6 and current->top->sprite->state!=last_played)          current->top->playable    = true;
            if (current->bottom and current->bottom->sprite->state<6 and current->bottom->sprite->state!=last_played) current->bottom->playable = true;
            if (current->left and current->left->sprite->state<6 and current->left->sprite->state!=last_played)       current->left->playable   = true;
            if (current->right and current->right->sprite->state<6 and current->right->sprite->state!=last_played)    current->right->playable  = true;
        }

        //update locked
        int n=0;
        for (int k=0; k<size; k++) if (pixels[k]->playable) { n++; }
        return (n==0);
    }
    void play_move(Pixel *playpixel) {
        if (state!=P1PLAYING and state!=P2PLAYING) {
            DEBUGMSG("not in playing state!!\n");
            return;
        }

        DEBUGMSG("------------\n");

        assert(playpixel->playable);
        last_played = playpixel->sprite->state;
        PixelsSet *playerpixels = (state == P1PLAYING) ? &p1pixels : &p2pixels;
        const int offset        = (state == P1PLAYING) ? 6 : 12;
        const int start_state   = (state == P1PLAYING) ? 18 : 19;

        //reset inqueue state and find initial seeds
        SeedsQueue wonpixels;
        for (int k=0; k<size; k++) {
            Pixel *current = pixels[k];
            current->inqueue = false;
            if (current->playable and current->sprite->state==last_played) { wonpixels.push(make_pair(0,current)); current->inqueue = true; }
        }

        //fast marching on seeds
        int k = 0;
        while (not wonpixels.empty()) {
            int top_distance = wonpixels.top().first;
            Pixel *top_pixel = wonpixels.top().second;
            fflush(stdout);
            wonpixels.pop();

            assert(top_pixel->sprite->state==last_played);
            playerpixels->insert(top_pixel);
            if (top_pixel->top and top_pixel->top->inqueue==false and top_pixel->top->sprite->state==last_played) { wonpixels.push(make_pair(top_distance+1,top_pixel->top)); top_pixel->top->inqueue = true; }
            if (top_pixel->bottom and top_pixel->bottom->inqueue==false and top_pixel->bottom->sprite->state==last_played) { wonpixels.push(make_pair(top_distance+1,top_pixel->bottom)); top_pixel->bottom->inqueue = true; }
            if (top_pixel->left and top_pixel->left->inqueue==false and top_pixel->left->sprite->state==last_played) { wonpixels.push(make_pair(top_distance+1,top_pixel->left)); top_pixel->left->inqueue = true; }
            if (top_pixel->right and top_pixel->right->inqueue==false and top_pixel->right->sprite->state==last_played) { wonpixels.push(make_pair(top_distance+1,top_pixel->right)); top_pixel->right->inqueue = true; }
            k++;
        }
        DEBUGMSG("won %d pixels\n",k);

        //update set color
        for (PixelsSet::const_iterator i=playerpixels->begin(); i!=playerpixels->end(); i++) if ((*i)->sprite->state!=start_state) {
            (*i)->sprite->state = last_played+offset;
        }

        { //update score display
            char foo[10];
            snprintf(foo,10,"%d",p1pixels.size());
            p1score->update(foo);
            snprintf(foo,10,"%d",p2pixels.size());
            p2score->update(foo);
        }

        //switch players and look for end of game
        if (state == P1PLAYING) { //player 1 ends its turn
            state = P2PLAYING;
            const bool p2locked = update_playable();
            DEBUGMSG("player 2 locked = %d\n",p2locked);
            if (p2locked) {
                state = P1PLAYING;
                const bool p1locked = update_playable();
                DEBUGMSG("player 1 locked = %d\n",p1locked);
                //if (p1locked) {
                    const int p1score = p1pixels.size();
                    const int p2score = p2pixels.size();
                    if (p1score > p2score) {
                        MessageManager::get()->add_message("p1 score win");
                        status->update("p1 score win");
                        state = P1WIN;
                    } else if (p1score == p2score) {
                        MessageManager::get()->add_message("draw");
                        status->update("draw");
                        state = DRAW;
                    } else {
                        MessageManager::get()->add_message("p2 score win");
                        status->update("p2 score win");
                        state = P2WIN;
                    }
                //} else {
                //    MessageManager::get()->add_message("p1 win");
                //    status->update("p1 win");
                //    state = P1WIN;
                //}
                endsfx->play_once();
            } else {
                MessageManager::get()->add_message("p2 plays");
                status->update("p2");
            }
        } else { //player 2 ends its turn
            state = P1PLAYING;
            const bool p1locked = update_playable();
            DEBUGMSG("player 1 locked = %d\n",p1locked);
            if (p1locked) {
                state = P2PLAYING;
                const bool p2locked = update_playable();
                DEBUGMSG("player 2 locked = %d\n",p2locked);
                //if (p2locked) {
                    const int p1score = p1pixels.size();
                    const int p2score = p2pixels.size();
                    if (p1score > p2score) {
                        MessageManager::get()->add_message("p1 score win");
                        status->update("p1 score win");
                        state = P1WIN;
                    } else if (p1score == p2score) {
                        MessageManager::get()->add_message("draw");
                        status->update("draw");
                        state = DRAW;
                    } else {
                        MessageManager::get()->add_message("p2 score win");
                        status->update("p2 score win");
                        state = P2WIN;
                    }
                //} else {
                //    MessageManager::get()->add_message("p2 win");
                //    status->update("p2 win");
                //    state = P2WIN;
                //}
                endsfx->play_once();
            } else {
                MessageManager::get()->add_message("p1 plays");
                status->update("p1");
            }
        }
    }

    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_f:
            SdlManager::get()->toogle_fullscreen();
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
        if (button == 1) {
            Pixel *current = get_hoovered_pixel(x,y);
            if (current and current->playable) {
                click->play_once();
                play_move(current);
            }
        }
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        SdlManager::get()->get_mouse_position(cursor->x,cursor->y);
        cursor->draw(dt);

        Pixel *hoovered = get_hoovered_pixel(cursor->x,cursor->y);
        if (hoovered and hoovered->playable) hoovered->magnified = true;
        for (int k=0; k<size; k++) { pixels[k]->draw(dt); }
        if (hoovered and hoovered->playable) hoovered->magnified = false;

        p1score->draw(dt);
        p2score->draw(dt);
        status->draw(dt);

        return true;
    }
    virtual void unregister_self() {
    }
    const int nw;
    const int nh;
    const int size;
    const float spacing;

    enum State { P1PLAYING, P2PLAYING, P1WIN, P2WIN, DRAW };
    State state;
    int last_played;

    Pixel **pixels;
    Sprite *cursor;
    PixelsSet p1pixels;
    PixelsSet p2pixels;
    Text *p1score;
    Text *p2score;
    Text *status;
    Sfx *click;
    Sfx *endsfx;
};

int main() {
    try {
        srand(time(NULL));

        SdlManager::init();
        SdlManager::get()->set_background_color(0,0,0);

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

