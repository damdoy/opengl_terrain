# Procedural terrain generation (Work in progress)

The goal of this project is to mix the examples done here : https://github.com/damdoy/opengl_examples
into a big procedurally generated hilly terrain.

This runs on a laptop with a pretty weak gpu (Intel HD Graphics 620 Intel core i7-7500U 8GB Ram) at ~30fps.

#### Video:

[![Terrain](http://img.youtube.com/vi/FPl3LFbh6wM/0.jpg)](https://youtu.be/FPl3LFbh6wM "Terrain")

#### Features:
- Dynamic LOD terrain (on cpu)
- Dynamic grass
- Shadow mapping
- Dynamic sky
- Phong lighting model
- Water reflections
- Trees
- Shadow mapping

Everything is procedurally generated.

## Controls
- `A/S/D/W` Move
- `J/K/L/I` Look around
- `T/Z` Activate/deactivate moving sun
- `O/P` Increase/decrase amount of clouds

## how to build
Create a `build` folder and call `cmake ..` inside it, then `make`

This was build on a Linux Mint 18.1, the needed libraries are the following:

- glfw2 2.7.9
- glew 1.13.0
- devIl 1.7.8
