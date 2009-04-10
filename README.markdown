Supershooter is an awesome SHMUP game with lots of bullets. It is still very young but it is already playable!!! I made it easily customisable: adding new ships and new sequences of enemies can be done througth editing an xml file and putting images in a directory.

# Installation

Build from sources
------------------

Supershooter is hosted at [github](https://github.com/elcerdo/supershooter/tree/master). You can get the bleeding edge version by cloning the public git reposity:

    git clone git://github.com/elcerdo/supershooter.git

Or get the lastest release [here](http://github.com/elcerdo/supershooter/downloads).

Supershooter depends on the following libraries/tools:

* [cmake](http://www.cmake.org/) used for the build system.
* [sdl](http://www.libsdl.org/) and opengl for graphics.
* [sdl_image](http://www.libsdl.org/projects/SDL_image/) for image loading.
* [sdl_mixer](http://www.libsdl.org/projects/SDL_mixer/) for sfx and music.
* [boost](http://www.boost.org/) only for regex (i am a lazy fat pig).
* [tinyxml](http://www.grinninglizard.com/tinyxml/) is included in the project and used for xml parsing.
* supershooter also makes heavy usage of the [stl](http://www.sgi.com/tech/stl/).

Once you have got all those dependencies. Build the project be typing the following command in the project root directory (where the CMakeLists.txt file lies):

    cmake . && make

Using archlinux PKGBUILD
------------------------

You can use the good old AUR procedure:

    yaourt supershooter

Or use the PKGBUILD file in the [github](https://github.com/elcerdo/supershooter/tree/master) repository.

    git clone git://github.com/elcerdo/supershooter.git
    cd supershooter
    makepkg
    sudo pacman -U supershoter-...

# Play

Hopefully you can now enjoy **supershooter**!!!
The game has only been tested on linux (arch and fedora mainly) but i didn't test it under windows because i think developpment under this "system" sucks dicks. Anyway if someone has the strength to do that let me know, i would be delighted.

The game is played only using your mouse and the escape key on your keyboard. Beat the crap out of the bad guys and challenge your friend to get the highest score!!!

For screenshots and tweaking information visit [http://sd-12155.dedibox.fr:5001/SuperShooter](http://sd-12155.dedibox.fr:5001/SuperShooter).
