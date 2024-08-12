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
    coordinate position;
    rectangle hitbox;
    bitmap player_model;
    bool model_facing_right = true;
    double model_scaling;
};

double coordinates_to_pixel(double coord)
{
    return coord * get_tile_size();
}

void build_room(room_data &room)
{
    int size_y = room.size_y;
    int size_x = room.size_x;

    for (int y = 0; y < size_y; y++)
    {
        for (int x = 0; x < size_x; x++)
        {

            if (x == 0 || x == size_x - 1 || y == 0 || y == size_y - 1)
            {
                room.floor_array[y][x].tile_color = room.color_pattern[2];
                room.floor_array[y][x].passable = false;
                continue;
            }

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

void draw_tile(const room_data &room, const coordinate &coord)
{
    double x = coord.x;
    double y = coord.y;

    const tile_data &tile = room.floor_array[(int)y][(int)x];
    color tile_color = tile.tile_color;
    int pos_x = get_tile_size() * x;
    int pos_y = get_tile_size() * y;
    int size = tile.size;
    fill_rectangle(tile_color, pos_x, pos_y, size, size);
}

void draw_floor(const room_data &room)
{
    for (int y = 0; y < room.size_y; y++)
    {
        for (int x = 0; x < room.size_x; x++)
        {
            coordinate tile_coorindate = {(double)x, (double)y};
            draw_tile(room, tile_coorindate);
        }
    }
}

// TODO: access properties to draw different things for rooms with different properties, and will access more functiosn to add the stuff for different rooms
void draw_room(const room_data &room)
{
    draw_floor(room);
}

void update_player_hitbox(player_data &player)
{
    double model_width = bitmap_width(player.player_model);
    double model_height = bitmap_height(player.player_model);
    player.hitbox = {(double)player.position.x, (double)player.position.y, model_width * player.model_scaling, model_height * player.model_scaling};
}

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
    update_player_hitbox(player);
}

void draw_player(const player_data &player)
{
    double model_width = bitmap_width(player.player_model);
    double model_height = bitmap_height(player.player_model);
    double scaling = player.model_scaling;

    double pos_x = player.position.x + (((model_width * scaling) - model_width) / 2);
    double pos_y = player.position.y + (((model_height * scaling) - model_height) / 2);
    if (player.model_facing_right)
    {
        draw_bitmap(player.player_model, pos_x, pos_y, option_scale_bmp(player.model_scaling, player.model_scaling));
    }
    else
    {
        draw_bitmap(player.player_model, pos_x, pos_y, option_flip_y(option_scale_bmp(player.model_scaling, player.model_scaling)));
    }

    const rectangle &hitbox = player.hitbox;

    draw_rectangle(color_red(), hitbox.x, hitbox.y, hitbox.width, hitbox.height); // TODO: hide hitbox
}

void move_player(player_data &player, double delta_time)
{
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

void control_player(player_data &player, int &last_updated_time)
{
    int current_time = current_ticks();
    double delta_time = (current_time - last_updated_time) / 1000.0; // Convert to seconds

    move_player(player, delta_time);

    last_updated_time = current_time;
}

int main()
{
    open_window("YOU.exe", get_screen_width(), SCREEN_RESOLUTION);

    room_data npc_room;
    player_data player;

    int last_updated_time = 0;

    clear_screen(color_white());

    build_npc_room(npc_room);
    load_player(player, npc_room);

    while (!quit_requested())
    {

        draw_room(npc_room);
        draw_player(player);

        control_player(player, last_updated_time);

        update_player_hitbox(player);
        refresh_screen();
        process_events();
    }

    return 0;
}
