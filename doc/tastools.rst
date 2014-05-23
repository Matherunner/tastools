.. highlight:: none

TasTools and utilities
======================

TasTools is a mod written for the latest version of Half-Life.  TasTools on its own is hardly useful, there are several small utilities that enhance one's productivity in TAS creation.


New commands and cvars
----------------------

TasTools mod added several new TAS-related commands and some cvars for testing purposes.  Notice that some of the commands accept a ``COUNT`` argument.  For these commands, they perform their respective actions for at most ``COUNT`` times.  For instance, by specifying ``tas_dtap 5`` the player will only ducktap 5 times.  To perform an action for a practically infinite number of times, specify ``-1`` for ``COUNT``.  To disable a command immediately, specify ``0`` for ``COUNT``.

``+/-linestrafe``
  Perform line strafing.
``+/-backpedal``
  Perform braking by backpedalling.
``+/-leftstrafe``
  Perform strafing to the left.
``+/-rightstrafe``
  Perform strafing to the right.
``tas_cjmp COUNT``
  Jump whenever the player lands on the ground.
``tas_dtap COUNT``
  Ducktap whenever the player lands on the ground.  If ``tas_cjmp`` is also active, then ``tas_dtap`` will be taken precedence.
``tas_dwj COUNT``
  Press the duck key whenever the player jumps, then release it after one frame.  This can be useful for longjumping and DWJ jumping style (sometimes in conjunction with ``tas_db4l``).
``tas_lgagst COUNT``
  Leave the ground by ducktapping or jumping, depending on where tas_cjmp or tas_dtap is active at the moment, when the acceleration of airstrafe exceeds that of groundstrafe.  ``cl_mtype 1`` is required for this to work.  "lgagst" stands for "leave ground at air-ground speed threshold".
``tas_jb COUNT``
  Perform a jumpbug before landing to avoid fall damage.  This command has the highest priority over all other autoaction commands.
``tas_db4c COUNT``
  Duck to avoid collision with non ground planes.  If collision still happens after ducking, then it will not duck unless ``+duck`` is active.  "db4c" stands for "duck before collision".
``tas_db4l COUNT``
  Duck to avoid landing on ground planes, unless the player has ducked anyway.  "db4l" stands for "duck before landing".
``tas_yaw YAW``
  Set the yaw angle to ``YAW`` immediately.  If the player is line strafing, then this command overrides the DLS instead.
``tas_pitch PITCH``
  Set the pitch angle to ``PITCH`` immediately.
``tas_olsshift DIST``
  Shift the OLS perpendicularly to the right by ``DIST`` units.  If ``DIST`` is negative, then shift to the left instead.  This is mainly used to make the player line strafe with approximately ``DIST`` amplitude.
``tas_sba ANGLE``
  Perform left or right strafing automatically until the velocity polar angle has changed by ``ANGLE`` degrees.  Note that either ``+leftstrafe`` or ``+rightstrafe`` must be active for this to work, and it requires a ``wait`` followed by ``exec waitscript.cfg`` to work as intended.  It relies on the ability to create ``waitscript.cfg`` (called the *waitscript*) in the mod directory.  This is a rather special command as it prevents further execution of the script that invokes it until the condition is met, accomplished by writing ``wait; exec waitscript.cfg`` to waitscript so that the waitscript executes itself forever.  Once the condition is fulfilled, TasTools will stop the loop by clearing the content of waitscript, allowing the main script to resume execution.
``ch_health HEALTH``
  Change the health amount to ``HEALTH``.  This is a cheat and should be used for testing purposes only.
``ch_armor ARMOR``
  Change the armour amount to ``ARMOR``.  As with ch_health, this is a cheat.
``cl_mtype 1/2``
  If 1, then optimal strafing is performed when ``+linestrafe``, ``+leftstrafe`` or ``+rightstrafe`` is activated.  If 2, then speed preserving strafing is performed instead.
``cl_db4c_ceil 0/1``
  If 1, then the ceiling will be considered as a plane to avoid when ``tas_db4c`` is active.
``sv_taslog 0/1``
  Dump a lot of useful information to the console.
``sv_bcap 0/1``
  Enable or disable bunnyhop cap.
``sv_sim_qg 0/1``
  Simulate a quickgauss shot when releasing the gauss charge.  This is a cheat.
``sv_sim_grf 0/1``
  Simulate gauss rapidfire when firing the gauss cannon in primary mode.  This is a cheat.
``sv_sim_qws 0/1``
  Simulate quick weapon switch upon switching weapons.  This is a cheat.

When strafing commands (``+linestrafe``, ``+leftstrafe``, ``+rightstrafe``, ``+backpedal``) are active, do not activate the "normal" movement commands (such as ``+moveleft``), or else you are asking for trouble.  Also, please avoid doing things like mixing ``+leftstrafe`` and ``+backpedal`` together.


TasTools HUD
------------

TasTools adds several custom HUD elements that can be useful for TAS planning and quick monitoring during script execution.

.. image:: _static/hud-1.jpg

These new elements are located at the top-left corner.  At the time of writing, the positions cannot be changed, nor can their display be disabled.

=======  ===========
item     description
=======  ===========
H        horizontal speed (or more accurately, the magnitude of the change in position per second, which is different from the velocity stored in variables like ``pmove->velocity``)
V        vertical speed
ZA       angle :math:`\arccos n_z` in degrees, where :math:`n_z` is the :math:`z`-component of the plane normal under crosshair
HD       horizontal distance from the player origin to the point under crosshair
VD       vertical distance from the player origin to the point under crosshair
Y        yaw angle
P        pitch angle
EH       health of the entity under crosshair
CN       ``pev->classname`` of the entity under crosshair
DS       a purple square will appear if the duckstate is 2, empty otherwise
ST       strafetype, which shows "F" when linestrafing, "L" when leftstrafing, "R" when rightstrafing, and "B" when backpedalling
SG       the maximum damage the gauss beam can have to trigger selfgauss
=======  ===========


TAS logging
-----------

If ``sv_taslog 1``, TasTools mod will dump mostly player-related information to the console each frame, which can be highly useful to the runner to analyse what exactly happened during the run, especially for very complex and fast paced sequences.  To save the console output to a file, one must either specify the ``-condebug`` switch when executing the game, or issue the ``condebug`` command in the console while the game is running.  The output will be appended to ``qconsole.log`` which resides in the Half-Life directory.

Perhaps even more importantly, this TAS log is essential in generating the final legit script as needed by the genlegit.py script.

The log file is usually not read directly, but instead fed to the qconread program for easier reading, but we will describe the format here.  For each frame the following information will be printed::

    prethink FRAMENO FRAMETIME
    health HP AP
    usercmd MSEC BUTTONS PITCH YAW
    fsu FMOVE SMOVE UMOVE
    fg FRICMULT GRAVMULT
    pa PUNCHPITCH PUNCHYAW
    pos 1 PX PY PZ
    pmove 1 VX VY VZ BX BY BZ INDUCK FLAGS ONGROUND WATERLVL
    ntl NUMTOUCH LADDER
    pos 2 PX PY PZ
    pmove 2 VX VY VZ BX BY BZ INDUCK FLAGS ONGROUND WATERLVL
    [obj PUSH OVX OVY]
    [dmg DAMAGE DMGTYPE DIRX DIRY DIRZ]
    [expld SRCX SRCY SRCZ TARGETX TARGETY TARGETZ ENDX ENDY ENDZ]

The tokens in uppercase here are replaced by the actual value, while those in lowercase are literal.  The lines in square brackets may or may not appear in a particular frame.

``prethink``
  The first line gives the frame number (``FRAMENO``) which is not necessarily unique and ``FRAMETIME`` is the duration of this frame, or the CFR.  The frame number is the value of ``g_ulFrameCount`` defined in ``dlls/globals.cpp``, which is incremented only when ``StartFrame`` in ``dlls/client.cpp`` is called.  The frame time is grabbed from ``gpGlobals->frametime``.
``health``
  The health information is straightforward.  Note that the values are printed in ``CBasePlayer::PreThink``, which is before any damage inflictions or other actions that might alter the health take place.
``usercmd``
  This line is printed before any player physics happen in ``pm_shared.c``.  ``MSEC`` is the UFR, ``BUTTONS`` is ``pmove->cmd.buttons`` which contains bits that correspond to button presses, while ``PITCH`` and ``YAW`` are the original pitch and yaw inputs from the clientside before punchpitch modification.
``fsu``
  ``FMOVE``, ``SMOVE`` and ``UMOVE`` are ``pmove->cmd.forwardmove``, ``pmove->cmd.sidemove`` and ``pmove->cmd.upmove`` respectively.  As with the ``usercmd`` line, these are original inputs from the clientside, before alterations.
``fg``
  ``FRICMULT`` and ``GRAVMULT`` are ``pmove->friction`` and ``pmove->gravity`` respectively.  These are multipliers that change the effective friction coefficient :math:`k` and gravitational acceleration :math:`g` when computing ground movement and gravity.  For example, ``FRICMULT`` is observed to be 0.15 when standing on the films of water at the beginning of c1a1 map.  This means the actual friction coefficient is 0.15 times the default ``sv_friction``.
``pa``
  This line is straightforward.  They are the punchangles before ``PM_DropPunchAngle`` is called.
``pos 1``
  This line gives the player position before physics computations.
``pmove 1``
  The ``VX``, ``VY`` and ``VZ`` are components of the player velocity.  ``BX``, ``BY`` and ``BZ`` are components of player basevelocity.  ``INDUCK`` can be 1 or 0, which tells whether the player duckstate is 1.  ``FLAGS`` is ``pmove->flags``, which can be used to test if the ``FL_DUCKING`` bit is set to determine whether the player duckstate is 2.  ``ONGROUND`` can be -1 (not onground) or anything else (onground).  Lastly, ``WATERLVL`` can be 0, 1 or 2, depending on how deep the player has immersed into some water.  The values in this line are prior to any physics computations.
``ntl``
  When this line is printed, the physics computations have been completed for this frame.  ``NUMTOUCH`` gives the number of entities the player is touching.  ``LADDER`` (0 or 1) tells whether the player is climbing on some ladder.
``pos 2``
  Similar to ``pos 1``, except this is printed after physics computations.
``pmove 2``
  Similar to ``pmove 1``, except this is printed after physics computations.
``obj``
  This line is printed only when pushing or pulling an object.  ``PUSH`` can be 0 or 1, which says whether the interaction with this object is a pull or a push.  ``OVX`` and ``OVY`` are the components of the horizontal object velocity before pulling or pushing.  This line is printed from ``CPushable::Move`` in ``dlls/func_break.cpp``.
``dmg``
  This line is printed only when the player receives damage.  ``DAMAGE`` is the amount of damage received, ``DMGTYPE`` contains bits defined in ``dlls/cbase.h`` which describe the types of damage, while ``DIRX``, ``DIRY`` and ``DIRZ`` are the components of the unit direction vector associated with the damage.  The first two fields in this line are printed from ``CBasePlayer::TakeDamage`` in ``dlls/player.cpp``, while the rest are printed from ``CBaseMonster::TakeDamage`` in ``dlls/combat.cpp``.
``expld``
  This line is printed only when the damage received is a blast damage.  ``SRCX``, ``SRCY`` and ``SRCZ`` are the components of the position of explosion source, ``TARGETX``, ``TARGETY`` and ``TARGETZ`` are the components of the position as returned by the ``BodyTarget`` function, while ``ENDX``, ``ENDY`` and ``ENDZ`` are the components of the position upon which the damage ultimately inflicts.

Parsing the TAS log is straightforward.


Half-Life execution script
--------------------------

In Linux it is not possible to execute ``hl_linux`` directly, as it depends on the values of certain environment variables usually set by the Steam process.  We are therefore forced to use the Steam GUI to specify additional switches to the executable, a process requiring several mouse clicks.  To eliminate this inconvenience we must be able to set up the environment correctly before executing ``hl_linux``, and to do this we first run Half-Life via Steam then grab the values of relevant environment variables by issuing ::

  ps ex | grep '[h]l_linux'

This process has been done for you, and the resulting script, named ``runhl.sh``, is

.. code-block:: bash

    #!/bin/bash

    # Modify this to point to your local Steam directory.
    export STEAM_PATH=~/.local/share/Steam

    export STEAM_RUNTIME=$STEAM_PATH/ubuntu12_32/steam-runtime

    export PATH=$PATH:\
    $STEAM_PATH/ubuntu12_32:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/amd64/bin:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/amd64/usr/bin

    export LD_LIBRARY_PATH=\
    $STEAM_PATH/SteamApps/common/Half-Life:\
    $STEAM_PATH/ubuntu12_32:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/i386/lib/i386-linux-gnu:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/i386/lib:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/i386/usr/lib/i386-linux-gnu:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/i386/usr/lib:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/amd64/lib/x86_64-linux-gnu:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/amd64/lib:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/amd64/usr/lib/x86_64-linux-gnu:\
    $STEAM_PATH/ubuntu12_32/steam-runtime/amd64/usr/lib:\
    $STEAM_PATH/ubuntu12_64:\
    /usr/lib32

    cd $STEAM_PATH/SteamApps/common/Half-Life
    exec ./hl_linux -steam "$@"

Nevertheless, we do not guarantee that this script will run successfully in your system.


Scripting
---------

There are two kinds of script as far as TasTools is concerned: the *simulation script* and the *legitimate script*.  Simulation scripts use TasTools-specific commands and cvars heavily to "simulate" a run. The console output, which is usually saved to ``qconsole.log``, can then be parsed to produce the legitimate script.  This legitimate script can be run in either Minimod or unmodded Half-Life, depending on whether the bhop cap is meant to be present.

One should define the following commonly used aliases in ``userconfig.cfg`` to reduce keystrokes when writing simulation scripts::

    alias +f +linestrafe; alias -f -linestrafe
    alias +l +leftstrafe; alias -l -leftstrafe
    alias +r +rightstrafe; alias -r -rightstrafe
    alias +b +backpedal; alias -b -backpedal
    alias +j +jump; alias -j -jump
    alias +d +duck; alias -d -duck

Along with these recommended settings::

    cl_bob 0
    clockwindow 0
    sv_aim 0
    cl_forwardspeed 10000
    cl_backspeed 10000
    cl_sidespeed 10000
    cl_upspeed 10000

The 10000 for the last four cvars is to max out values of ``forwardmove``, ``sidemove`` and ``upmove`` in ``pmove->cmd``.  According to the file ``delta.lst``, the range for these variables is :math:`[-2047, 2047]`.  With 10000 they will always have the maximum value.

One very convenient aspect of simulation script is that we do not need to write out the ``wait``\ s explicitly.  Instead, we can write mathematical expressions in RPN in place of them.  Then we use ``gensim.py`` which evaluates the expressions and replaces the expressions by the correct number of ``wait``\ s.  It also ignores lines containing only comments and blank lines.  Suppose we have ``myscript.cfg_`` which contains the following lines::

    +f
    101 98 - 1 +
    -f
    +attack
    1
    -attack

In Linux we can simply run ``gensim.py < myscript.cfg_`` which prints the following output to stdout

::

    +f
    wait
    wait
    -f
    +attack
    wait
    -attack

This example is meant to be trivial, but suppose exactly 5452 waits are needed.  The traditional means of using ``wait`` aliases becomes cumbersome as one needs to define an enormous amount of them.  Suppose we write ``w2000;w2000;w1000;w452`` instead.  What if after analysing the log file we decided that 5452 ``wait``\ s are too long by 1738 frames?  As helpful as mental computation is for shopping in supermarkets, it will rarely be quicker than just writing an expression which subtracts 1738 from the original value, unless you calculates at John von Neumann's speed.

If ``gensim.py`` encounters a line with this format: ``@U N1 N2`` where ``N1`` and ``N2`` are integers, then it will output ``N2`` of the following in place of that line::

    <N1 waits>
    +use
    wait
    -use

This is immensely useful for object manoeuvring, instead of copying the same lines manually over and over again, resulting in an unmaintainable script.

In general, very often ``r_norefresh 1`` can come in handy as it disables screen refreshing (though not rendering). This can dramatically increase the frame rate to skip over long sequences or parts that have been completed/finalised.


Script execution
----------------

We will focus on script execution in the latest version of Half-Life.  The technique for older versions is simpler and easier to carry out.

First of all, we must bind a key in ``userconfig.cfg`` to execute the script upon pressing.  Then the content of ``game.cfg`` must have the following format::

    sv_taslog 1
    <waitpad 1>
    pause
    <waitpad 2>
    [save SAVE]

where *waitpad 1* and *waitpad 2* are lines containing only ``wait`` commands.  For the first waitpad, the number of ``wait``\ s, called the *wait number*, must be determined experimentally, usually 20 for maps that are not too complex.  The waitpad should normally be empty and the ``save`` statement should not be present, except for handling level transitions (described in :ref:`segmentation`).

Yet another Python script called ``gamecfg.py`` is written to allow easy generation of ``game.cfg`` conforming to the format described above, though it prints to stdout.  It accepts two mandatory arguments, ``N1`` and ``N2`` which correspond to the wait number for waitpads 1 and 2.  It also accepts the optional flag ``--save`` which causes it to output the final ``save`` statement if specified.  In some rare cases we might not want to enable logging, hence the ``--nolog`` switch.

Usually, to run the game we should have some means of executing the following sequence of commands quickly (this is just an example that works most of the time)::

    rm -f $HLP/qconsole.log
    gamecfg.py 20 0 > $HLP/valve/game.cfg
    runhl.sh -game tastools -condebug +host_framerate 0.0001 +load <savename>

where ``$HLP`` should be defined somewhere in ``.bashrc`` to point to the Half-Life directory.  Having this variable defined can save a lot of keystrokes when typing in the terminal.  Note that modifications to core variables such as ``sv_maxspeed`` should be done by passing switches to ``runhl.sh`` as well.  After starting the game, we must *hold the bound key while the game is still loading*, then release the key *after* the script starts executing.  The purpose of doing this is to ensure the script gets executed as soon as the game engine does ``CL_SignonReply: 2``.  It would not work if we execute the script from ``game.cfg`` instead.

After executing the script we should close the game and have ``qconsole.log`` opened in ``qconread`` for analysis.  We should check the beginning of the log to verify that the script has been executed correctly.

Assuming that the simulation script is finalised.  The legitimate script must then be generated using ``genlegit.py`` by reading the log file from stdin and emits the final script to stdout.  However, it makes one important assumption while generating the legitimate script: at least one frame after the ``CL_SignonReply: 2`` string has zero for ``MSEC`` in the ``usercmd`` line.  It requires such frame to exist in order to know the initial pitch and yaw so that the correct ``cl_yawspeed`` values can be computed.  If such frame does not exist, the runner might have to insert a fake frame manually into the log file to fool the generator.  It also always set ``cl_forwardspeed``, ``cl_sidespeed`` and ``cl_upspeed`` to 10000 as hardcoded into the code.  ``genlegit.py`` also inserts a ``host_framerate 0.0001`` before the final ``wait`` by default, unless ``--noendhfr`` is specified.  This is needed for handling level transitions correctly and is harmless for traditional segmenting within the same map.


.. _segmentation:

Segmentation
------------

When we say a run is "segmented", it simply means it was done piece by piece where each piece is loaded from a saved game (also known as savestate).  One of the main motivations to segmenting a run is to allow human runners to better optimise the run, though another reason is to exploit glitches introduced when saving and loading the game in the middle of some event or process.  For Half-Life TASes, segmentation is always needed for level transition if the run is entirely scripted.

There are two kinds of segment in TASing: *hard segment* and *soft segment*.  Soft segments are done only in long simulated runs.  The game is always saved in TasTools and when the simulation script has been finalised the segments will be merged.  The purpose of soft segement is to decrease development cycle time, just like ``r_norefresh 1`` does.  On the other hand, hard segment is applicable also to legitimate runs and the game must be saved instead in legitimate mods.  Hard segments are used for exploiting savestate related glitches and level transitions.

The segments are easy to create, except for hard segments at level transitions which are trickier to handle.  For this we need ``host_framerate 0.0001`` before the final ``wait`` in the legitimate script which is generated by default by ``genlegit.py``.  Then, *before* the level transition begins we must modify ``game.cfg`` so that it contains 50 ``wait``\ s for waitpad 1 and about 100 ``wait``\ s for waitpad 2, plus the final ``save`` statement.  Obviously, the exact wait numbers must be determined experimentally.  To check if the game was saved correctly we must utilise ``qconread``.  We should verify, after the script ended, that

#. "Player paused the game" is found somewhere before "CL_SignonReply: 1" and *not* "Player unpaused the game"

#. "Saving game to" and another "Player paused the game" appear *within the same frame* as "CL_SignonReply: 2"

#. UFT is always 0 until "CL_SignonReply: 2"

If "Can't save during transition" is found, then the wait number for waitpad 2 must be increased.  Assuming the resulting savestate is correct, the method to execute the script for the next segment will be as normal.


qconread program
----------------

The qconread program is a simple GUI application which parses and presents the qconsole.log in an accessible way so that the runner can have complete knowledge of the player information for each frame.  It is a C++ program using Qt as the GUI toolkit, which happens to serve the need to display a few hundred thousand elements efficiently.

.. image:: _static/qconread.png

It has many columns with succinct labels, including one which is rated PG.  Upon reading a log file, the player information will be populated, with each row representing one frame.

First of all, we have the frame number column, which displays the ``g_ulFrameCount`` values grabbed from ``client.cpp``.  They may not be sequential, and if this is the case in the middle of a run then you may have some trouble.  If a frame number is in bold appended with an asterisk, then you can turn on the display of "Extra lines" by going to View->Extra lines, and click on the frame number to have unrecognised lines displayed in the text box.  The unrecognised lines are lines that appear between this and the next ``prethink`` lines, except for the very first frame, which contains lines above the first ``prethink`` line.

We have "fr" and "ms", which are CFR and UFT.  Then we have "hp" and "ap" which are health and armour amounts.  They are displayed in different colours than the rest to make them stand out.  They may be displayed in red background, white foreground and in bold if the player receives damage.  Hover your mouse over them so that the damage amount and type are displayed in the status bar.  We also have "hspd", which is the horizontal speed computed from ``pmove->velocity`` *after* ``PM_PlayerMove`` returns, while "ang" is the polar angle of horizontal velocity and "vspd" is the vertical speed.  If "hspd" and "ang" are in bold, then it means an object is being pulled or pushed.  You can also hover the mouse cursor over the fields to have the object horizontal speed displayed.  In addition, if an asterisk is displayed at the end of each of "hspd" and "ang" then horizontal basevelocity is nonzero and you can hover the mouse cursor over them to see the basevelocity in the status bar.  The similar is true if "vspd" is bold.  Sometimes the background of all "hspd", "ang" and "vspd" turns light red, in this case the player has collided with some solid entity, which usually changes the velocity.

For "yaw" and "pitch", they may have light purple background.  In this case the corresponding punchangle is nonzero, and again, the mouse cursor can be hovered over them to have the punchangle value displayed.

"fm", "sm" and "um" stands for forwardmove, sidemove and upmove.  They do not currently display the actual values, but rather, the signs of the values.  Blue denotes positive values and red denotes negative values.  "og" is green if the player is onground, and "ds" shows the duckstate, which is white, grey and black for duckstate 0, 1 and 2 respectively, both being states after ``PM_PlayerMove``.  "d", "j", "u", "at", "at2" and "rl" are not white if ``+duck``, ``+jump``, ``+use``, ``+attack``, ``+attack2`` and ``+reload`` are active respectively.  "wl" shows the waterlevel in white, light blue and blue for 0, 1 and 2 respectively, while "ld" tells whether the player is onladder.

Lastly, we have the player positions.  The :math:`z` component is generally more useful.
