#ifndef __SOUND_H__
#define __SOUND_H__

#include <string>
#include <map>
#include <iostream>
#include <SDL/SDL_mixer.h>

class Sfx {
friend class SoundManager;
public:
    void play_once() const;
    void play_start();
    void play_stop();
protected:
    Sfx(Mix_Chunk *chunk,const bool &play_sfxs,const std::string &name);
    Mix_Chunk *chunk;
    const bool &play_sfxs;
    std::string name;
    int channel;
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
    bool toggle_music();
    bool set_playing_music(bool);
    bool is_playing_music() const;
    bool toggle_sfx();
    bool set_playing_sfx(bool);
    bool is_playing_sfx() const;
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
