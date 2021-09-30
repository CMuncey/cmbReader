# cmbReader

A very unfinished cmb (and other file formats) reader for OoT3D

## Requirements:

* OpenGL verison 4.6

## Compilation:

Requires glad, glfw, and glm (included).

On Linux, run the makefile to compile, you will need an `obj/` directory because git doesn't allow you to add empty directories apparently.

I've gotten it to work on Windows once, but I don't remember how.

To my knowledge, MacOS does not support OpenGL

## Usage:

`./ViewCMB <file.cmb>`

This is still very much in development, so the keybindings are weird.

* `WASD` to move the camera
* `MOUSE` to look around
* `SCROLL` to zoom in and out
* `ESC` to exit
* `TAB` to toggle wireframe
* `LSHIFT` to speed up the camera
* `C` to unlock/lock the cursor
* `UP/DOWN ARROW` to scale the model up and down
* `LEFT/RIGHT ARROW` to change which mesh is loaded, leftmost is all meshes in model