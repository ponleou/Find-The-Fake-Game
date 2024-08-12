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
    int x, y;
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
    // color color_pattern[2]; // TODO: if this is not needed to be used, remove it because its already it a functioin for each npc room
    int size_y = ROOM_HEIGHT;
    int size_x = ROOM_WIDTH;
    coordinate spawn_coords = {ROOM_WIDTH / 2, ROOM_HEIGHT - 1};
    room_properties_data properties;
};

// player info struct
struct player_data
{
    int health;
    rectangle hitbox;
    bitmap player_model;
    double model_scaling;
    coordinate coords;
};

int coordinates_to_pixel(int coord)
{
    return coord * get_tile_size();
}

void build_room(room_data &room)
{
    color slate_grey = rgb_color(112, 128, 144);
    color light_slate_grey = rgb_color(132, 144, 153);

    // npc_room.floor.color_pattern[0] = softBlue;
    // npc_room.floor.color_pattern[1] = paleYellow; //TODO: remove if not needed

    int size_y = room.size_y;
    int size_x = room.size_x;

    for (int y = 0; y < size_y; y++)
    {
        for (int x = 0; x < size_x; x++)
        {
            color first_color = slate_grey;
            color second_color = light_slate_grey;
            if (y % 2 != 0)
            {
                first_color = light_slate_grey;
                second_color = slate_grey;
            }

            if (x % 2 == 0)
                room.floor_array[y][x].tile_color = first_color;
            else
                room.floor_array[y][x].tile_color = second_color;
        }
    }
}

void build_npc_room(room_data &room)
{
    build_room(room);
    room.properties.is_npc_room = true;
}

void draw_tile(const room_data &room, const coordinate &coord)
{
    int x = coord.x;
    int y = coord.y;

    const tile_data &tile = room.floor_array[y][x];
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
            coordinate tile_coorindate = {x, y};
            draw_tile(room, tile_coorindate);
        }
    }
}

// TODO: access properties to draw different things for rooms with different properties (wull probablt)
void draw_room(const room_data &room)
{
    draw_floor(room);
}

void load_player(player_data &player, const room_data &room)
{
    player.player_model = load_bitmap("player_idle", "./image_data/player_data/player_idle.png");
    player.health = 10; // TODO: change

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

    player.coords = room.spawn_coords;
}

void draw_player(const player_data &player)
{
    int pos_x = coordinates_to_pixel(player.coords.x);
    int pos_y = coordinates_to_pixel(player.coords.y);
    draw_bitmap(player.player_model, pos_x, pos_y, option_scale_bmp(player.model_scaling, player.model_scaling));
}

int main()
{
    open_window("YOU.exe", get_screen_width(), SCREEN_RESOLUTION);

    room_data npc_room;
    player_data player;

    clear_screen(color_white());

    build_npc_room(npc_room);
    draw_room(npc_room);
    load_player(player, npc_room);
    draw_player(player);

    // bitmap player_bitmap = load_bitmap("player_idle", "player_idle.png");

    // draw_bitmap(player_bitmap, 50, 50, option_scale_bmp(4, 4)); // Use player_bitmap directly

    refresh_screen();

    while (!quit_requested())
    {
        process_events();
    }

    return 0;
}

// TODO: get rid of npc_room_data, make room struct the top in struct heirachy (maybbe add a room_properties struct in all rooms)