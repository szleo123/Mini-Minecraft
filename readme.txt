Milestone1 

Procedural Terrain: Yilin li 

-I used abs(fractal_noise(abs(Perlin(x, z)))) to generate mountain terrain and offseted Worley noise to generate grass terrain. 
-I used Perlin noise with a very large grid size, remapped it to [0, 1], and used smoothstep(0.4, 0.6, Perlin) to mix two biomes. 
-The water level is bewteen 128 and 142. 

Efficient Terrain Rendering and Chunking: Haochen Wang 

-We created member variable in each chunk to store VBOdata buffer and indices buffer. 
-We iterated through x, y, z position in each chunk and for each of its neighbor, check if it's empty. 
 Create VBO data and indices data only if current chunk is filled and neighbor is empty. 
-A terrain zone is 4x4 chunk. At each tick, we check player surrounding 3x3 terrain zone. If the zone is not created yet, generate it. 
 We rendered every chunk for this milestone. 


Milestone 2     4.11due

Texturing and Texture Animation: Yilin Li 

- I used two interleaved VBO data, one for opaque blocks, the other for transparent blocks. 
- Enabled GL_BLEND and draw every opaque block before drawing transparent blocks. 
- Water is simply animated using a cosine function to go back and forth between three water texture piece in the fragment shader.

Multithreading: Haochen Wang 

-We used standard library to implment multithreading. 
-For each tick, we check terrain zone in a given search radius, if it's not existed, create thread to build blocks in it. 
 If it is existed, check if VBO data has been created, create thread to create VBO data and push to a vector of ChunkVBOdata if not.
 Buffer every ChunkVBOdata at the end of check terrain function and clear the buffer vector. 
-We didn't clear data on GPU because so far we haven't met problem with it.  

Milestone 3 

Day and night cycle (60 pts): Yilin Li 

- I modified codes from Adam's procedural sky code to create the day and night background cycle. 
- I created a new class called Planet which is similar to terrain but has chunks in 3D. 
- I used Planet class to build a sphere which acted as a sun. It rotates around the terrain day and night. 
- Change the light direction corresponding to the position of the sun. 

Game Engine Tick Function and Player Physics(75 pts): Haochen Wang 

- We didn't implement game engine in Milestone 1, so we did in Milestone 3. 
- We created flight mode without gravity and normal node with gravity. 
- We borrowed the gridMarching code from the slide to check for player collision. 
- The blocks can be placed by right clicks and can be removed by left clicks. 