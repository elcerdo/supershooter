#include "sound.h"

#include "except.h"
#include <boost/regex.hpp>
#include <dirent.h>
#include <vector>

static SoundManager *mSoundManager=NULL;

SoundManager *SoundManager::get() { return mSoundManager; }
void SoundManager::free() { if (mSoundManager) { delete mSoundManager; mSoundManager=NULL; } }
void SoundManager::init(int nchannel) {
    if (mSoundManager) throw Except(Except::SS_INIT_ERR);
    mSoundManager=new SoundManager(nchannel);
}

void SoundManager::hook_musics_finished() {
    mSoundManager->playing=false;
    if (mSoundManager->play_musics_continuious) mSoundManager->play_random_music();
}

SoundManager::SoundManager(int nchannel) : playing(false), play_musics(true), play_musics_continuious(false) {
    if (Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,nchannel,1024)) {
        std::cerr<<"cannot initialize sound..."<<std::endl;
        throw Except(Except::SS_INIT_ERR);
    }
    Mix_HookMusicFinished(SoundManager::hook_musics_finished);
}

SoundManager::~SoundManager() {
    Mix_HaltMusic();
    for (Musics::const_iterator i=musics.begin(); i!=musics.end(); i++) Mix_FreeMusic(i->second);
    Mix_CloseAudio();
}

void SoundManager::dump(std::ostream &os) const {
    //os<<"play_musics="<<play_musics<<" play_musics_continuious="<<play_musics_continuious<<" playing="<<playing<<std::endl;
    os<<musics.size()<<" musics:"<<std::endl;
    for (Musics::const_iterator i=musics.begin(); i!=musics.end(); i++) {
        os<<"* "<<i->first<<" ";
        switch(Mix_GetMusicType(i->second)) {
            case MUS_NONE:
            case MUS_CMD:
                os<<"command"; break;
            case MUS_WAV:
                os<<"wav"; break;
            case MUS_MOD:
                os<<"mod"; break;
            case MUS_MID:
                os<<"midi"; break;
            case MUS_OGG:
                os<<"ogg"; break;
            case MUS_MP3:
                os<<"mp3"; break;
            default:
                os<<"unknown"; break;
        }
        os<<std::endl;
    }
}

void SoundManager::play_random_music() {
    typedef std::vector<std::string> Ids;
    Ids ids;
    for (Musics::const_iterator i=musics.begin(); i!=musics.end(); i++) ids.push_back(i->first);
    play_music(ids[rand()%ids.size()]);
}

void SoundManager::play_music(const std::string &id) {
    if (not play_musics) return;

    Musics::const_iterator i=musics.find(id);
    if (i==musics.end()) throw Except(Except::SS_MUSIC_UNKNOWN_ERR,id);

    //cout<<"playing "<<i->first<<std::endl;
    Mix_PlayMusic(i->second,1);
    playing=true;
}
    
bool SoundManager::toogle_musics() {
    play_musics=not play_musics;
    if (not play_musics and playing) {
        Mix_HaltMusic();
        playing=false;
    }
    if (play_musics and play_musics_continuious) play_random_music();
    return play_musics;
}

bool SoundManager::load_directory(const std::string &directory) {
    DIR *dir=opendir(directory.c_str());
    if (not dir) return false;

    std::string prefix(directory);
    if (prefix.size()>0 and prefix[prefix.size()-1]!='/') prefix+='/';

    dirent *ent;
    while (ent=readdir(dir)) {
        std::string filename(ent->d_name);
        if (filename=="." or filename=="..") continue;

        try { load_music(prefix+filename);
        } catch (Except &e) {
            if (e.n!=Except::SS_MUSIC_LOADING_ERR) throw Except(e.n);
        }
    }

    closedir(dir);
    return true;
}

void SoundManager::load_music(const std::string &filename) {
    static const boost::regex e("(\\A|\\A.*/)(\\w+)\\.(ogg|mp3)\\Z");
    boost::smatch what;
    if (not regex_match(filename,what,e)) throw Except(Except::SS_MUSIC_LOADING_ERR);
    if (musics.find(what[2])!=musics.end()) throw Except(Except::SS_MUSIC_DUPLICATE_ERR);

    Mix_Music *music=Mix_LoadMUS(filename.c_str());
    if (not music) throw Except(Except::SS_MUSIC_LOADING_ERR);

    musics[what[2]]=music;
}
