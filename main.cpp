#include "splashkit.h"

const int SCREEN_RESOLUTION = 720;

const int ROOM_WIDTH = 32;  // aka the column
const int ROOM_HEIGHT = 24; // aka the row
// the ratio of the room is 16:9

const int MAX_NPC_PER_ROOM = 5;

int get_tile_size()
{
    return SCREEN_RESOLUTION / ROOM_HEIGHT;
}

int get_screen_width()
{
    double width = (double)SCREEN_RESOLUTION * ((double)ROOM_WIDTH / (double)ROOM_HEIGHT);
    return (int)width;
}

// floor tile struct
struct tile_data
{
    color tile_color;
    bool passable = true;
    int size = get_tile_size();
};

struct coordinate
{
    double x, y;
};

// struct for different rooms
struct room_properties_data
{
    bool is_npc_room = false;
    bool is_boss_room = false;
};

// flooring struct, using tiles as arrays
struct room_data
{
    tile_data floor_array[ROOM_HEIGHT][ROOM_WIDTH];
    color color_pattern[3]; // 0 and 1 is the floor checkers color pattern, 2 is the walls
    int size_y = ROOM_HEIGHT;
    int size_x = ROOM_WIDTH;
    coordinate spawn_coords = {(ROOM_WIDTH - 1) / 2, (ROOM_HEIGHT - 1) - 2}; // -2 because one for the wall, one for the leg (player coord is at the head)
    room_properties_data properties;
};

// player info struct
struct player_data
{
    int health;
    int speed;
    int attack_speed;

    coordinate position;
    rectangle hurtbox;
    bitmap player_model;
    bool model_facing_right = true;
    double model_scaling;

    bool is_attacking = false;
    rectangle hitbox;
};

// sword info stuct
struct sword_data
{
    bitmap sword_model_1;
    bitmap sword_model_2;
    double model_scaling;
    coordinate position;
};

enum sword_phase
{
    SWORD_DRAW = 0,
    SWORD_SWING,
};

// coordinates are from the flooring and tiles, change floor tiling coordinates to pixel/screen coorindates
double coordinates_to_pixel(double coord)
{
    return coord * get_tile_size();
}

// setting values for members in room_data struct
void build_room(room_data &room)
{
    int size_y = room.size_y;
    int size_x = room.size_x;

    for (int y = 0; y < size_y; y++)
    {
        for (int x = 0; x < size_x; x++)
        {

            // making the walls of the room, they cannot be passed
            if (x == 0 || x == size_x - 1 || y == 0 || y == size_y - 1)
            {
                room.floor_array[y][x].tile_color = room.color_pattern[2];
                room.floor_array[y][x].passable = false;
                continue;
            }

            // making the floor as a checked pattern
            color first_color = room.color_pattern[0];
            color second_color = room.color_pattern[1];
            if (y % 2 != 0)
            {
                first_color = room.color_pattern[1];
                second_color = room.color_pattern[0];
            }

            if (x % 2 == 0)
                room.floor_array[y][x].tile_color = first_color;
            else
                room.floor_array[y][x].tile_color = second_color;
        }
    }
}

// TODO: have it for each type of room
void build_npc_room(room_data &room)
{
    color slate_grey = rgb_color(112, 128, 144);
    color light_slate_grey = rgb_color(132, 144, 153);
    color light_steel_blue = rgb_color(150, 170, 200);

    room.color_pattern[0] = slate_grey;
    room.color_pattern[1] = light_slate_grey;
    room.color_pattern[2] = light_steel_blue;

    build_room(room);
    room.properties.is_npc_room = true;
}

// draw each tile of the floor on different coordinates as specified
void draw_tile(const tile_data &tile, const coordinate &coord)
{
    double x = coord.x;
    double y = coord.y;

    color tile_color = tile.tile_color;
    int pos_x = get_tile_size() * x;
    int pos_y = get_tile_size() * y;
    int size = tile.size;
    fill_rectangle(tile_color, pos_x, pos_y, size, size);
}

// function to draw the checker floor of the
void draw_floor(const room_data &room)
{
    for (int y = 0; y < room.size_y; y++)
    {
        for (int x = 0; x < room.size_x; x++)
        {
            coordinate tile_coorindate = {(double)x, (double)y};
            draw_tile(room.floor_array[y][x], tile_coorindate);
        }
    }
}

// TODO: access properties to draw different things for rooms with different properties, and will access more functiosn to add the stuff for different rooms
void draw_room(const room_data &room)
{
    draw_floor(room);
}

// update the position of the player's hurtbox and hitbox
void update_player_box(player_data &player)
{
    double model_width = bitmap_width(player.player_model);
    double model_height = bitmap_height(player.player_model);
    player.hurtbox = {(double)player.position.x, (double)player.position.y, model_width * player.model_scaling, model_height * player.model_scaling};

    double hitbox_size;

    if (player.is_attacking)
        hitbox_size = get_tile_size() * 2;
    else
        hitbox_size = 0;

    if (player.model_facing_right)
        player.hitbox = {(double)player.position.x + (model_width * player.model_scaling), (double)player.position.y, hitbox_size, hitbox_size};
    else
        player.hitbox = {(double)player.position.x - hitbox_size, (double)player.position.y, hitbox_size, hitbox_size};
}

// load values into the player_data struct
void load_player(player_data &player, const room_data &room)
{
    player.player_model = load_bitmap("player_idle", "./image_data/player/player_idle.png");
    player.health = 10;                 // TODO: change
    player.speed = 5 * get_tile_size(); // tiles per second   // TODO: change

    double model_width = bitmap_width(player.player_model);
    double model_height = bitmap_height(player.player_model);

    if (model_width < model_height)
    {
        player.model_scaling = (double)get_tile_size() / model_width;
    }

    if (model_height < model_width)
    {
        player.model_scaling = (double)get_tile_size() / model_height;
    }

    player.position = {coordinates_to_pixel(room.spawn_coords.x), coordinates_to_pixel(room.spawn_coords.y)};
    update_player_box(player);
}

// draw player onto the screen
void draw_player(const player_data &player)
{
    double model_width = bitmap_width(player.player_model);
    double model_height = bitmap_height(player.player_model);
    double scaling = player.model_scaling;

    // fixing bitmap scaling position
    double pos_x = player.position.x + (((model_width * scaling) - model_width) / 2);
    double pos_y = player.position.y + (((model_height * scaling) - model_height) / 2);

    // flip when facing opposite direction
    if (player.model_facing_right)
    {
        draw_bitmap(player.player_model, pos_x, pos_y, option_scale_bmp(player.model_scaling, player.model_scaling));
    }
    else
    {
        draw_bitmap(player.player_model, pos_x, pos_y, option_flip_y(option_scale_bmp(player.model_scaling, player.model_scaling)));
    }

    const rectangle &hurtbox = player.hurtbox;
    const rectangle &hitbox = player.hitbox;

    draw_rectangle(color_red(), hurtbox.x, hurtbox.y, hurtbox.width, hurtbox.height); // TODO: hide hurtbox
    draw_rectangle(color_red(), hitbox.x, hitbox.y, hitbox.width, hitbox.height);     // TODO: hide hitbox
}

// function to update the sword position to align with the player
void update_sword_position(sword_data &sword, const player_data &player)
{
    double player_model_width = bitmap_width(player.player_model);
    double player_model_height = bitmap_height(player.player_model);

    double sword_model_width = bitmap_width(sword.sword_model_1);

    // making the sword align with the player
    if (player.model_facing_right)
    {
        sword.position.x = player.position.x + (player_model_width * player.model_scaling);
        sword.position.y = player.position.y + (player_model_height / 2 * player.model_scaling);
    }
    else
    {
        sword.position.x = player.position.x - (sword_model_width * sword.model_scaling);
        sword.position.y = player.position.y + (player_model_height / 2 * player.model_scaling);
    }
}

// load values into members of sword_data struct
void load_sword(sword_data &sword, const player_data &player)
{
    sword.sword_model_1 = load_bitmap("Sword draw", "./image_data/sword/sword_1.png");
    sword.sword_model_2 = load_bitmap("Sword attack", "./image_data/sword/sword_2.png");

    // both sword bitmap have the same size
    double sword_model_width = bitmap_width(sword.sword_model_1);
    double sword_model_height = bitmap_height(sword.sword_model_1);

    // flip when facing opposite direction
    if (sword_model_width < sword_model_height)
    {
        sword.model_scaling = (double)get_tile_size() / sword_model_width;
    }

    if (sword_model_height < sword_model_width)
    {
        sword.model_scaling = (double)get_tile_size() / sword_model_height;
    }

    update_sword_position(sword, player);
}

// draw sword onto the screen
void draw_sword(const sword_data &sword, const player_data &player, sword_phase model)
{
    double sword_model_width = bitmap_width(sword.sword_model_1);
    double sword_model_height = bitmap_height(sword.sword_model_1);
    double scaling = sword.model_scaling;

    double player_model_height = bitmap_height(player.player_model);

    // fixing bitmap scaling position
    double pos_x = sword.position.x + (((sword_model_width * scaling) - sword_model_width) / 2);
    double pos_y = sword.position.y + (((sword_model_height * scaling) - sword_model_height) / 2);

    if (player.model_facing_right)
    {
        if (model == SWORD_DRAW)
            draw_bitmap(sword.sword_model_1, pos_x, pos_y, option_scale_bmp(scaling, scaling));
        if (model == SWORD_SWING)
            draw_bitmap(sword.sword_model_2, pos_x, pos_y - (player_model_height / 2 * player.model_scaling), option_scale_bmp(scaling, scaling));
    }
    else
    {
        if (model == SWORD_DRAW)
            draw_bitmap(sword.sword_model_1, pos_x, pos_y, option_flip_y(option_scale_bmp(scaling, scaling)));
        if (model == SWORD_SWING)
            draw_bitmap(sword.sword_model_2, pos_x, pos_y - (player_model_height / 2 * player.model_scaling), option_flip_y(option_scale_bmp(scaling, scaling)));
    }

    // draw_rectangle(color_red(), sword.position.x, sword.position.y, sword_model_width * scaling, sword_model_height * scaling); // TODO: remove
}

// function to calculate and change position of player
void move_player(player_data &player, double delta_time)
{
    // calculating distance using delta_time to avoid game lag issues
    double distance = player.speed * delta_time;

    if (key_down(W_KEY))
    {
        player.position.y -= distance;
    }
    if (key_down(S_KEY))
    {
        player.position.y += distance;
    }
    if (key_down(A_KEY))
    {
        player.position.x -= distance;
        player.model_facing_right = false;
    }
    if (key_down(D_KEY))
    {
        player.position.x += distance;
        player.model_facing_right = true;
    }
}

void player_attack(player_data &player)
{
    if (key_down(SPACE_KEY) || mouse_clicked(LEFT_BUTTON))
        player.is_attacking = true;
    else
        player.is_attacking = false;
}

// function to control character
void control_player(player_data &player, int &last_updated_time)
{
    int current_time = current_ticks();
    double delta_time = (current_time - last_updated_time) / 1000.0; // Convert to seconds

    move_player(player, delta_time);
    player_attack(player);

    last_updated_time = current_time;
}

int main()
{
    open_window("YOU.exe", get_screen_width(), SCREEN_RESOLUTION);

    room_data npc_room;
    player_data player;
    sword_data sword;

    int last_updated_time = 0;

    clear_screen(color_white());

    build_npc_room(npc_room);
    load_player(player, npc_room);
    load_sword(sword, player);

    while (!quit_requested())
    {
        draw_room(npc_room);
        draw_player(player);

        update_sword_position(sword, player);

        control_player(player, last_updated_time);

        update_player_box(player);
        refresh_screen();
        process_events();
    }

    return 0;
}
