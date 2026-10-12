#include <bn_core.h>
#include <bn_display.h>
#include <bn_log.h>
#include <bn_keypad.h>
#include <bn_random.h>
#include <bn_rect.h>
#include <bn_sprite_ptr.h>
#include <bn_sprite_text_generator.h>
#include <bn_size.h>
#include <bn_string.h>
#include <bn_color.h>
#include <bn_colors.h>
#include <bn_backdrop.h>

#include "bn_sprite_items_player.h"
#include "bn_sprite_items_treasure.h"
#include "bn_sprite_items_snake.h"
#include "bn_sprite_items_snake_2.h"
#include "bn_sprite_items_snake_3.h"
#include "bn_sprite_items_diamond.h"
#include "bn_sprite_items_boostpotion.h"
#include "common_fixed_8x16_font.h"
#include "bn_sound_items.h"
#include "bn_music_items.h"
#include "bn_music.h"
#include "bn_regular_bg_items_cavebackground.h"
#include "bn_regular_bg_items_victory.h"
#include "bn_regular_bg_ptr.h"

// Pixels / Frame player moves at
static constexpr bn::fixed SPEED = 1.5;

// Width and height of the the player and treasure bounding boxes
static constexpr bn::size PLAYER_SIZE = {8, 8};
static constexpr bn::size TREASURE_SIZE = {8, 8};
// Width and height of the snake debuff bounding box
static constexpr bn::size SNAKE_SIZE = {8, 8};
// Width and height of the diamond
static constexpr bn::size DIAMOND_SIZE = {8, 8};
// Width and height of the speed boost potion.
static constexpr bn::size BOOSTPOTION_SIZE = {8, 8};

// Full bounds of the screen
static constexpr int MIN_Y = -bn::display::height() / 2;
static constexpr int MAX_Y = bn::display::height() / 2;
static constexpr int MIN_X = -bn::display::width() / 2;
static constexpr int MAX_X = bn::display::width() / 2;

// Number of characters required to show the longest numer possible in an int (-2147483647)
static constexpr int MAX_SCORE_CHARS = 11;
// Number of boosts is only set to 3. The characters required to show this should be 1
static constexpr int MAX_BOOST_CHARS = 1;
// Timer
static constexpr int MAX_SECONDS = 3;

// Score location
static constexpr int SCORE_X = 105;
static constexpr int SCORE_Y = -70;

// Boost location
static constexpr int BOOST_X = 105;
static constexpr int BOOST_Y = -60;

// Timer location
static constexpr int TIME_X = -75;
static constexpr int TIME_Y = -70;

// Player spawn location
static constexpr int START_X = 0;
static constexpr int START_Y = 50;

// Game timer - Keep track of how long the game has been running.
int game_timer = 0;
bool game_timer_running = false;

int main()
{
    bn::core::init();
    // Set game timer to true
    game_timer_running = true;

    // Background image
    bn::regular_bg_ptr bg = bn::regular_bg_items::cavebackground.create_bg(0, 0);

    bn::random rng = bn::random();
    bn::fixed current_speed = SPEED;
    // Will hold the sprites for the score
    bn::vector<bn::sprite_ptr, MAX_SCORE_CHARS> score_sprites = {};
    bn::sprite_text_generator text_generator(common::fixed_8x16_sprite_font);

    // Will hold the sprites for boosts
    bn::vector<bn::sprite_ptr, MAX_BOOST_CHARS> speed_boost_sprites = {};

    // Display text for the score, timer and boosts
    bn::vector<bn::sprite_ptr, 32> text_sprites; // Vector for generated text
    text_generator.generate(45, -70, "Score:", text_sprites);
    text_generator.generate(45, -60, "Boosts:", text_sprites);

    bn::vector<bn::sprite_ptr, MAX_SECONDS> timer_sprites = {};
    text_generator.generate(-115, -70, "Time:", text_sprites);

    int score = 0;
    int speedBoost = 3;

    // Background music
    bn::music_items::cavern.play(0.5);

    // Will hold the timer for the speed boost.
    int speed_boost_timer = 0;
    bool speed_boost_active = false;
    // Will hold the timer for debuff
    int debuff_timer = 0;
    bool debuff_active = false;
    // Will hold the interval for spawning diamond
    int diamond_timer = 0;
    bool diamond_activated = false;
    // Will hold a score tracker to track intervals to spawn items
    int next_diamond_score = 3;
    // Will hold tracking for boost potion variables
    int potion_activated = false;
    int next_boost_score = 2;
    int potion_timer = 0;
    // Variables for the spawn coordinates for the snakes
    int snk_x = 0;
    int snk_2_x = 0;
    int snk_3_x = 0;

    int snk_y = 0;
    int snk_2_y = 0;
    int snk_3_y = 0;

    int diamond_x = 0;
    int diamond_y = 0;

    int potion_x = 0;
    int potion_y = 0;

    bn::sprite_ptr player = bn::sprite_items::player.create_sprite(START_X, START_Y);
    bn::sprite_ptr treasure = bn::sprite_items::treasure.create_sprite(0, 0);

    // Start the snakes out of bounds. With the scroll loop logic, the player will not touch it here.
    bn::sprite_ptr snake = bn::sprite_items::snake.create_sprite(300, 300);
    bn::sprite_ptr snake_2 = bn::sprite_items::snake_2.create_sprite(300, 300);
    bn::sprite_ptr snake_3 = bn::sprite_items::snake_3.create_sprite(300, 300);

    // Spawn a more valuable treasure to allow for double points
    bn::sprite_ptr diamond = bn::sprite_items::diamond.create_sprite(320, 320);

    // Spawn a boost potion
    bn::sprite_ptr potion = bn::sprite_items::boostpotion.create_sprite(320, 320);

    while (true)
    {
        // Start the game timer
        game_timer++;

        // tracks the coordinate for x and y.
        int current_x = player.x().round_integer();
        int current_y = player.y().round_integer();

        // Move player with d-pad
        if (bn::keypad::left_held())
        {
            player.set_x(player.x() - current_speed);
        }
        if (bn::keypad::right_held())
        {
            player.set_x(player.x() + current_speed);
        }
        if (bn::keypad::up_held())
        {
            player.set_y(player.y() - current_speed);
        }
        if (bn::keypad::down_held())
        {
            player.set_y(player.y() + current_speed);
        }
        if (bn::keypad::start_pressed())
        {
            // Clear the HUD
            text_sprites.clear();
            bg = bn::regular_bg_items::cavebackground.create_bg(0, 0);
            // When the start button is pressed - reset the position of treasure, score, and player
            player.set_position(START_X, START_Y);
            snake.set_position(300, 300);
            snake_2.set_position(300, 300);
            snake_3.set_position(300, 300);
            treasure.set_position(0, 0);
            diamond.set_position(320, 320);

            // Regenerate the HUD
            text_generator.generate(45, -70, "Score:", text_sprites);
            text_generator.generate(45, -60, "Boosts:", text_sprites);
            text_generator.generate(-115, -70, "Time:", text_sprites);
            score = 0;              // Reset the score
            speedBoost = 3;         // Reset the speed boost
            game_timer = 0;         // Reset the game timer
            next_boost_score = 2;   // Reset the score to spawn boost
            next_diamond_score = 3; // Reset the score to spawn diamond

            // Start the bgm again
            bn::music_items::cavern.play(0.5);
        }

        // When the A button is pressed apply a speed boost that lasts 5 seconds
        if (bn::keypad::a_pressed() && (speedBoost > 0))
        {
            // Speed boost is activated
            speed_boost_active = true;
            speed_boost_timer = 0;
            // Speed boost should decrement after it is used. speedBoost--;
            speedBoost--;
        }

        // The bounding boxes of the player and treasure, snapped to integer pixels
        bn::rect player_rect = bn::rect(player.x().round_integer(),
                                        player.y().round_integer(),
                                        PLAYER_SIZE.width(),
                                        PLAYER_SIZE.height());
        bn::rect treasure_rect = bn::rect(treasure.x().round_integer(),
                                          treasure.y().round_integer(),
                                          TREASURE_SIZE.width(),
                                          TREASURE_SIZE.height());
        // The bounding box of the snake debuff
        bn::rect snake_rect = bn::rect(snake.x().round_integer(),
                                       snake.y().round_integer(),
                                       SNAKE_SIZE.width(),
                                       SNAKE_SIZE.height());
        // The bounding box of the snake_2 debuff
        bn::rect snake_2_rect = bn::rect(snake_2.x().round_integer(),
                                         snake_2.y().round_integer(),
                                         SNAKE_SIZE.width(),
                                         SNAKE_SIZE.height());

        // The bounding box of the snake_3 debuff
        bn::rect snake_3_rect = bn::rect(snake_3.x().round_integer(),
                                         snake_3.y().round_integer(),
                                         SNAKE_SIZE.width(),
                                         SNAKE_SIZE.height());
        // The bounding box of the diamond
        bn::rect diamond_rect = bn::rect(diamond.x().round_integer(),
                                         diamond.y().round_integer(),
                                         DIAMOND_SIZE.width(),
                                         DIAMOND_SIZE.height());
        // The bounding box of the potion
        bn::rect potion_rect = bn::rect(potion.x().round_integer(),
                                        potion.y().round_integer(),
                                        BOOSTPOTION_SIZE.width(),
                                        BOOSTPOTION_SIZE.height());

        // If the bounding boxes overlap, set the treasure to a new location an increase score
        if (player_rect.intersects(treasure_rect))
        {
            // Jump to any random point in the screen
            int new_x = rng.get_int(MIN_X, MAX_X);
            int new_y = rng.get_int(MIN_Y, MAX_Y);
            treasure.set_position(new_x, new_y);
            // If the player interesects with the treasure, play a jingle sfx.
            bn::sound_items::sparkle.play();
            score++;
        }

        // Flags used to check which snake is touched
        bool touch_snake = player_rect.intersects(snake_rect);
        bool touch_snake_2 = player_rect.intersects(snake_2_rect);
        bool touch_snake_3 = player_rect.intersects(snake_3_rect);

        // If the player's bounding box hits the snake, remove a point from the player
        if (player_rect.intersects(snake_rect) || player_rect.intersects(snake_2_rect) || player_rect.intersects(snake_3_rect))
        {
            // Play the snake rattle
            bn::sound_items::rattlesnake.play();
            // Set the random spawn coordinates for the snake
            if (touch_snake)
            {
                snk_x = rng.get_int(MIN_X, MAX_X);
                snk_y = rng.get_int(MIN_Y, MAX_Y);
                snake.set_position(snk_x, snk_y); // Move the snake once touched
            }
            if (touch_snake_2)
            {
                snk_2_x = rng.get_int(MIN_X, MAX_X);
                snk_2_y = rng.get_int(MIN_Y, MAX_Y);
                snake_2.set_position(300, 200); // Move the snake once touched
            }
            if (touch_snake_3)
            {
                snk_3_x = rng.get_int(MIN_X, MAX_X);
                snk_3_y = rng.get_int(MIN_Y, MAX_Y);
                snake_3.set_position(300, 200); // Move the snake once touched
            }

            // Reduce the player score and start debuff/debuff timer
            debuff_active = true;
            debuff_timer = 0;
            score--;
        }

        // If the player intersects with the diamond, add 2 points to the player
        if (diamond_activated && player_rect.intersects(diamond_rect))
        {
            bn::sound_items::sparkle.play();
            score += 2;
            // random spawn
            int rnd_spawnx = rng.get_int(MIN_X, MAX_X);
            int rnd_spawny = rng.get_int(MIN_Y, MAX_Y);
            diamond.set_position(rnd_spawnx, rnd_spawny);
            diamond_timer = 0;
            diamond_activated = true;
            next_diamond_score = score + 3;
        }
        if (potion_activated && player_rect.intersects(potion_rect))
        {
            if (speedBoost <= 3)
            {
                speedBoost++;
            }
            int rnd_potionx = rng.get_int(MIN_X, MAX_X);
            int rnd_potiony = rng.get_int(MIN_Y, MAX_Y);
            potion.set_position(rnd_potionx, rnd_potiony);
            potion_timer = 0;
            potion_activated = true;
            next_boost_score = score + 2;
        }

        // If the player is outside the bounds of the screen, they loop to the other side
        if (player.x() > MAX_X)
        {
            player.set_position(MIN_X, current_y);
        }
        if (player.x() < MIN_X)
        {
            player.set_position(MAX_X, current_y);
        }
        if (player.y() > MAX_Y)
        {
            player.set_position(current_x, MIN_Y);
        }
        if (player.y() < MIN_Y)
        {
            player.set_position(current_x, MAX_Y);
        }

        // Changes player speed if speed boost is active
        if (speed_boost_active)
        {
            current_speed = SPEED * 3;
            speed_boost_timer++;
            if (speed_boost_timer == 180)
            {
                speed_boost_active = false;
                current_speed = SPEED;
            }
        }

        // If debuff is active, the player slows down
        if (debuff_active)
        {
            current_speed = SPEED - 1;
            debuff_timer++;
            if (debuff_timer == 120)
            {
                debuff_active = false;
                current_speed = SPEED;
            }
        }

        // Update score display
        bn::string<MAX_SCORE_CHARS> score_string = bn::to_string<MAX_SCORE_CHARS>(score);
        score_sprites.clear();
        text_generator.generate(SCORE_X, SCORE_Y,
                                score_string,
                                score_sprites);

        // Update speed boost display
        bn::string<MAX_BOOST_CHARS> boost_string = bn::to_string<MAX_BOOST_CHARS>(speedBoost);
        speed_boost_sprites.clear();
        text_generator.generate(BOOST_X, BOOST_Y, boost_string, speed_boost_sprites);

        // Update the display for the timer. Divide fps by 60 to get seconds
        bn::string<MAX_SECONDS> timer_string = bn::to_string<MAX_SECONDS>(game_timer / 60);
        timer_sprites.clear();
        text_generator.generate(TIME_X, TIME_Y, timer_string, timer_sprites);

        // If the player score is a multiple of 5, add a snake
        if (score >= 5)
        {
            // Spawn in the snake within the bounds of the screen
            snake.set_position(snk_x, snk_y);
            if (score >= 10)
            {
                snake_2.set_position(snk_2_x, snk_2_y);
                if (score >= 15)
                {
                    snake_3.set_position(snk_3_x, snk_3_y);
                }
            }
        }
        // Activates diamond after score threshold is met.
        if (!diamond_activated && score >= next_diamond_score)
        {
            diamond_x = rng.get_int(MIN_X, MAX_X);
            diamond_y = rng.get_int(MIN_Y, MAX_Y);
            diamond.set_position(diamond_x, diamond_y);
            diamond_timer = 0;
            diamond_activated = true;
        }
        // This sets a timer once the diamond is spawned and will despawn if not touched
        if (diamond_activated)
        {
            diamond_timer++;
            if (diamond_timer == 240)
            {
                diamond.set_position(320, 320);
                diamond_activated = false;
            }
        }
        // activates the potion spawn if the player ran out of boosts and the score threshold is met.
        if (!potion_activated && speedBoost == 0 && (score >= next_boost_score))
        {
            potion_x = rng.get_int(MIN_X, MAX_X);
            potion_y = rng.get_int(MIN_Y, MAX_Y);
            potion.set_position(potion_x, potion_y);
            potion_timer = 0;
            potion_activated = true;
            next_boost_score = score + 2;
        }
        // This sets the timer despawning a boost potion.
        if (potion_activated)
        {
            potion_timer++;
            if (potion_timer == 240 || speedBoost >= 1)
            {
                potion.set_position(320, 320);
                potion_activated = false;
            }
        }

        // Win condition - If player score gets to 30
        if (score >= 5)
        {
            // Clear all the sprites vectors to clear the screen
            bg = bn::regular_bg_items::victory.create_bg(0, 0);
            text_sprites.clear();
            timer_sprites.clear();
            speed_boost_sprites.clear();
            bn::music::stop();                                        // Stop the bgm
            text_generator.generate(45, -70, "Score:", text_sprites); // Redisplay the score
            text_generator.generate(-30, 0, "You Win!", text_sprites);
            player.set_position(1000, 1000);
            text_generator.generate(-100, 10, "Press Start to play again!", text_sprites);
        }
        else if (game_timer >= 600)
        {
            // Clear all the sprites vectors to clear the screen 1800
            text_sprites.clear();
            timer_sprites.clear();
            speed_boost_sprites.clear();
            bn::music::stop(); // Stop the bgm
            player.set_position(300, 300);
            text_generator.generate(45, -70, "Score:", text_sprites); // Redisplay the score
            text_generator.generate(-40, 0, "You Lose!", text_sprites);
            text_generator.generate(-100, 10, "Press Start to play again!", text_sprites);
        }

        rng.update();

        bn::core::update();
    }
}