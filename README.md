# Weedkiller Laser - an 8x8 matrix game 

As a gardener, your job is to make sure people's yards are being taken care of. That includes making sure that the weeds growing are kept at bay, which is a trifle for your skillful hands. But when a rival gardening corporation appears in town, you start to worry about going out of business. That's when an alien spaceship drops one of their weapons in your courtyard one night, a laser gun inscriptioned Lasergun3000. After firing it accidentally, you notice something: the clump of weeds that was your target is completely gone, leaving the grass patch cleaner than a lawn mower could ever. With this new tool, you could make sure your small business doesn't go bankrupt. 

Destroy the weeds in people's yards with the laser in the shortest amount of time to secure your company's future in Weedkiller Laser!

## Gameplay

Using a joystick, you can move up, down, left and right on an 8x8 matrix. Shoot a laser bullet out of the gun by pressing the switch button on the joystick. The bullet will wipe everything in its range. The game has 5 levels, each getting harder and harder (more weed spots will appear on the map). The player can't go through a patch of weeds and they can't go out of the bounds of the matrix (garden). The game keeps count of the time passed since it started, which should be use to determine a highscore ranking (didn't implement it as of now).

I chose to implement this tanks style game because to me it was kinda fun to play and it reminded me a bit of the old arcade games I used to play like crazy when I was five.

## Menu

The menu of the game is navigated with a joystick. It contains the options to start the game, info about the creator (me), a short description of how to play and a settings submenu, from where you can adjust matrix brightness, LCD brightness and toggle the buzzer on and off (didn't implement as of now). The buzzer will make a sound for scrolling through the menu and when selecting an option.

## The project requirements

Considering how this project was made for my robotics course, there were some requirements to meet for the aimed grade: </br>
- <b>Minimal components</b>: LCD, Matrix, Joystick, Buzzer and of course, wires, capacitors and the Arduino Board. </br>
- Sounds when playing the game. (didn't implement as of now) </br>
- Intuitive and fun to play. </br>
- Must make sense with the setup. </br>
- Must have a difficulty increase. </br>

Aside from these, there are obviously the other requirements, regarding a clean coding style. As of now, I am aware of a few slip ups inside this code, such as the use of delays and magic numbers.

You can find photos of the setup inside the "Setup pics" directory, and the demo on how it works on here : (insert link)
