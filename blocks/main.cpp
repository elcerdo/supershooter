#include "except.h"
#include "utils.h"
#include "message.h"
#include "sound.h"
#include "gui.h"
#include "boardblocks.h"
#include <sstream>
using std::cerr;
using std::endl;
using std::cout;

class Pixel : public Button {
public:
    static const float MAGNIFYFACTOR = 1.2;
    Pixel(Size row, Size column) : Button("blocks",callback), magnifiable(false), row(row), column(column) {
        casted = dynamic_cast<StateSprite*>(sprite);
        assert(casted);

    }

    virtual bool interact(float x, float y) {
        bool valid = is_click_valid(x,y);
        if (valid and magnifiable and clicked) {
            clicked(this);
            return true;
        }
        return false;
    }

    virtual bool draw(float x,float y,float dt) const {
        if (not enabled) return false;

        if (magnifiable and is_click_valid(x,y)) {
            sprite->factorx *= MAGNIFYFACTOR;
            sprite->factory *= MAGNIFYFACTOR;
            sprite->draw(dt);
            sprite->factorx /= MAGNIFYFACTOR;
            sprite->factory /= MAGNIFYFACTOR;
            return true;
        } 

        sprite->draw(dt);
        return true;
    }

    void update(int state,bool playable) {
        magnifiable = playable;
        casted->state = state;
    }

    const Size row,column;
private:
    bool magnifiable;
    StateSprite *casted;

    static void callback(Button *but) {
        cout<<"callback"<<endl;
        but->sprite->dump();
    }
};

class MainApp : public Listener {
public:
    static const float SCORESEPARATION = 40;
    MainApp() : nw(22), nh(14), spacing(35), board(NULL) { 
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

        pixels = new Array(nw,nh);
        GuiManager::get()->add_widget(pixels,"pixels");

        for (int row=0; row<nh; row++) for (int column=0; column<nw; column++) {
            Pixel *pixel = new Pixel(row,column);
            pixel->sprite->y = SdlManager::get()->height/2. + (pixel->sprite->h+5)*(row-(nh-1)/2.);
            pixel->sprite->x = SdlManager::get()->width/2.  + (pixel->sprite->w+5)*(column-(nw-1)/2.);
            pixels->add_widget(pixel,row,column);
        }

        MessageManager::get()->add_message("init");
    }
    ~MainApp() {
        unregister_self();
        delete p1score;
        delete p2score;
        delete status;
        delete endsfx;
    }
protected:
    //Pixel*  get_hoovered_pixel(float x,float y) {
    //    const float cj = (2*x-SdlManager::get()->width+spacing*nw)/(2*spacing);
    //    const float ci = (2*y-SdlManager::get()->height+spacing*nh)/(2*spacing);
    //    if (ci>=0 and cj>=0 and cj<nw and ci<nh) { return get_pixel(ci,cj); }
    //    else { return NULL; }
    //}
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
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        p1score->draw(dt);
        p2score->draw(dt);
        status->draw(dt);

        return true;
    }
    virtual void register_self() {
        assert(not board);
        board = new BoardBlocks(nw,nh,true);
        sync_board();
        pixels->enabled = true;
    }
    virtual void unregister_self() {
        delete board;
        board = NULL;
        pixels->enabled = false;
    }
    void sync_board() {
        assert(board);
        for (int row=0; row<nh; row++) for (int column=0; column<nw; column++) {
            const BoardBlocks::TokenBlocks &token = board->get_const_token(row,column);
            Pixel *pixel = static_cast<Pixel*>(pixels->get_widget(row,column));
            assert(pixel);
            int state = token.color;
            if (token.player==PLAYER_1)  state += 6;
            if (token.player==PLAYER_2)  state += 12;
            if (row==nh-1 and column==0) state = 18;
            if (row==0 and column==nw-1) state = 19;
            pixel->update(state,token.playable);
        }

        {
        std::stringstream ss;
        ss<<board->get_p1score();
        p1score->update(ss.str());
        } {
        std::stringstream ss;
        ss<<board->get_p2score();
        p2score->update(ss.str());
        }
    }
    const int nw;
    const int nh;
    const float spacing;

    Text *p1score;
    Text *p2score;
    Text *status;
    Sfx *endsfx;
    BoardBlocks *board;
    Array *pixels;
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

        GuiManager::init();
        SdlManager::get()->register_listener(GuiManager::get());
        GuiManager::get()->add_sound_widgets();

        {
            Fps fps;
            fps.set_display(true);
            MainApp mainapp;
            SdlManager::get()->register_listener(&fps);
            SdlManager::get()->register_listener(&mainapp);

            SdlManager::get()->main_loop();
        }

        GuiManager::free();
        SoundManager::free();
        MessageManager::free();
        SpriteManager::free();
        SdlManager::free();
    } catch (Except e) {
        e.dump();
        return 1;
    }
}

