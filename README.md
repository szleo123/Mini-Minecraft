**University of Pennsylvania, Mini-Minecraft**

* Yilin Li

## Introduction 
This repository contains the final project of CIS 560 Interactive Computer Graphics. 

Check out the video presentation here: https://www.youtube.com/watch?v=LQ_T4J69U6w

## How to open 
1. Clone the project and go to ./assignment_package folder.
2. Open `miniMinecraft` with Qt.
3. Enjoy the mini game.

## How to control
- Use `W` `A` `S` `D` to move the Player, `Q` `E` to change the elevation of the Player while in flight mode.
- Use `Space bar` to jump.
- Use `right click` of the mouse to build a stone block and `left click` to delete a block.
- Use `F` to switch between normal mode and flight mode. 

## Some key features
- I used a combination of `fractal_noise` and `Perlin_noise` to generate mountain terrain and offseted Worley noise to generate grass terrain.
- I used Perlin noise with a very large grid size, remapped it to [0, 1], and used `smoothstep(0.4, 0.6, Perlin)` to mix two biomes.
- Water is simply animated using a cosine function to go back and forth between three water texture piece in the fragment shader.
- I used Planet class to build a sphere which acted as a sun. It rotates around the terrain day and night. The light direction is changed in accordance with the position of the sun.
- I used grid marching technique to check for player's collision. 
