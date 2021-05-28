# ForestShooter
 OpenGL shooter game in a forest
 
 using the things I have learned about openGL, namely model loading, and apply it into a simple game.
 
 Controls:
 
  Arrow Keys  - Move position
  
  Mouse       - Move view
  
  Mouse Right - Shoot
  
  Left Shift  - Sprint
 
  F1          - regenerate chunks
  
  F2          - toggle enemies
 
 
The game loads models and stores them in a model and mesh class. I first import the models with assimp then transfer the models and meshes into my classes.

I use a directional lighting system, where all models are lit from a single direction. This gives the textures more visiblity and looks a lot nicer than without lighting.

There is a skybox, which is a scaled sphere with a repeating cloud texture. To make the objects coming through the skybox look more natural I use a fog system, where colours are shifted towards the skybox blue colour depending on their distance from the camera.

The game generates chucks which contain a random number of trees with random offsets. The chunks generate as you move around the world. chucks get deleted when you move too far away from them.

Enemies are spawned after a delay at a random direction from the player. They travel in the direction of the player, and when the enemy and player collide, the chunks are regenerated and all enemies and bullets are removed.

The player can shoot bullets which destroy enemies when the collide with them, the bullets are removed when they are too far away. When the bullets collide with the ground, their y-velocity is reflected.
