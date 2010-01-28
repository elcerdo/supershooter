#include "message.h"
#include "except.h"

static MessageManager *mMessageManager=NULL;

MessageManager *MessageManager::get() { return mMessageManager; }
void MessageManager::free() { if (mMessageManager) { delete mMessageManager; mMessageManager=NULL; } }
void MessageManager::init(size_t nmessage,size_t nfade) {
    if (mMessageManager) throw Except(Except::ZIZI_INIT_ERR,"messagemanager already exists");
    mMessageManager=new MessageManager(nmessage,nfade);
}

MessageManager::MessageManager(size_t nmessage,size_t nfade) : nmessage(nmessage), nplain(nmessage-nfade-1), maxalpha(0.7), display(false) {}
MessageManager::~MessageManager() { unregister_self(); }

void MessageManager::add_message(const std::string &message) {
    //std::cout<<"MESSAGE: "<<message<<std::endl;
    Text *text=SpriteManager::get()->get_text(message,"fonttiny",Text::RIGHT);
    text->x=SdlManager::get()->width-8;
    text->alpha=maxalpha;
    text->factorx=.5;
    text->factory=.5;
    text->update_alpha();
    texts.push_front(text);
}

void MessageManager::set_display(bool disp) {
    display = disp;
}

bool MessageManager::key_down(SDLKey key) { if (key==SDLK_d) display=not display; return true; }

bool MessageManager::frame_entered(float t,float dt) {
    while (texts.size()>nmessage) { delete texts.back(); texts.pop_back(); }

    float y=SdlManager::get()->height-8;
    //float y=8;
    size_t count=0;
    for (Texts::const_iterator i=texts.begin(); i!=texts.end() and display; i++) {
        (*i)->y=y;
        if (count>=nplain) { (*i)->alpha=maxalpha*(1.-static_cast<float>(count-nplain)/(nmessage-nplain)); (*i)->update_alpha(); }
        (*i)->draw(dt);
        count++;
        y-=(*i)->h*(*i)->factory + 2;
        //y+=16;
    }
    return true;
}

void MessageManager::unregister_self() { while (not texts.empty()) { delete texts.back(); texts.pop_back(); } }

