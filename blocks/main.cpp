#include "except.h"
#include "utils.h"
#include "message.h"
#include "sound.h"
#include "gui.h"
#include "boardblocks.h"
#include "uct.h"
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
    virtual Token get_current_player() const = 0;
    virtual void submit_move(const Move *move) = 0;
};

class Player {
public:
    virtual void start_playing(const Board *board, const Move *oppmove, Submittable *answer) = 0;
};

class Observer {
public:
    virtual void board_updated(const Board *board) = 0;
};

class Referee : public Submittable {
public:
    Referee(int nw=22, int nh=14) : nw(nw), nh(nh), board(NULL), p1(NULL), p2(NULL), observer(NULL), winner(NOT_PLAYED) { }
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

        Token player = board->get_current_player();
        if (player==PLAYER_1) return "p1 playing";
        if (player==PLAYER_2) return "p2 playing";

        return "undefined";
    }
    virtual Token get_current_player() const {
        return get_const_board().get_current_player();
    }

    virtual void submit_move(const Move *move) {
        if (move) board->play_move(*move);

        winner = board->check_for_win(); 
        board_modified();

        banco();
    }

    Observer *observer;
    const int nw,nh;
protected:
    void banco() {
        if (winner!=NOT_PLAYED) return;

        Token player = board->get_current_player();
        if (player==PLAYER_1) p1->start_playing(board,board->get_const_lastmove(),this);
        if (player==PLAYER_2) p2->start_playing(board,board->get_const_lastmove(),this);
    }
    void board_modified() {
        if (not observer or not board) return;
        observer->board_updated(board);
    }

    Player *p1;
    Player *p2;
    BoardBlocks *board;
    Token winner;
};

class Bot : public Player {
public:
    Bot(Token player,float max_sec, float uct_constant) : player(player), max_sec(max_sec), uct_constant(uct_constant), answer(NULL), oppmove(NULL), board(NULL) {
        assert(player!=NOT_PLAYED);
        pthread_mutex_init(&init_mutex,NULL);
        pthread_cond_init(&init_complete,NULL);
        pthread_create(&thread,NULL,&Bot::main_loop,this);
    }
    ~Bot() {
        pthread_cancel(thread);
        pthread_join(thread,NULL);
        pthread_cond_destroy(&init_complete);
        pthread_mutex_destroy(&init_mutex);
    }
    virtual void start_playing(const Board *board, const Move *oppmove,Submittable *answer) {
        assert(board and answer);
        {
            pthread_mutex_lock(&init_mutex);
            this->board   = board;
            this->oppmove = oppmove;
            this->answer  = answer;
            pthread_cond_broadcast(&init_complete);
            cout<<"submitted"<<endl;
            pthread_mutex_unlock(&init_mutex);
        }
    }
protected:
    static void *main_loop(void *abstract) {
        cout<<"thread started"<<endl;
        Bot *bot = static_cast<Bot*>(abstract);
        const float max_sec = bot->max_sec;
        const Token player  = bot->player;

        Node *root = new Node(bot->uct_constant);

        while (true) {
            const Move *move = NULL;
            Submittable *local_answer = NULL;
            const Move *local_oppmove = NULL;
            const Board *local_board = NULL;

            { //wait for local data
                pthread_mutex_lock(&bot->init_mutex);

                cout<<"waiting for data"<<endl;
                pthread_cleanup_push(cleanup_root,root);
                pthread_cond_wait(&bot->init_complete,&bot->init_mutex); //also cancel point
                pthread_cleanup_pop(0);

                local_answer = bot->answer;
                local_board = bot->board->deepcopy();
                if (bot->oppmove) local_oppmove = bot->oppmove->deepcopy();
                cout<<"copied local data"<<endl;
                pthread_mutex_unlock(&bot->init_mutex);
            }

            assert(local_board);
            assert(local_answer);
            cout<<"--------------------------------------"<<endl;

            //reuse last simulations if possibles
            if (local_oppmove) {
                root = root->advance_and_detach(local_oppmove);
                delete local_oppmove;
            }
            Count saved_simulations=root->get_nb();
        
            clock_t start=clock(),end=clock();
            int k = 0;
            while (root->get_mode()==NORMAL and end-start<max_sec*CLOCKS_PER_SEC) {
                Board *copy=local_board->deepcopy();
                Token winner=root->play_random_game(copy,player);
                delete copy;
                end=clock();
                k++;
            }

            const Node *best_child=root->get_best_child();
            if (best_child) move = best_child->get_move();

            //simulation report
            std::cout<<"simulated "<<k<<" games ("<<saved_simulations<<" saved) in "<<float(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;

            //move report
            std::cout<<"playing ";
            switch (root->get_mode()) {
            case NORMAL:
                std::cout<<"normal "<<best_child->get_winning_probability()<<" ";
                break;
            case WINNER:
                std::cout<<"loosing ";
                break;
            case LOOSER:
                std::cout<<"winning ";
                break;
            }
            std::cout<<"move ";
            if (move) move->print();
            else cout<<"NULL";
            std::cout<<std::endl;

            //play best_move
            if (move) {
                root=root->advance_and_detach(move);
            }
            local_answer->submit_move(move);
            cout<<"submitted move"<<endl;

            //delete stuff
            delete local_board;
        }

        delete root;

        cout<<"return from thread"<<endl;
        return NULL;
    }

    static void cleanup_root(void *abstract) {
        cout<<"cleaning up root"<<endl;
        Node *root = static_cast<Node*>(abstract);
        delete root;
    }

    pthread_t thread;
    pthread_mutex_t init_mutex;
    pthread_cond_t  init_complete;
    const Token player;
    const float max_sec;
    const float uct_constant;
    const Board *board;
    const Move  *oppmove;
    Submittable *answer;
    
};

static Submittable *hanswer = NULL;

void pixel_callback(Button *abstract) {
    assert(hanswer);
    Pixel *but = dynamic_cast<Pixel*>(abstract);
    assert(but);
    MoveBlocks move(hanswer->get_current_player(),but->get_color());
    MessageManager::get()->add_message("human played");
    hanswer->submit_move(&move);
}

class MainApp : public Listener, public Player, public Observer {
public:
    static const float SCORESEPARATION = 40;
    MainApp() : spacing(35), bot(NULL) { 
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
    virtual void start_playing(const Board *board, const Move *oppmove,Submittable *answer) {
        assert(answer==hanswer);
        //if (oppmove and oppmove->player==PLAYER_1) bot->start_playing(board,oppmove,answer);
        if (sync_board(static_cast<const BoardBlocks*>(board),true)) MessageManager::get()->add_message("human playing");
        else {
            MessageManager::get()->add_message("no possible move");
            answer->submit_move(NULL);
        }
    }
    virtual void board_updated(const Board *board) {
        MessageManager::get()->add_message("updating board");
        sync_board(static_cast<const BoardBlocks*>(board),false);
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
        bot = new Bot(PLAYER_2,.8,.2);
        referee.reset(this,bot);
        pixels->enabled = true;
    }
    virtual void unregister_self() {
        pixels->enabled = false;
        delete bot;
        bot = NULL;
    }
    bool sync_board(const BoardBlocks *board, bool update_playable) {
        bool any = false;
        for (int row=0; row<referee.nh; row++) for (int column=0; column<referee.nw; column++) {
            const BoardBlocks::TokenBlocks &token = board->get_const_token(row,column);
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
        ss<<board->get_p1score();
        p1score->update(ss.str());
        } {
        std::stringstream ss;
        ss<<board->get_p2score();
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
    Bot *bot;
};

int main() {
    try {
        srand(time(NULL));
        cout.precision(2);

        SdlManager::init();
        SdlManager::get()->set_background_color(0,0,.05);

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

