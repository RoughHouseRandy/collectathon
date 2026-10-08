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
Change starting position for player and treasure.

Addtionally for speed boost we can initialize a variable to hold 3 boosts.
For speedbBoost {
    if (a is pressed && speed boost is greater than 0) {
        do the speed boost for a few seconds.
    }
}
## Brainstorming game ideas
- Change sprite for character and treasure 
- add debuffs like slow down fake treasures.
>> Create a sprite for the debuff
>> Add a counter to track the coins. At 5, start spwning debuffs Every increment of ten, 
increase the spawn of debuffs.
>> Once speed boosts run out, begining spawning boost pick ups
- Add special pick ups to increase boosts once the character has run out of buffs. Perhaps adding
a conditional: if(player has 0 speed boosts && player current_score > 3){spawn a buff}
- Add more treasures with extra point values.

10/7/26
Changing the music up upon a certain, score threshold - Russell.
Adding a more dyanmic sprite. - Both.
Add a more cavelike background - Both.
Enlarge the sprites - Both.

10/8/25 - Feedback
Cambpell and Esteban used switch statements with coin counts to change level backgrounds and text
We could use this to make a game over screen.
Text clear, blending + transparency.
Joey had a timer and the timer runs outs, game over.
Displays top score.


## Plan for implementing game



