A place to write your findings and plans

## Understanding
I can see that the positions for the character, treasure and score are all initialized before
the infinite while loop. Attributes for the character like size and speed are also initialized 
in a variable before the while loop. Inside the loop is where our controls, and scores are updated
based on player interactions. - Russell
The while loop contains button press and and update function.
The player and the coin always start at the same coordinates when the games boots.
This keeps the programming scanning for inputs and refreshing the screen.
If the player's position intersects with the coin's, the score is increased and a new coin is generated at random x and y coordinates.
- Corey.

## Planning required changes
Change player speed, make character loop, add boost and change backdrop. 

For Speed Boost {
    if (a is pressed && speed boost is greater than 0) {
        do the speed boost for a few seconds.
    }
}
## Brainstorming game ideas
Change sprite for character and treasure, add debuffs like slow down fake treasures.
## Plan for implementing game


