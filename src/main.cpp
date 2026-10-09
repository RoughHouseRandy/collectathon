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
#include "common_fixed_8x16_font.h"
#include "bn_sound_items.h"
#include "bn_music_items.h"

// Pixels / Frame player moves at
static constexpr bn::fixed SPEED = 1.5;

// Width and height of the the player and treasure bounding boxes
static constexpr bn::size PLAYER_SIZE = {8, 8};
static constexpr bn::size TREASURE_SIZE = {8, 8};
// Width and height of the snake debuff bounding box
static constexpr bn::size SNAKE_SIZE = {8, 8};

// Full bounds of the screen
static constexpr int MIN_Y = -bn::display::height() / 2;
static constexpr int MAX_Y = bn::display::height() / 2;
static constexpr int MIN_X = -bn::display::width() / 2;
static constexpr int MAX_X = bn::display::width() / 2;

// Number of characters required to show the longest numer possible in an int (-2147483647)
static constexpr int MAX_SCORE_CHARS = 11;
// Number of boosts is only set to 3. The characters required to show this should be 1
static constexpr int MAX_BOOST_CHARS = 1;

// Score location
static constexpr int SCORE_X = 70;
static constexpr int SCORE_Y = -70;

// Boost location

static constexpr int BOOST_X = 70;
static constexpr int BOOST_Y = -60;

// Player spawn location
static constexpr int START_X = 0;
static constexpr int START_Y = 50;

int main()
{
    bn::core::init();
    // Background color
    bn::backdrop::set_color(bn::color(0x0210));

    bn::random rng = bn::random();
    bn::fixed current_speed = SPEED;
    // Will hold the sprites for the score
    bn::vector<bn::sprite_ptr, MAX_SCORE_CHARS> score_sprites = {};
    bn::sprite_text_generator text_generator(common::fixed_8x16_sprite_font);

    // Will hold the sprites for boosts
    bn::vector<bn::sprite_ptr, MAX_BOOST_CHARS> speed_boost_sprites = {};

    int score = 0;
    int speedBoost = 3;

    // Background music
    bn::music_items::cavern.play(0.5);

    // Will hold the timer for the speed boost.
    int speed_boost_timer = 0;
    bool speed_boost_active = false;

    bn::sprite_ptr player = bn::sprite_items::player.create_sprite(START_X, START_Y);
    bn::sprite_ptr treasure = bn::sprite_items::treasure.create_sprite(0, 0);
    bn::sprite_ptr snake = bn::sprite_items::snake.create_sprite();

    while (true)
    {

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
            // When the start button is pressed - reset the position of treasure, score, and player
            score = 0;
            player.set_position(START_X, START_Y);
            treasure.set_position(0, 0);
            speedBoost = 3; // Reset the speed boost
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

        bn::rect snake_rect = bn::rect(snake.x().round_integer(),
                                       snake.y().round_integer(),
                                       SNAKE_SIZE.width(),
                                       SNAKE_SIZE.height());

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

        // If the player's bounding box hits the snake, remove a point from the player
        if (player_rect.intersects(snake_rect))
        {
            // Move the snake outside of the screen
            int out_of_bounds_x = 300;
            int out_of_bounds_y = 200;
            // Reduce the player score
            score--;
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

        // // If the player score is a multiple of 5, add a snake
        // if (score / 5 == 0)
        // {
        //     bn::sprite_ptr snake = bn::sprite_items::snake.create_sprite();

        //     // The bounding box of the snake debuff
        //     bn::rect snake_rect = bn::rect(snake.x().round_integer(),
        //                                    snake.y().round_integer(),
        //                                    SNAKE_SIZE.width(),
        //                                    SNAKE_SIZE.height());
        // }

        // Update RNG seed every frame so we don't get the same sequence of positions every time
        rng.update();

        bn::core::update();
    }
}

void make_snake()
{
    bn::sprite_ptr snake = bn::sprite_items::snake.create_sprite();
};