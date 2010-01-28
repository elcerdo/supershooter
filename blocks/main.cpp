#include "except.h"
#include "utils.h"
#include "message.h"
#include "sound.h"
#include "gui.h"
#include "boardblocks.h"
#include "uct.h"
#include <sstream>
#include <iomanip>
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
        if (not magnifiable) return false;
        if (not Button::interact(x,y)) return false;
        return true;
    }

    virtual bool draw(float x,float y,float dt) const {
        if (not enabled) return false;

        if (magnifiable and Widget::is_click_valid(sprite,x,y)) {
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
        winner = NOT_PLAYED;
        if (board) {
            delete board;
            board = NULL;
            this->p1 = NULL;
            this->p2 = NULL;
        }
        assert(not board);
        assert(not this->p1 and p1);
        assert(not this->p2 and p2);

        this->p1 = p1;
        this->p2 = p2;
        this->board = new BoardBlocks(nw,nh,true);
        board_modified();

        banco();
    }
    bool game_ended() const {
        return winner!=NOT_PLAYED;
    }
    const BoardBlocks &get_const_board() const {
        assert(board);
        return *board;
    }
    std::string get_status() const {
        if (not board) return "uninitialized";

        if (winner==PLAYER_1) return "p1 won";
        if (winner==PLAYER_2) return "p2 won";

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
        if (winner==PLAYER_1) MessageManager::get()->add_message("p1 won");
        if (winner==PLAYER_2) MessageManager::get()->add_message("p2 won");
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
    Bot(Token player,float max_sec, float uct_constant, bool god_mode) : player(player), max_sec(max_sec), god_mode(god_mode), uct_constant(uct_constant), answer(NULL), oppmove(NULL), board(NULL), is_waiting(false), hail_for_waiting(false) {
        assert(player!=NOT_PLAYED);
        pthread_mutex_init(&init_mutex,NULL);
        pthread_mutex_init(&wait_mutex,NULL);
        pthread_mutex_init(&hail_mutex,NULL);
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
        assert(board);
        assert(answer);

        //hail thread to become waiting
        pthread_mutex_lock(&hail_mutex);
        hail_for_waiting = true;
        pthread_mutex_unlock(&hail_mutex);

        //wait for thread to be pending for submission
        pthread_mutex_lock(&wait_mutex);
        while (not is_waiting) {
            pthread_mutex_unlock(&wait_mutex);
            usleep(10); //avoid spin lock
            pthread_mutex_lock(&wait_mutex);
        }
        pthread_mutex_unlock(&wait_mutex);

        //submit new board and opponent move for computation
        pthread_mutex_lock(&init_mutex);
        this->board   = board;
        this->oppmove = oppmove;
        this->answer  = answer;

        //reset thread waiting state
        pthread_mutex_lock(&wait_mutex);
        is_waiting = false;
        pthread_mutex_unlock(&wait_mutex);

        //reset thread hail state
        pthread_mutex_lock(&hail_mutex);
        hail_for_waiting = false;
        pthread_mutex_unlock(&hail_mutex);

        pthread_cond_broadcast(&init_complete);
        pthread_mutex_unlock(&init_mutex);
    }
protected:
    static void *main_loop(void *abstract) {
        Bot *bot = static_cast<Bot*>(abstract);
        const float max_sec = bot->max_sec;
        const bool god_mode = bot->god_mode;
        const Token player  = bot->player;
        cout<<"bot thread started id="<<std::hex<<pthread_self()<<std::dec<<" player="<<player<<" max_sec="<<max_sec<<" god_mode="<<god_mode<<endl; //cancel point

        Node *root = new Node(bot->uct_constant);

        //beware: all system call are cancel point!!!
        while (true) {
            Submittable *local_answer = NULL;
            Board *local_board = NULL;
            Move *local_oppmove = NULL;

            { //wait for local data
                pthread_mutex_lock(&bot->init_mutex);

                pthread_mutex_lock(&bot->wait_mutex);
                bot->is_waiting = true;
                pthread_mutex_unlock(&bot->wait_mutex);

                pthread_cleanup_push(cleanup_root,root);
                pthread_cond_wait(&bot->init_complete,&bot->init_mutex); //also cancel point
                pthread_cleanup_pop(0);

                local_answer = bot->answer;
                local_board = bot->board->deepcopy();
                if (bot->oppmove) local_oppmove = bot->oppmove->deepcopy();
                pthread_mutex_unlock(&bot->init_mutex);
            }

            if (local_oppmove) { //reuse last simulations if possibles
                root = root->advance_and_detach(local_oppmove);
                delete local_oppmove;
                local_oppmove = NULL;
            }
        
            { //simulate games
                pthread_cleanup_push(cleanup_root,root);
                pthread_cleanup_push(cleanup_board,local_board);

                int nsimu = 0;
                clock_t start=clock();
                clock_t duration = 0;
                while (root->get_mode()==NORMAL and duration<max_sec*CLOCKS_PER_SEC) {
                    Board *copy=local_board->deepcopy();
                    Token winner=root->play_random_game(copy,player);
                    delete copy;
                    duration = clock()-start;
                    nsimu++;
                    pthread_testcancel(); //cancel point
                }

                cout<<endl<<"bot "<<player<<" main computation"<<endl;
                print_report(root,nsimu,duration);

                pthread_cleanup_pop(0);
                pthread_cleanup_pop(0);
            }


            { //send best move to submitable and play it on local board
                const Move *best_move = NULL;
                const Node *best_child=root->get_best_child();
                if (best_child) best_move = best_child->get_move(); //this is a normal move

                if (best_move) { //play submitted best move play best_move
                    root=root->advance_and_detach(best_move);
                    local_board->play_move(*best_move);
                }

                pthread_cleanup_push(cleanup_root,root);
                pthread_cleanup_push(cleanup_board,local_board);
                local_answer->submit_move(best_move); //potentially cancel point
                pthread_cleanup_pop(0);
                pthread_cleanup_pop(0);
            }

            if (god_mode) { //simulate more games
                pthread_cleanup_push(cleanup_root,root);
                pthread_cleanup_push(cleanup_board,local_board);

                int nsimu = 0;
                clock_t start = clock();
                pthread_mutex_lock(&bot->hail_mutex);
                while (root->get_mode()==NORMAL and not bot->hail_for_waiting) {
                    pthread_mutex_unlock(&bot->hail_mutex);

                    Board *copy=local_board->deepcopy();
                    Token winner=root->play_random_game(copy,other_player(player)); //oppenent play first
                    delete copy;
                    nsimu++;

                    pthread_testcancel(); //cancel point

                    pthread_mutex_lock(&bot->hail_mutex);
                }
                pthread_mutex_unlock(&bot->hail_mutex);

                cout<<endl<<"bot "<<player<<" overtime computation"<<endl;
                print_report(root,nsimu,clock()-start);

                pthread_cleanup_pop(0);
                pthread_cleanup_pop(0);
            }

            //delete stuff
            delete local_board;
        }


        assert(false);
        return NULL;
    }

    static void print_report(const Node *root, int nsimu, clock_t duration) { //is a cancel point
        cout<<"simulated "<<nsimu<<" games in "<<float(duration)/CLOCKS_PER_SEC<<"s (total "<<root->get_nb()<<" games)";
        if (duration) cout<<" "<<float(nsimu*CLOCKS_PER_SEC)/duration<<"simu/s";
        cout<<endl;

        const Move *best_move = NULL;
        const Node *best_child=root->get_best_child();
        if (best_child) best_move = best_child->get_move(); //this is a normal move

        if (best_move) {
            cout<<"best move is ";
            best_move->print();
        } else cout<<"no move";
        cout<<endl;

        //move report
        switch (root->get_mode()) {
        case NORMAL:
            cout<<std::fixed<<std::setprecision(0)<<100.*best_child->get_winning_probability()<<"\% chance of win";
            break;
        case WINNER:
            cout<<"loosing";
            break;
        case LOOSER:
            cout<<"sure win";
            break;
        }
        cout<<endl;
    }

    static void cleanup_root(void *abstract) {
        cout<<"cleaning up root"<<endl;
        Node *root = static_cast<Node*>(abstract);
        delete root;
    }

    static void cleanup_board(void *abstract) {
        cout<<"cleaning up board"<<endl;
        Board *board = static_cast<Board*>(abstract);
        delete board;
    }

    pthread_t thread;
    pthread_mutex_t init_mutex, wait_mutex, hail_mutex;
    pthread_cond_t init_complete;
    const Token player;
    const float max_sec;
    const bool god_mode;
    const float uct_constant;
    const Board *board;
    const Move *oppmove;
    Submittable *answer;
    bool is_waiting;
    bool hail_for_waiting;
};

static Submittable *hanswer = NULL;

void pixel_callback(Button *abstract) {
    assert(hanswer);
    Pixel *but = dynamic_cast<Pixel*>(abstract);
    assert(but);
    MoveBlocks move(hanswer->get_current_player(),but->get_color());
    //MessageManager::get()->add_message("human played");
    hanswer->submit_move(&move);
}

class GameApp : public Listener, public Player, public Observer {
public:
    static const float SCORESEPARATION = 40;
    GameApp() : spacing(35), bot1(NULL), bot2(NULL), p1human(true), p2human(true), bot1god(false), bot2god(false) { 
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
    }
    ~GameApp() {
        delete p1score;
        delete p2score;
        delete status;
        delete endsfx;
    }
    virtual void start_playing(const Board *board, const Move *oppmove,Submittable *answer) {
        assert(answer==hanswer);
        if (!sync_board(static_cast<const BoardBlocks*>(board),true)) {
            MessageManager::get()->add_message("no possible move");
            answer->submit_move(NULL);
        } // else MessageManager::get()->add_message("human playing");
    }
    virtual void board_updated(const Board *board) {
        //MessageManager::get()->add_message("updating board");
        sync_board(static_cast<const BoardBlocks*>(board),false);
    }

    bool game_ended() { return referee.game_ended(); }
    bool p1human;
    bool p2human;
    bool bot1god;
    bool bot2god;
protected:
    virtual bool frame_entered(float t,float dt) {
        p1score->draw(dt);
        p2score->draw(dt);
        status->draw(dt);
        return true;
    }
    virtual void register_self() {
        MessageManager::get()->add_message("init");
        assert(not bot1 or not bot2);
        Player *p1 = this;
        Player *p2 = this;
        if (not p1human) {
            bot1 = new Bot(PLAYER_1,1,.2,bot1god);
            p1 = bot1;
        }
        if (not p2human) {
            bot2 = new Bot(PLAYER_2,1,.2,bot2god);
            p2 = bot2;
        }
        referee.reset(p1,p2);
        pixels->enabled = true;
    }
    virtual void unregister_self() {
        MessageManager::get()->add_message("uninit");
        pixels->enabled = false;

        if (bot1) { delete bot1; bot1=NULL; }
        if (bot2) { delete bot2; bot2=NULL; }
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
    Bot *bot1;
    Bot *bot2;
};

class MainApp : public Listener {
public:
    MainApp() {
        group = new Group;
        GuiManager::get()->add_widget(group,"mainapp");

        {
        Button *but = new Button(SpriteManager::get()->get_text("human vs human","font01",Text::CENTER),&human_human_callback);
        but->sprite->x = SdlManager::get()->width/2.;
        but->sprite->y = 100;
        but->data = this;
        group->add_widget(but,"hhbut");
        } {
        Button *but = new Button(SpriteManager::get()->get_text("human vs bot","font01",Text::CENTER),&human_bot_callback);
        but->sprite->x = SdlManager::get()->width/2.;
        but->sprite->y = 150;
        but->data = this;
        group->add_widget(but,"hbbut");
        } {
        Button *but = new Button(SpriteManager::get()->get_text("bot vs human","font01",Text::CENTER),&bot_human_callback);
        but->sprite->x = SdlManager::get()->width/2.;
        but->sprite->y = 200;
        but->data = this;
        group->add_widget(but,"bhbut");
        } {
        Button *but = new Button(SpriteManager::get()->get_text("bot vs bot","font01",Text::CENTER),&bot_bot_callback);
        but->sprite->x = SdlManager::get()->width/2.;
        but->sprite->y = 250;
        but->data = this;
        group->add_widget(but,"bbbut");
        }

        {
        Button *but = new ToggleButton("check",NULL,game.bot1god,"bot1 god mode");
        but->sprite->x = 50;
        but->sprite->y = 400;
        group->add_widget(but,"bot1god");
        } {
        Button *but = new ToggleButton("check",NULL,game.bot2god,"bot2 god mode");
        but->sprite->x = 50;
        but->sprite->y = 430;
        group->add_widget(but,"bot2god");
        }

    }
    void launch_game(bool p1h,bool p2h) {
        assert(not SdlManager::get()->is_listener_registered(&game));
        game.p1human = p1h;
        game.p2human = p2h;
        game.bot1god = static_cast<ToggleButton*>(group->get_widget("bot1god"))->state;
        game.bot2god = static_cast<ToggleButton*>(group->get_widget("bot2god"))->state;
        SdlManager::get()->register_listener(&game);
        GuiManager::get()->get_widget("mainapp")->enabled = false;
    }
protected:
    virtual bool key_down(SDLKey key) {
        switch (key) {
        case SDLK_f:
            SdlManager::get()->toggle_fullscreen();
            break;
        case SDLK_ESCAPE:
            cout<<"escape "<<SdlManager::get()->is_listener_registered(&game)<<endl;
            if (SdlManager::get()->is_listener_registered(&game)) {
                SdlManager::get()->unregister_listener(&game);
                GuiManager::get()->get_widget("mainapp")->enabled = true;
            } else return false;
            break;
        default:
            break;
        }
        return true;
    }
    virtual bool frame_entered(float t,float dt) {
        return true;
    }

    GameApp game;
    Group *group;
private:
    static void human_human_callback(Button *but) {
        MessageManager::get()->add_message("human vs human");
        static_cast<MainApp*>(but->data)->launch_game(true,true);
    }
    static void human_bot_callback(Button *but) {
        MessageManager::get()->add_message("human vs bot");
        static_cast<MainApp*>(but->data)->launch_game(true,false);
    }
    static void bot_human_callback(Button *but) {
        MessageManager::get()->add_message("bot vs human");
        static_cast<MainApp*>(but->data)->launch_game(false,true);
    }
    static void bot_bot_callback(Button *but) {
        MessageManager::get()->add_message("bot vs bot");
        static_cast<MainApp*>(but->data)->launch_game(false,false);
    }
};

int main() {
    try {
        srand(time(NULL));
        //cout.precision(2);

        SdlManager::init();
        SdlManager::get()->set_background_color(0,0,.05);

        SoundManager::init();
        if (not SoundManager::get()->load_directory("data"))
        if (not SoundManager::get()->load_directory("../data"))
        if (not SoundManager::get()->load_directory("../../data"))
        if (not SoundManager::get()->load_directory("/usr/share/supershooter/data"))
        if (not SoundManager::get()->load_directory("/usr/local/share/supershooter/data")) {
            cerr<<"can't locate sound data..."<<endl;
            return 1;
        }
        SoundManager::get()->dump();

        SpriteManager::init();
        if (not SpriteManager::get()->load_directory("data"))
        if (not SpriteManager::get()->load_directory("../data"))
        if (not SpriteManager::get()->load_directory("../../data"))
        if (not SpriteManager::get()->load_directory("/usr/share/supershooter/data"))
        if (not SpriteManager::get()->load_directory("/usr/local/share/supershooter/data")) {
            cerr<<"can't locate sprite data..."<<endl;
            return 1;
        }
        SpriteManager::get()->dump();

        MessageManager::init();
        SdlManager::get()->register_listener(MessageManager::get());

        GuiManager::init();
        SdlManager::get()->register_listener(GuiManager::get());
        GuiManager::get()->add_sound_widgets();

        {
            Fps fps;
            MainApp mainapp;

            //fps.set_display(true);
            //MessageManager::get()->set_display(true);

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

