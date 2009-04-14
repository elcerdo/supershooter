#ifndef __SOUND_H__
#define __SOUND_H__

#include <string>
#include <map>
#include <iostream>
#include <SDL/SDL_mixer.h>

class Sfx {
friend class SoundManager;
public:
    void play() const;
protected:
    Sfx(Mix_Chunk *chunk,const bool &play_sfxs,const std::string &name);
    Mix_Chunk *chunk;
    const bool &play_sfxs;
    std::string name;
};

//***********************************************************
class SoundManager {
public:
    static SoundManager *get();
    static void free();
    static void init(int nchannel=2);

    void dump(std::ostream &os=std::cout) const;
    bool load_directory(const std::string &directory);
    void load_sound(const std::string &filename);
    void play_music(const std::string &id);
    Sfx *get_sfx(const std::string &id);
    void play_random_music();
    bool toogle_musics();
    bool toogle_sfxs();
    bool play_musics_continuious;
protected:
    static void hook_musics_finished();
    bool playing;
    bool play_musics;
    bool play_sfxs;
    SoundManager(int nchannel);
    ~SoundManager();

    typedef std::map<std::string,Mix_Music*> Musics;
    Musics musics;
    typedef std::map<std::string,Mix_Chunk*> Chunks;
    Chunks chunks;
};

#endif
