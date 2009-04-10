#ifndef __SOUND_H__
#define __SOUND_H__

#include <string>
#include <map>
#include <iostream>
#include <SDL/SDL_mixer.h>

class SoundManager {
public:
    static SoundManager *get();
    static void free();
    static void init(int nchannel=2);

    void dump(std::ostream &os=std::cout) const;
    bool load_directory(const std::string &directory);
    void load_music(const std::string &filename);
    void play_music(const std::string &id);
    void play_random_music();
    bool toogle_musics();
    bool play_musics_continuious;
protected:
    static void hook_musics_finished();
    bool playing;
    bool play_musics;
    SoundManager(int nchannel);
    ~SoundManager();

    typedef std::map<std::string,Mix_Music*> Musics;
    Musics musics;
};

#endif
