#include "engine.h"

#include <SDL/SDL.h>
#include <iostream>
#include <GL/gl.h>
#include "except.h"

static SdlManager *mSdlManager=NULL;

SdlManager *SdlManager::get() { return mSdlManager; }
void SdlManager::free() { if (mSdlManager) { delete mSdlManager; mSdlManager=NULL; } }
void SdlManager::init(int w,int h,int d, float x, float y) {
    if (mSdlManager) throw Except(Except::ZIZI_INIT_ERR,"sdlmanager already exists");
    mSdlManager=new SdlManager(w,h,d,x,y);
}

SdlManager::SdlManager(int w,int h,int d, float x, float y) : in_main_loop(false), width(w), height(h), old_ticks(0), x(x), y(y) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER|SDL_OPENGL)) throw Except(Except::ZIZI_INIT_ERR,"cannot initialize sdl");

    screen=SDL_SetVideoMode(width,height,d,SDL_OPENGL|SDL_DOUBLEBUF);
    if (not screen) throw Except(Except::ZIZI_INIT_ERR,"cannot create sdl screen");

    SDL_SetEventFilter(&event_filter);
    SDL_ShowCursor(false);

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0,0,0,0);
	glEnable(GL_DEPTH_TEST);// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);// The Type Of Depth Testing To Do
	glEnable(GL_BLEND);//Alpha blending
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// Really Nice Perspective Calculations
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.05);

    update_projection();
}

void SdlManager::update_projection() const {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x,x+width,y+height,y,-10.0,10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void SdlManager::set_screen_center(float cx,float cy) {
    const_cast<float&>(x) = cx-width/2.;
    const_cast<float&>(y) = cy-height/2.;
    update_projection();
}

void SdlManager::toggle_fullscreen() const { SDL_WM_ToggleFullScreen(screen); }
const unsigned char *SdlManager::get_key_state() const { return SDL_GetKeyState(NULL); }
void SdlManager::get_overlay_mouse_position(float &rx,float &ry) const {
    int xx,yy;
    SDL_GetMouseState(&xx,&yy);
    rx=xx; ry=yy;
}

void SdlManager::get_mouse_position(float &xx,float &yy) const {
    get_overlay_mouse_position(xx,yy);
    xx += x;
    yy += y;
}

void SdlManager::set_background_color(float r,float g,float b) { glClearColor(r,g,b,0); }

SdlManager::~SdlManager() {
    SDL_FreeSurface(screen);
    SDL_Quit();
}

void SdlManager::register_listener(Listener *listener) { //FIXME duplicate
    if (in_main_loop) listener->register_self();
    listeners.insert(listener);
}

void SdlManager::unregister_listener(Listener *listener) { //FIXME existance
    listeners.erase(listener);
    if (in_main_loop) listener->unregister_self();
}

int SdlManager::event_filter(const SDL_Event *ev) {
    if (ev->type==SDL_MOUSEMOTION or ev->type==SDL_ACTIVEEVENT or ev->type==SDL_VIDEOEXPOSE) return 0;
    return 1;
}

void SdlManager::main_loop() {
    in_main_loop=true;

    for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end(); i++) (*i)->register_self();

    old_ticks=SDL_GetTicks();
    bool quit=false;
    while (not quit and not listeners.empty()) {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        SDL_Event event;
        while (SDL_PollEvent(&event) and not quit) {
            switch (event.type) {
            case SDL_KEYDOWN:
                for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end() and not quit; i++) quit=not (*i)->key_down(event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end() and not quit; i++) quit=not (*i)->key_up(event.key.keysym.sym);
                break;
            case SDL_MOUSEBUTTONDOWN:
                for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end() and not quit; i++) quit=not (*i)->mouse_down(event.button.button,event.button.x+x,event.button.y+y);
                break;
            case SDL_MOUSEBUTTONUP:
                for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end() and not quit; i++) quit=not (*i)->mouse_up(event.button.button,event.button.x+x,event.button.y+y);
                break;
            case SDL_QUIT:      
                quit=true;
                break;
            default:
                std::cerr<<"unhandled event "<<static_cast<int>(event.type)<<std::endl;
                break;
            }
        }

        long int ticks=SDL_GetTicks();
        float t=ticks/1000.;
        float dt=(ticks-old_ticks)/1000.;
        old_ticks=ticks;
        for (Listeners::const_iterator ii=listeners.begin(); ii!=listeners.end() and not quit;) {
            Listeners::const_iterator i=ii++;
            quit=not (*i)->frame_entered(t,dt);
        }

        SDL_GL_SwapBuffers();  
        SDL_Flip(screen);

        SDL_Delay(5);
    }

    for (Listeners::const_iterator i=listeners.begin(); i!=listeners.end(); i++) (*i)->unregister_self();

    in_main_loop=false;
}

