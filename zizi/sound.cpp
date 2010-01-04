#include "sound.h"

#include "except.h"
#include <boost/regex.hpp>
#include <dirent.h>
#include <vector>

Sfx::Sfx(Mix_Chunk *chunk,const bool &play_sfxs,const std::string &name) : chunk(chunk), play_sfxs(play_sfxs), name(name), channel(-1) {}
void Sfx::play_once() const { if (play_sfxs) Mix_PlayChannel(-1,chunk,0); }
void Sfx::play_start() { if (play_sfxs and channel==-1) channel=Mix_PlayChannel(-1,chunk,-1); }
void Sfx::play_stop() { if (channel!=-1) { channel=-1; Mix_HaltChannel(channel); } }

//***********************************************************
static SoundManager *mSoundManager=NULL;

SoundManager *SoundManager::get() { return mSoundManager; }
void SoundManager::free() { if (mSoundManager) { delete mSoundManager; mSoundManager=NULL; } }
void SoundManager::init(int nchannel) {
    if (mSoundManager) throw Except(Except::ZIZI_INIT_ERR,"sound manager already exists");
    mSoundManager=new SoundManager(nchannel);
}

void SoundManager::hook_musics_finished() {
    mSoundManager->playing=false;
    if (mSoundManager->play_musics_continuious) mSoundManager->play_random_music();
}

SoundManager::SoundManager(int nchannel) : playing(false), play_musics(true), play_sfxs(true), play_musics_continuious(false) {
    if (Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,nchannel,1024)) throw Except(Except::ZIZI_INIT_ERR,"cannot initialize sound");
    Mix_HookMusicFinished(SoundManager::hook_musics_finished);
}

SoundManager::~SoundManager() {
    Mix_HaltMusic();
    for (Musics::const_iterator i=musics.begin(); i!=musics.end(); i++) Mix_FreeMusic(i->second);
    for (Chunks::const_iterator i=chunks.begin(); i!=chunks.end(); i++) Mix_FreeChunk(i->second);
    Mix_CloseAudio();
}

void SoundManager::dump(std::ostream &os) const {
    //os<<"play_musics="<<play_musics<<" play_musics_continuious="<<play_musics_continuious<<" playing="<<playing<<std::endl;
    os<<musics.size()<<" musics, "<<chunks.size()<<" sfxs:"<<std::endl;
    for (Musics::const_iterator i=musics.begin(); i!=musics.end(); i++) {
        os<<"* music "<<i->first<<" ";
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
    for (Chunks::const_iterator i=chunks.begin(); i!=chunks.end(); i++) os<<"* sfx "<<i->first<<std::endl; 
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
    if (i==musics.end()) throw Except(Except::ZIZI_SOUND_UNKNOWN_ERR,id);

    //cout<<"playing "<<i->first<<std::endl;
    Mix_PlayMusic(i->second,1);
    playing=true;
}

Sfx *SoundManager::get_sfx(const std::string &id) {
    Chunks::const_iterator i=chunks.find(id);
    if (i==chunks.end()) throw Except(Except::ZIZI_SOUND_UNKNOWN_ERR,id);

    Sfx *sfx=new Sfx(i->second,play_sfxs,i->first);
    return sfx;
}
    
bool SoundManager::toggle_music() {
    play_musics=not play_musics;
    if (not play_musics and playing) {
        Mix_HaltMusic();
        playing=false;
    }
    if (play_musics and play_musics_continuious) play_random_music();
    return play_musics;
}

bool SoundManager::is_playing_music() const {
    return play_musics;
}

bool SoundManager::toggle_sfx() {
    play_sfxs=not play_sfxs;
    return play_sfxs;
}

bool SoundManager::is_playing_sfx() const {
    return play_sfxs;
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

        try { load_sound(prefix+filename);
        } catch (Except &e) {
            if (e.n!=Except::ZIZI_SOUND_LOADING_ERR) throw e;
            else e.dump();
        }
    }

    closedir(dir);
    return true;
}

void SoundManager::load_sound(const std::string &filename) {
    static const boost::regex e("(\\A|\\A.*/)(\\w+)\\-(music|sfx).(ogg|mp3)\\Z");
    boost::smatch what;
    if (not regex_match(filename,what,e)) throw Except(Except::ZIZI_SOUND_LOADING_ERR,filename);

    if (what[3]=="music") {
        if (musics.find(what[2])!=musics.end()) throw Except(Except::ZIZI_SOUND_DUPLICATE_ERR,what[2]);

        Mix_Music *music=Mix_LoadMUS(filename.c_str());
        if (not music) throw Except(Except::ZIZI_SOUND_LOADING_ERR,filename);

        musics[what[2]]=music;
    }

    if (what[3]=="sfx") {
        if (chunks.find(what[2])!=chunks.end()) throw Except(Except::ZIZI_SOUND_DUPLICATE_ERR,what[2]);

        Mix_Chunk *chunk=Mix_LoadWAV(filename.c_str());
        if (not chunk) throw Except(Except::ZIZI_SOUND_LOADING_ERR,filename);

        chunks[what[2]]=chunk;
    }
}
