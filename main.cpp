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

struct tile_data
{
    color tile_color;
    bool passable = true;
    int size = get_tile_size();
};

struct flooring_data
{
    tile_data floor_array[ROOM_HEIGHT][ROOM_WIDTH];
    // color color_pattern[2]; // TODO: if this is not needed to be used, remove it because its already it a functioin for each npc room
    int size_y = ROOM_HEIGHT;
    int size_x = ROOM_WIDTH;
};

struct coordinate
{
    int x, y;
};

struct npc_room_data
{
    flooring_data floor;
    int npc_array[MAX_NPC_PER_ROOM] = {}; // TODO: change the type to whatever the npcs are
    int npc_size = 0;                     // TODO: remove the declaration for size and array
};

struct player_data
{
    int health;
    rectangle hitbox;
    bitmap player_idle = load_bitmap("player_idle", "./image_data/player_data/player_idle.png");
    int size = get_tile_size();
};

void build_npc_room(npc_room_data &room)
{
    color slate_grey = rgb_color(112, 128, 144);
    color light_slate_grey = rgb_color(132, 144, 153);

    // npc_room.floor.color_pattern[0] = softBlue;
    // npc_room.floor.color_pattern[1] = paleYellow; //TODO: remove if not needed

    int size_y = room.floor.size_y;
    int size_x = room.floor.size_x;

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
                room.floor.floor_array[y][x].tile_color = first_color;
            else
                room.floor.floor_array[y][x].tile_color = second_color;
        }
    }
}

void draw_tile(const flooring_data &floor, const coordinate &coord)
{
    int x = coord.x;
    int y = coord.y;

    const tile_data &tile = floor.floor_array[y][x];
    color tile_color = tile.tile_color;
    int pos_x = get_tile_size() * x;
    int pos_y = get_tile_size() * y;
    int size = tile.size;
    fill_rectangle(tile_color, pos_x, pos_y, size, size);
}

void draw_floor(const flooring_data &floor)
{
    for (int y = 0; y < floor.size_y; y++)
    {
        for (int x = 0; x < floor.size_x; x++)
        {
            coordinate tile_coorindate = {x, y};
            draw_tile(floor, tile_coorindate);
        }
    }
}

void draw_npc_room(const npc_room_data &room)
{
    draw_floor(room.floor);
}

void draw_player(const player_data &player)
{
}

int main()
{
    open_window("YOU.exe", get_screen_width(), SCREEN_RESOLUTION);

    npc_room_data npc_room;

    clear_screen(color_white());

    build_npc_room(npc_room);
    draw_npc_room(npc_room);

    // bitmap player_bitmap = load_bitmap("player_idle", "player_idle.png");

    // draw_bitmap(player_bitmap, 50, 50, option_scale_bmp(4, 4)); // Use player_bitmap directly

    refresh_screen();

    while (!quit_requested())
    {
        process_events();
    }

    return 0;
}