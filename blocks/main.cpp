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

void pixel_callback(Button *but); //forward

class Pixel : public Button {
public:
    static const float MAGNIFYFACTOR = 1.2;
    Pixel() : Button("blocks",pixel_callback), magnifiable(false), color(NONE) {
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

    void update(int state,bool playable,Color color) {
        magnifiable = playable;
        casted->state = state;
        this->color = color;
    }

    Color get_color() const { return color; }
private:
    Color color;
    bool magnifiable;
    StateSprite *casted;
};

class Submittable {
public:
    virtual void submit_move(const MoveBlocks *move) = 0;
};

class Player {
public:
    virtual void start_playing(const BoardBlocks &board, const MoveBlocks &oppmove,Submittable *answer) = 0;
};

class Observer {
public:
    virtual void board_updated(const BoardBlocks &board) = 0;
};

class Referee : public Submittable {
public:
    Referee(int nw=22, int nh=14) : nw(nw), nh(nh), board(NULL), p1(NULL), p2(NULL), observer(NULL), winner(NOT_PLAYED) {}
    ~Referee() {
        if (board) delete board;
    }
    void reset(Player *p1, Player *p2) {
        assert(winner == NOT_PLAYED);
        assert(not board);
        assert(not this->p1 and p1);
        assert(not this->p2 and p2);

        this->p1 = p1;
        this->p2 = p2;
        this->board = new BoardBlocks(nw,nh,true);
        board_modified();

        banco();
    }
    const BoardBlocks &get_const_board() const {
        assert(board);
        return *board;
    }
    std::string get_status() const {
        if (not board) return "uninitialized";

        if (winner==PLAYER_1) return "p1 win";
        if (winner==PLAYER_2) return "p2 win";

        const MoveBlocks &lastmove = board->get_const_lastmove();
        if (lastmove.player==PLAYER_1) return "p2 playing";
        if (lastmove.player==PLAYER_2) return "p1 playing";

        return "undefined";
    }
    virtual void submit_move(const MoveBlocks *move) {
        if (move) {
            assert(winner == NOT_PLAYED);
            const MoveBlocks &lastmove = board->get_const_lastmove();
            assert(lastmove.player==other_player(move->player));
            board->play_move(*move);
        }

        winner = board->check_for_win(); 
        board_modified();

        banco();
    }

    Observer *observer;
    const int nw,nh;
protected:
    void banco() {
        if (winner!=NOT_PLAYED) return;
        const MoveBlocks &lastmove = board->get_const_lastmove();
        assert(lastmove.player!=NOT_PLAYED);
        if (lastmove.player==PLAYER_1) p2->start_playing(*board,lastmove,this);
        if (lastmove.player==PLAYER_2) p1->start_playing(*board,lastmove,this);
    }
    void board_modified() {
        if (not observer or not board) return;
        observer->board_updated(*board);
    }

    Player *p1;
    Player *p2;
    BoardBlocks *board;
    Token winner;
};

static Referee *hanswer = NULL;

void pixel_callback(Button *abstract) {
    assert(hanswer);
    const Token player = other_player(hanswer->get_const_board().get_const_lastmove().player);
    Pixel *but = dynamic_cast<Pixel*>(abstract);
    assert(but);
    MoveBlocks move(player,but->get_color());
    MessageManager::get()->add_message("human played");
    hanswer->submit_move(&move);
}

class MainApp : public Listener, public Player, public Observer {
public:
    static const float SCORESEPARATION = 40;
    MainApp() : spacing(35) { 
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

        referee.observer = this;
        hanswer = &referee;

        pixels = new Array(referee.nw,referee.nh);
        GuiManager::get()->add_widget(pixels,"pixels");
        pixels->enabled = false;

        for (int row=0; row<referee.nh; row++) for (int column=0; column<referee.nw; column++) {
            Pixel *pixel = new Pixel;
            pixel->sprite->y = SdlManager::get()->height/2. + (pixel->sprite->h+5)*(row-(referee.nh-1)/2.);
            pixel->sprite->x = SdlManager::get()->width/2.  + (pixel->sprite->w+5)*(column-(referee.nw-1)/2.);
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
    virtual void start_playing(const BoardBlocks &board, const MoveBlocks &oppmove,Submittable *answer) {
        assert(answer==hanswer);
        if (sync_board(board,true)) MessageManager::get()->add_message("human playing");
        else {
            MessageManager::get()->add_message("no possible move");
            answer->submit_move(NULL);
        }
    }
    virtual void board_updated(const BoardBlocks &board) {
        MessageManager::get()->add_message("updating board");
        sync_board(board,false);
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
        referee.reset(this,this);
        pixels->enabled = true;
    }
    virtual void unregister_self() {
        pixels->enabled = false;
    }
    bool sync_board(const BoardBlocks &board, bool update_playable) {
        bool any = false;
        for (int row=0; row<referee.nh; row++) for (int column=0; column<referee.nw; column++) {
            const BoardBlocks::TokenBlocks &token = board.get_const_token(row,column);
            Pixel *pixel = static_cast<Pixel*>(pixels->get_widget(row,column));
            assert(pixel);
            int state = token.color;
            if (token.player==PLAYER_1)  state += 6;
            if (token.player==PLAYER_2)  state += 12;
            if (row==referee.nh-1 and column==0) state = 18;
            if (row==0 and column==referee.nw-1) state = 19;
            any |= token.playable;
            pixel->update(state,token.playable and update_playable,token.color);
        }

        {
        std::stringstream ss;
        ss<<board.get_p1score();
        p1score->update(ss.str());
        } {
        std::stringstream ss;
        ss<<board.get_p2score();
        p2score->update(ss.str());
        } {
        status->update(referee.get_status());
        }

        return any and update_playable;
    }
    const float spacing;

    Referee referee;
    Text *p1score;
    Text *p2score;
    Text *status;
    Sfx *endsfx;
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

