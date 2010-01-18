#include "engine.h"
#include "except.h"
#include "utils.h"
#include "sound.h"
#include "gui.h"
using std::cout;

void doitnow(Button *but) {
    printf("clicked...\n");
    but->sprite->dump();
}
void toggle_testb(Button *but) {
    Widget *testb = but->get_root_group()->get_widget("groupa");
    if (!testb) return;
    testb->enabled = not testb->enabled;
}

void toggle_music_callback(Button *but) {
    ToggleButton *casted = static_cast<ToggleButton*>(but);
    SoundManager::get()->set_playing_music(casted->state);
}

void toggle_sfx_callback(Button *but) {
    ToggleButton *casted = static_cast<ToggleButton*>(but);
    SoundManager::get()->set_playing_sfx(casted->state);
}

int main() {
    try {
        SdlManager::init();
        SpriteManager::init();
        SoundManager::init();
        GuiManager::init();

        SdlManager::get()->set_background_color(1,.3,.3);
        SpriteManager::get()->load_directory("data");
        SpriteManager::get()->load_directory("../data");
        SpriteManager::get()->load_directory("../../data");
        SpriteManager::get()->dump(cout);

        SoundManager::get()->load_directory("data");
        SoundManager::get()->load_directory("../data");
        SoundManager::get()->load_directory("../../data");
        SoundManager::get()->play_musics_continuious=true;
        SoundManager::get()->dump(cout);

        {
        Button *testa = new Button("logo",toggle_testb);
        testa->sprite->x = 250;
        testa->sprite->y = 100;
        GuiManager::get()->add_widget(testa,"testa");

        Group *group = new Group();
        GuiManager::get()->add_widget(group,"groupa");

        Button *testb = new Button("bullet02",doitnow);
        group->add_widget(testb,"testb");
        testb->sprite->x = 260;
        testb->sprite->y = 200;
        Button *testc = new ToggleButton("check",doitnow);
        group->add_widget(testc,"testc");
        testc->sprite->x = 260;
        testc->sprite->y = 230;
        Button *testd = new ToggleButton("check",doitnow,false,SpriteManager::get()->get_text("prout","font03",Text::LEFT));
        group->add_widget(testd,"testd");
        testd->sprite->x = 260;
        testd->sprite->y = 260;

        {
            Group *sound_group = new Group();
            GuiManager::get()->add_widget(sound_group,"sound");
            Button *music_button = new ToggleButton("togglemusic",toggle_music_callback,SoundManager::get()->is_playing_music());
            music_button->sprite->x = SdlManager::get()->width - 20;
            music_button->sprite->y = 20;
            sound_group->add_widget(music_button,"music");
            Button *sfx_button = new ToggleButton("togglesfx",toggle_sfx_callback,SoundManager::get()->is_playing_sfx());
            sfx_button->sprite->x = music_button->sprite->x - 32;
            sfx_button->sprite->y = music_button->sprite->y;
            sound_group->add_widget(sfx_button,"sfx");
        }

        Fps fps;
        Killer killer;

        SdlManager::get()->register_listener(&killer);
        SdlManager::get()->register_listener(&fps);
        SdlManager::get()->register_listener(GuiManager::get());
        SdlManager::get()->main_loop();
        }

        GuiManager::free();
        SoundManager::free();
        SdlManager::free();
        SpriteManager::free();
    } catch (Except e) {
        e.dump();
    }
}


