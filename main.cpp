#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <iostream>
#include <cassert>
#include <map>
#include <utility>
#include <string>
using std::endl;
using std::cout;
using std::cerr;

static SDL_Surface *screen=NULL;

class Listener {
public:
    Listener() : quit(false) {}
    void update() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym==SDLK_ESCAPE) quit=true; //quit game
                if (event.key.keysym.sym==SDLK_SPACE) quit=true;
                break;
            case SDL_QUIT:      
                quit=true;
                break;
            default:
                break;
            }
        }
    }
    bool quit;
protected:
};

struct Except {
    enum ExceptType {
        SS_INIT_ERR,
        SS_LOADING_ERR,
        SS_TOO_MANY_SPRITES_ERR,
        SS_CONVERSION_ERR,
    };
    Except(ExceptType n) : n(n) {};
    ExceptType n;
};


void init_sdl(int w=800,int h=600,int d=32) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_OPENGL)) { cerr<<"cannot initialize sdl..."<<endl; throw Except(Except::SS_INIT_ERR); }

    screen=SDL_SetVideoMode(800,600,32,SDL_OPENGL|SDL_DOUBLEBUF);
    if (not screen) { cerr<<"cannot create sdl screen..."<<endl; throw Except(Except::SS_INIT_ERR); }

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(1,0,0,1);
	glClearDepth(1);// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);// The Type Of Depth Testing To Do
	glEnable(GL_BLEND);//Alpha blending
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// Really Nice Perspective Calculations

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,w,h,0,1.0,-1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

class Sprite {
public:
    Sprite(unsigned int id,double w,double h) : id(id), x(0), y(0), angle(0), w(w), h(h) {}
    void draw() const {}
    float x,y,angle;
protected:
    unsigned int id;
    float w,h;
};


class SpriteManager {
public:
    SpriteManager(size_t maxid=256) : maxid(maxid) {
        ids=new unsigned int[maxid];
        currentid=0;
        glGenTextures(maxid,ids);
    }
    void load_image(const std::string &filename) {
        if (currentid>=maxid-1) throw Except(Except::SS_TOO_MANY_SPRITES_ERR);

        SDL_Surface *surf=IMG_Load(filename.c_str());
        if (not surf) { cerr<<"error loading '"<<filename<<"': "<<IMG_GetError()<<endl; throw Except(Except::SS_LOADING_ERR); }
        cout<<"loaded '"<<filename<<"' "<<surf->w<<"x"<<surf->h<<endl;

        if (surf->format->BitsPerPixel!=32) { SDL_FreeSurface(surf); throw Except(Except::SS_CONVERSION_ERR); }
        glBindTexture(GL_TEXTURE_2D,currentid);
        glTexImage2D(GL_TEXTURE_2D,0,4,surf->w,surf->h,0,GL_RGBA,GL_UNSIGNED_BYTE,static_cast<unsigned char*>(surf->pixels));
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

        idmap[filename]=std::make_pair(ids[currentid],surf);
        currentid++;
    }
    Sprite *get_sprite(const std::string &name) {
        std::pair<unsigned int,SDL_Surface*> match=idmap[name];
        return new Sprite(match.first,match.second->w,match.second->h);
    }
    void dump() const {
        cout<<currentid<<"/"<<maxid<<" sprites"<<endl;
        for (IdMap::const_iterator i=idmap.begin(); i!=idmap.end(); i++) cout<<i->first<<" "<<i->second.first<<" "<<i->second.second->w<<"x"<<i->second.second->h<<endl;
    }
    ~SpriteManager() {
        delete [] ids;
        for (IdMap::const_iterator i=idmap.begin(); i!=idmap.end(); i++) SDL_FreeSurface(i->second.second);
    }
protected:
    typedef std::map<std::string,std::pair<unsigned int,SDL_Surface*> > IdMap;
    unsigned int *ids;
    size_t currentid,maxid;
    IdMap idmap;
};

    
int main() {
    init_sdl();

    SpriteManager sprites;
    sprites.load_image("logo.png");
    Sprite *aa=sprites.get_sprite("logo.png");

    sprites.dump();

    Listener listener;
    while (not listener.quit) {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        SDL_GL_SwapBuffers();  
        SDL_Flip(screen);

        listener.update();

        SDL_Delay(10);
    }

    delete aa;
    SDL_FreeSurface(screen);
    SDL_Quit();
}


