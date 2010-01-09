Supershooter is an awesome SHMUP game with lots of bullets. It is still very young but it is already playable!!! I made it easily customisable: adding new ships and new sequences of enemies can be done througth editing an xml file and putting images in a directory. For details on how to do this please refer to correspondig [section](#adding-new-content).

# Installation

Supershooter is hosted at [github](https://github.com/elcerdo/supershooter/tree/master). You can get the bleeding edge version by cloning the public git reposity:

    git clone git://github.com/elcerdo/supershooter.git

Or get the lastest release [here](http://github.com/elcerdo/supershooter/downloads).

Supershooter depends on the following libraries/tools:

* [cmake](http://www.cmake.org/) used for the build system.
* [sdl](http://www.libsdl.org/) and opengl for graphics.
* [sdl_image](http://www.libsdl.org/projects/SDL_image/) for image loading.
* [sdl_mixer](http://www.libsdl.org/projects/SDL_mixer/) for sounds ans musics.
* [boost](http://www.boost.org/): only for regex (i am a lazy fat pig).
* [tinyxml](http://www.grinninglizard.com/tinyxml/) is included in the project and used for xml parsing.
* supershooter also makes heavy usage of the [stl](http://www.sgi.com/tech/stl/).

Once you have got all those dependencies. Build the project be typing the following command in the project root directory (where the CMakeLists.txt file lies):

    cmake . && make

If you use archlinux, supershooter is packaged in [aur](http://aur.archlinux.org/packages.php?ID=25439).

Hopefully you can now enjoy **supershooter**!!!
The game has only been tested on linux (arch and fedora mainly) but i didn't test it under windows because i think developpment under this "system" sucks dicks. Anyway if someone has the strength to do that let me know, i would be delighted.

The game is played only using your mouse and the escape key on your keyboard. Beat the crap out of the bad guys and challenge your friend to get the highest score!!!

# Adding new content

You can easily define new content for supershooter. You can define new ship designs and behaviors and then add them to a sequence in wich the ship appears during the game. All you have to do is add *png* images in the data directory following a conventionnal name and then edit the *config.xml* file which contains all the logic of the game.

Adding new sprites
------------------

Sprites must be saved in the *data* directory in 32bits (RGBA) png. Using the filename the engine will find if the sprite has states or is animated.
Here are the three kinds of sprite you can define:

-   static sprites.
    Static sprites are nothing more than an image. Filename must match *[name].png* where *name* is composed with letters and numbers. *name* will represents the sprite file in the configuration file.
-   state sprites.
    State sprites can more than one state. For example font sprites are state sprites. The filename of a state sprite must match *[name]-[nstate].png* where *name* is a unique identifier composed by letters and numbers and *nstate* is the number of state of the sprite. State sprite are build by stacking vertically the images of the different state.
-   animated sprite
-   Animated sprite can have different state, in which they are animated over different frame. Filenames must match *[name]-[nstate]x[nframe]* where *name* is an unique identifier, *nstate* is the number of state in the sprite and *nframe*. Animated sprites are built by statcking horizontally multiple state sprites.

Once you have added all images in the *data* directory you can start editing the configuration file.

Basic configuration file
------------------------

The configuration file *config.xml* is divided in two main section **ships** and **waves**. Here is a minimal configuration file:

    <config>
    	<ships>
    		<ship id="ship00" health="100">
    		...
    		</ship>
    		<ship id="ship01" health="100">
    		...
    		</ship>
    	</ships>
    	<waves>
    		<wave id="mainwave">
    		...
    		</wave>
    	</waves>
    </config>

A more complex example can be found [here](/config.xml) or in the program sources.
 
Adding new ship
---------------

To add a new ship to the game add a new **ship** section in the **ships** section of config.xml. Here are the definition of the tags used for defining ships:

ship
:   must contain one **sprite** and at least one **program**.

:   * health: the initial energy of the ship. *required*
    * id: must be unique over all ships, used in the wave definition. *required*
    * score: base point for killing this enemy. *required*

### Defining the look

sprite
:   can contain other **sprite**. This is where you can define the look of the ship.

:   * name: the parsed name of the image. *required*
    * id: must be unique in one ship definition, used in the programs of the ship.
    * state: initial state of the sprite, significant only is the sprite is a state sprite or an animated sprite.
    * speed: initial speed of the animation in fps, significant only if the sprite is an animated sprite.
    * repeat: must be 1 or 0, controls the repetition of the animation of an animated sprite.
    * length: number of frame played in the animation.
    * angle: initial orientation of the sprite relative to its parent if it exists.
    * x: initial x position of the sprite relative to its parent if it exists.
    * y: initial y position of the sprite relative to its parent if it exists.
    * z: controls the order in which sprites are draw, values must be between -10 to 10. High value will make the sprite appears over other sprites.
    * cx: x center of rotation in the sprite coordinate system.
    * cy: y center of rotation in the sprite coordinate system.


### Defining the behavior

program
:   can contain a sequence of **speed**, **position**, **positionrel**, **wait**, **shoot** or other **program**. Top level programs must define a ship-wide unique id that is used during wave definition. Programs are used to define the behavior of the ship.

:   * id: an ship-wise unique id. Ids of top level program are used in the wave definition.
    * repeat: a integer that control the child sequence is repeated. If repeat is null, then the program block is skipped. If repeat is strickly less that zero, the included sequence is repeated endlessly. This attribute is very powerfull for creating complex behavior.

wait
:   This block the parsing of the script for a certain lapse of time.

:   * time: number of seconds during which the program will be paused . The default value is 1s.

speed
:   sets the speed of the ship. If speed is set to a positive value, the ship will move forward in the direction of its current orientation. If the value is negative, then the ship will move backward.

:   * value: speed of the ship in px/s. The default value is 0, which makes the ship stops.

shoot
:   shoots a bullet using a sprite orientation and position.

:   * id: any valid sprite id. This selects the sprite from which the bullet is fired. If no id is given, the engine will use main (top most) sprite definition od the ship.
    * name: the name of the sprite used to draw the bullet. Default is "bullet00".
    * anglerel: relative angle refering to the orientation of the sprite in which the bullet is fired.
    * speed: the speed of the bullet in px/sec.
    * sprspeed: the speed of the animation of the bullet, revelant only if the sprite used by the bullet in an animated one.
    * sprrepeat: controls the repetition of the animation of the bullet's sprite.
    * sprlength: length of the animation of the sprite used by the bullet.

position
:   set the position and/or the orientation of the ship or one of its sprite.

:   * id: any valid sprite id. If no id is given, the engine will use main (top most) sprite definition od the ship.
    * x: new x position. If no value is given the x position of the ship or sprite isn't modified.
    * y: new y position. If no value is given the y position of the ship or sprite isn't modified.
    * angle: new orientation. If no value is given the the orientation of the ship or sprite isn't modified.

positionrel
:   set the position and/or the orientation of the ship or one of its sprite relatively to its current position.

:   * id: any valid sprite id. If no id is given, the engine will use main (top most) sprite definition od the ship.
    * x: the x position of the ship or sprite will be incremented by that value.
    * y: the y position of the ship or sprite will be incremented by that value.
    * angle: the orientation of the ship or sprite will be incremented by that value. Positive values will rotate the ship clockwise.

### Sample ship definition

~~~{.xml .numberLines}
<ship id="basicship" health="10">
	<sprite name="ship00" z="2."/>
	<program id="right">
		<speed value="200."/>
		<wait time=".8"/>
		<program repeat="3">
			<wait time=".4"/>
			<shoot speed="400" name="bullet02"/>
			<positionrel angle="-20"/>
		</program>
	</program>
	<program id="left">
		<speed value="200."/>
		<wait time=".8"/>
		<program repeat="3">
			<wait time=".4"/>
			<shoot speed="400" name="bullet02"/>
			<positionrel angle="20"/>
		</program>
	</program>
</ship>
~~~


Adding new sequence
-------------------

To add new sequence add a **wave** section under the **waves** tag in the configuration file. Here are the definition of tags used for defining new sequences:

wave
:   This is the core tag used for defining sequence. It behaves quite as the **program** tag. It can be composed of a sequence of **line**, **spawn**, **wait** and **wave**.

:   * id: unique id representing the wave. It must be unique over all waves. Top level waves must define an id.
    * repeat: a integer that control the child sequence is repeated. If repeat is null, then the wave block is skipped. If repeat is strickly less that zero, the included sequence is repeated endlessly. This attribute is very powerfull for creating complex sequence.

wait
:   This block the parsing of the script for a certain lapse of time.

:   * time: number of seconds during which the program will be paused . The default value is 1s.

spawn
:   create a enemy ship at a given position with a given behavior.

:   * id: id of the ship to be created as defined in the **ship** tag. *required*
    * program: id of ship's program as defined in the corresponding **program** tag. *required*
    * x: x position of the ship. Defined as a ratio of the effective position and the width of the window. *x=0* means the left side of the screen, *x=1* means the right of the screen. Can be negative or superior to 1. Default is 0.5.
    * y: y position of the ship. Defined as a ratio of the effective position and the height of the window. *y=0* means the top side of the screen, *y=1* means the bottom of the screen. Can be negative or superior to 1. Default is -0.1.
    * angle: orientation of the ship. 0 means left, 90 bottom, 180 right and -90 top. Default is 90.

line
:   create a line of enemies.
:   * id: id of the ships to be created as defined in the **ship** tag. *required*
    * program: id of ships program as defined in the corresponding **program** tag. *required*
    * startx: start x position of the line. Defined as a ratio of the effective position and the width of the window. *x=0* means the left side of the screen, *x=1* means the right of the screen. Can be negative or superior to 1.
    * starty: start y position of the line. Defined as a ratio of the effective position and the height of the window. *y=0* means the top side of the screen, *y=1* means the bottom of the screen. Can be negative or superior to 1.
    * endx: end x position of line.
    * endy: end y position of line.
    * n: number of ships. Default is 10.
    * angle: orientation of the ship. 0 means left, 90 bottom, 180 right and -90 top. Default is 90.

### Sample sequence

~~~{.xml .numberLines}
<wave id="mainwave" repeat="10">
	<line  id="basicship" program="left" startx="0.2" starty="-.2" endx="0.5" endy="-0.1" n="5" angle="90"/>
	<line  id="basicship" program="right" startx="0.5" starty="-.1" endx="0.8" endy="-0.2" n="5" angle="90"/>
	<wait  time="2."/>
	<wave repeat="2">
		<spawn id="bigship" program="main" x="0.45" angle="95"/>
		<spawn id="bigship" program="main" x="0.55" angle="85"/>
		<wait  time=".5"/>
	</wave>
	<spawn id="bigship" program="main" x="0.35" y="-0.1" angle="95"/>
	<spawn id="bigship" program="main" x="0.65" y="-0.1" angle="85"/>
	<wait  time="2."/>
	<spawn id="bigship" program="main" x="0.25" y="-0.2" angle="95"/>
	<spawn id="bigship" program="main" x="0.75" y="-0.2" angle="85"/>
	<wait  time="2."/>
	<spawn id="crotte" program="main"/>
	<wait/>
</wave>
~~~

