# Knight

This sample shows a knight model with reflections coming from a cube map. The
pipeline applies high dynamic range lighting with simple tone mapping and a
bloom effect. The knight has a particle system attached to the tip of his sword
which results in some interesting graphical effects. Using the keyboard it is
possible to blend seamlessly between two animations.


## Input

 * Use WASD to move and the mouse to look around.
 * Hold down LSHIFT to move faster.
 * Use 1 and 2 to blend between character animations.
 * Space freezes the scene, hitting space two times freezes the camera as well.
 * F1 sets fullscreen mode.
 * F3 switches between hdr, standard forward lighting and deferred shading.
 * F6 toggles frame stats and information display.
 * F7 toggles debug view.
 * F8 toggles wireframe mode.
 * ESC quits the application.

## Notes on content

The character model and animations were created by Rob Galanakis (www.robg3d.com)
and are distributed under the terms of the Creative Commons Attribution-
Noncommercial 3.0 License (http://creativecommons.org/licenses/by-nc/3.0/).
The cubemap texture is a modified version of one of M@dcow's high res skymaps
which can be found at BlenderArtist.org.
