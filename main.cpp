#include "splashkit.h"
#include <iostream>

const int SCREEN_RESOLUTION = 1080;

const int ROOM_WIDTH = 40;  // aka the column
const int ROOM_HEIGHT = 30; // aka the row
// the ratio of the room is 4:3

// TODO:
// member functions
// forward declaration of structs inside structs

double zoom_level = 1;
// TODO: focused zoom feature
//  if get_tile_size function returns a larger value, everything zooms in
//  a tracking camera can be implemented by calculated the coordinate of the top left that would keep the character in the middle

// size of each tile of the room, also the value that everything in the same is scaled by
int get_tile_size()
{
    return SCREEN_RESOLUTION / ROOM_HEIGHT * zoom_level;
}

int get_screen_width()
{
    double width = (double)SCREEN_RESOLUTION * ((double)ROOM_WIDTH / (double)ROOM_HEIGHT);
    return (int)width;
}

struct coordinate
{
    double x, y;

    coordinate()
    {
        x = 0;
        y = 0;
    }

    coordinate(double x, double y)
    {
        this->x = x;
        this->y = y;
    }

    coordinate tile_to_pixel()
    {
        return {x * get_tile_size(), y * get_tile_size()};
    }
};

class game_timing_data
{
private:
    double current_time;
    double delta_time;
    double last_update_time;
    double time_rate; // how many seconds the game should load in one second

public:
    game_timing_data()
    {
        current_time = current_ticks();
        last_update_time = 0;
        time_rate = 1;
    }

    int get_current_time()
    {
        return current_ticks();
    }

    // must be ran inside a game loop
    void update_timing()
    {
        delta_time = (get_current_time() - last_update_time) / time_rate;
        last_update_time = get_current_time();
    }

    int get_delta_time()
    {
        return delta_time;
    }

    void set_time_rate(double rate)
    {
        time_rate = rate;
    }
};

// TODO: for the zoom camera: tile_data must have its own position, position set when creating floor array
// floor tile struct
struct tile_data
{
    color tile_color;
    bool passable = true;
    int size = get_tile_size();
};

// flooring struct, using tiles as arrays
class room_data
{
private:
    tile_data floor_array[ROOM_HEIGHT][ROOM_WIDTH];
    color color_pattern[3]; // 0 and 1 is the floor checkers color pattern, 2 is the walls
    int size_y = ROOM_HEIGHT;
    int size_x = ROOM_WIDTH;
    coordinate spawn_coords;

    void construct_room(color floor_color_1, color floor_color_2, color wall_color, coordinate spawn_coords)
    {
        // setting room color
        color_pattern[0] = floor_color_1;
        color_pattern[1] = floor_color_2;
        color_pattern[2] = wall_color;

        this->spawn_coords = spawn_coords;

        for (int y = 0; y < size_y; y++)
        {
            for (int x = 0; x < size_x; x++)
            {

                // making the walls of the room, they cannot be passed
                if (x == 0 || x == size_x - 1 || y == 0 || y == size_y - 1)
                {
                    floor_array[y][x].tile_color = color_pattern[2];
                    floor_array[y][x].passable = false;
                    continue;
                }

                // making the floor as a checked pattern
                color first_color = color_pattern[0];
                color second_color = color_pattern[1];
                if (y % 2 != 0)
                {
                    first_color = color_pattern[1];
                    second_color = color_pattern[0];
                }

                if (x % 2 == 0)
                    floor_array[y][x].tile_color = first_color;
                else
                    floor_array[y][x].tile_color = second_color;
            }
        }
    }

public:
    // FIXME: each tile should also record its pixel coordinate // FIXME: wait why?
    // Constructor
    room_data(coordinate spawn_coords)
    {
        // setting room color
        color slate_grey = rgb_color(112, 128, 144);
        color light_slate_grey = rgb_color(132, 144, 153);
        color light_steel_blue = rgb_color(150, 170, 200);

        construct_room(slate_grey, light_slate_grey, light_steel_blue, spawn_coords);
    }

    room_data(color floor_color_1, color floor_color_2, color wall_color, coordinate spawn_coords)
    {
        construct_room(floor_color_1, floor_color_2, wall_color, spawn_coords);
    }

    void draw()
    {
        for (int y = 0; y < size_y; y++)
        {
            for (int x = 0; x < size_x; x++)
            {
                color tile_color = floor_array[y][x].tile_color;
                coordinate tile_coords = {(double)x, (double)y};
                coordinate coords = tile_coords.tile_to_pixel();
                int size = floor_array[y][x].size;
                fill_rectangle(tile_color, coords.x, coords.y, size, size);
            }
        }
    }

    coordinate get_spawn_coords()
    {
        return spawn_coords;
    }
};

enum sword_phase
{
    SWORD_DRAW = 0,
    SWORD_SWING,
};

// sword info stuct
struct sword_data
{
    bitmap sword_draw_model;
    bitmap sword_swing_model;
    double model_scaling;
    coordinate position;
    sword_phase phase;
};

// TODO: move all the default values to the constructor
// player info struct
class player_data
{
private:
    int health;
    double speed; // pixels per second

    coordinate position;
    rectangle hurtbox;
    bitmap player_model;
    bool model_facing_right = true;
    double model_scaling;

    int attack_speed; // ms
    bool is_attacking = false;
    bool create_hitbox = false;
    rectangle hitbox;
    int hitbox_lasting_time = 100; // ms
    sword_data sword;

    double attack_cooldown;
    bool can_attack;

    void attacking()
    {
        if (attack_cooldown < attack_speed)
        {
            sword.phase = SWORD_DRAW;
            draw_sword();
        }
        else if (attack_cooldown < attack_speed * 1.5)
        {
            sword.phase = SWORD_SWING;
            if (attack_cooldown < attack_speed + hitbox_lasting_time)
                create_hitbox = true;
            else
                create_hitbox = false;
            draw_sword();
        }
    }

    void construct_player(double model_size, coordinate spawn_coords)
    {
        // setting player
        player_model = load_bitmap("player_idle", "./image_data/player/player_idle.png");

        // default values or stats
        health = 10;                     // FIXME: change
        speed = 5.0 * model_size / 1000; // pixels per milisecond
        attack_speed = 1000;             // ms

        can_attack = true;
        attack_cooldown = 0;

        double model_width = bitmap_width(player_model);
        double model_height = bitmap_height(player_model);

        if (model_width < model_height)
        {
            model_scaling = model_size / model_width;
        }

        if (model_height < model_width)
        {
            model_scaling = model_size / model_height;
        }

        position = coordinate(spawn_coords.x, spawn_coords.y).tile_to_pixel();
        update_box();

        // setting sword struct
        sword.sword_draw_model = load_bitmap("Sword draw", "./image_data/sword/sword_1.png");
        sword.sword_swing_model = load_bitmap("Sword attack", "./image_data/sword/sword_2.png");

        // both sword bitmap have the same size
        double sword_model_width = bitmap_width(sword.sword_draw_model);
        double sword_model_height = bitmap_height(sword.sword_draw_model);

        // flip when facing opposite direction
        if (sword_model_width < sword_model_height)
        {
            sword.model_scaling = model_size / sword_model_width;
        }

        if (sword_model_height < sword_model_width)
        {
            sword.model_scaling = model_size / sword_model_height;
        }

        update_sword();
    }

public:
    // Constructor
    player_data(double model_size)
    {
        construct_player(model_size, coordinate(0, 0));
    }

    player_data(double model_size, coordinate spawn_coords)
    {
        construct_player(model_size, spawn_coords);
    }

    // getters and setters
    double get_speed()
    {
        return speed;
    }

    void set_speed(double speed)
    {
        this->speed = speed;
    }

    int get_health()
    {
        return health;
    }

    void set_health(int health)
    {
        this->health = health;
    }

    int get_attack_speed()
    {
        return attack_speed;
    }

    void set_attack_speed(int attack_speed)
    {
        this->attack_speed = attack_speed;
    }

    coordinate get_position()
    {
        return position;
    }

    void set_position(coordinate position)
    {
        this->position = position;
    }

    void set_is_facing_right(bool facing_right)
    {
        model_facing_right = facing_right;
    }

    void attack()
    {
        if (can_attack)
        {
            is_attacking = true;
        }
    }

    // update the hurtbox to align with the player's position
    void update_hurtbox()
    {
        double player_model_width = bitmap_width(player_model);
        double player_model_height = bitmap_height(player_model);
        hurtbox = {position.x, position.y, player_model_width * model_scaling, player_model_height * model_scaling};
    }

    // update the hitbox to align with the player's position
    void update_hitbox()
    {
        double player_model_width = bitmap_width(player_model);

        double sword_model_width = bitmap_width(sword.sword_draw_model);
        double sword_model_height = bitmap_height(sword.sword_draw_model);
        double hitbox_size_x, hitbox_size_y;

        // player hitbox is 0 if there isnt an attack happening (which is when create_hitbox is false)
        if (create_hitbox)
        {
            hitbox_size_x = sword_model_width * sword.model_scaling;
            hitbox_size_y = sword_model_height * sword.model_scaling * 2;
        }
        else
        {
            hitbox_size_x = 0;
            hitbox_size_y = 0;
        }

        if (model_facing_right)
            hitbox = {position.x + (player_model_width * model_scaling), position.y, hitbox_size_x, hitbox_size_y};
        else
            hitbox = {position.x - hitbox_size_x, position.y, hitbox_size_x, hitbox_size_y};
    }

    void update_box()
    {
        update_hitbox();
        update_hurtbox();
    }

    // update the sword's position to align with the player's position
    void update_sword()
    {
        double player_model_width = bitmap_width(player_model);
        double player_model_height = bitmap_height(player_model);

        double sword_model_width = bitmap_width(sword.sword_draw_model);

        // making the sword align with the player
        if (model_facing_right)
        {
            sword.position.x = position.x + (player_model_width * model_scaling);
            sword.position.y = position.y + (player_model_height / 2 * model_scaling);
        }
        else
        {
            sword.position.x = position.x - (sword_model_width * sword.model_scaling);
            sword.position.y = position.y + (player_model_height / 2 * model_scaling);
        }
    }

    // must be ran inside game loop
    void update_attack_cooldown(double delta_time)
    {
        attack_cooldown += delta_time; // FIXME: attack_cooldown wont accumulate

        if (attack_cooldown >= attack_speed * 2)
        {
            can_attack = true;
            is_attacking = false;
            attack_cooldown = 0;
        }
        else
            can_attack = false;
    }

    void update(game_timing_data &game)
    {
        update_box();
        update_sword();

        if (is_attacking)
        {
            update_attack_cooldown(game.get_delta_time());
            attacking();
        }

        // printf("attack cooldown: %d, delta time %d \n", attack_cooldown, game.get_delta_time());
    }

    double get_attack_cool_down()
    {
        return attack_cooldown;
    }

    // draw player onto screen
    void draw_player()
    {
        double model_width = bitmap_width(player_model);
        double model_height = bitmap_height(player_model);
        double scaling = model_scaling;

        // fixing bitmap scaling position
        double pos_x = position.x + (((model_width * scaling) - model_width) / 2);
        double pos_y = position.y + (((model_height * scaling) - model_height) / 2);

        // flip when facing opposite direction
        if (model_facing_right)
        {
            draw_bitmap(player_model, pos_x, pos_y, option_scale_bmp(model_scaling, model_scaling));
        }
        else
        {
            draw_bitmap(player_model, pos_x, pos_y, option_flip_y(option_scale_bmp(model_scaling, model_scaling)));
        }

        draw_rectangle(color_red(), hurtbox.x, hurtbox.y, hurtbox.width, hurtbox.height); // FIXME: hide hurtbox
        draw_rectangle(color_red(), hitbox.x, hitbox.y, hitbox.width, hitbox.height);     // FIXME: hide hitbox
    }

    // draw player's sword onto screen
    void draw_sword()
    {
        double sword_model_width = bitmap_width(sword.sword_draw_model);
        double sword_model_height = bitmap_height(sword.sword_draw_model);
        double scaling = sword.model_scaling;

        double player_model_height = bitmap_height(player_model);

        // fixing bitmap scaling position
        double pos_x = sword.position.x + (((sword_model_width * scaling) - sword_model_width) / 2);
        double pos_y = sword.position.y + (((sword_model_height * scaling) - sword_model_height) / 2);

        sword_phase model = sword.phase;

        if (model_facing_right)
        {
            if (model == SWORD_DRAW)
                draw_bitmap(sword.sword_draw_model, pos_x, pos_y, option_scale_bmp(scaling, scaling));
            if (model == SWORD_SWING)
                draw_bitmap(sword.sword_swing_model, pos_x, pos_y - (player_model_height / 2 * model_scaling), option_scale_bmp(scaling, scaling));
        }
        else
        {
            if (model == SWORD_DRAW)
                draw_bitmap(sword.sword_draw_model, pos_x, pos_y, option_flip_y(option_scale_bmp(scaling, scaling)));
            if (model == SWORD_SWING)
                draw_bitmap(sword.sword_swing_model, pos_x, pos_y - (player_model_height / 2 * model_scaling), option_flip_y(option_scale_bmp(scaling, scaling)));
        }
    }
};

// function to calculate and change position of player
void move_player(player_data &player, double delta_time)
{
    // calculating distance using delta_time to avoid game lag issues
    double distance = player.get_speed() * delta_time;

    if (key_down(W_KEY))
    {
        coordinate new_position(player.get_position().x, player.get_position().y - distance);
        player.set_position(new_position);
    }
    if (key_down(S_KEY))
    {
        coordinate new_position(player.get_position().x, player.get_position().y + distance);
        player.set_position(new_position);
    }
    if (key_down(A_KEY))
    {
        coordinate new_position(player.get_position().x - distance, player.get_position().y);
        player.set_position(new_position);
        player.set_is_facing_right(false);
    }
    if (key_down(D_KEY))
    {
        coordinate new_position(player.get_position().x + distance, player.get_position().y);
        player.set_position(new_position);
        player.set_is_facing_right(true);
    }
}

// controls timing and cooldown of player attack, sets player.is_attacking to true
void player_attack(player_data &player)
{
    if ((key_down(SPACE_KEY) || mouse_clicked(LEFT_BUTTON)))
    {
        player.attack();
    }
}

void slow_time(game_timing_data &game_timing)
{
    if (key_down(LEFT_SHIFT_KEY) || mouse_down(RIGHT_BUTTON))
        game_timing.set_time_rate(0.5);
    else
        game_timing.set_time_rate(1);
}

// function to control character
void control_player(player_data &player, game_timing_data &game_timing)
{
    move_player(player, game_timing.get_delta_time());
    player_attack(player);
    slow_time(game_timing);
}

int main()
{
    open_window("Find The Imposter.exe", get_screen_width(), SCREEN_RESOLUTION);

    game_timing_data game_timing;
    room_data room(coordinate((ROOM_WIDTH - 1) / 2, (ROOM_HEIGHT - 1) - 2)); // -2 because one for the wall, one for the leg (player coord is at the head));
    player_data player(get_tile_size(), room.get_spawn_coords());

    while (!quit_requested())
    {
        // setting game timing
        game_timing.update_timing();

        clear_screen(color_white());

        // TODO: HERE IS THE CAMERA FUNCTION
        // point_2d pt = {-50, 500};
        // set_camera_position(pt);

        room.draw();
        player.draw_player();
        player.update(game_timing);

        control_player(player, game_timing);

        write_line(std::to_string(player.get_attack_cool_down()));

        refresh_screen();
        process_events();
    }

    return 0;
}
