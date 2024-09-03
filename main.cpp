#include "splashkit.h"
#include <iostream>
#include <cmath>
#include <vector>

// increase the value of the input x by an ease out quint, until it reaches 1
double ease_out_quint(double x)
{
    return 1 - std::pow(1 - x, 5);
}

// make a game size struct
class game_size_data
{
private:
    int screen_height;
    int room_width;    // the number of tiles in the room on the x axis
    int room_height;   // the number of tiles in the room on the y axis
    double zoom_level; // the zoom level of the game or screen

public:
    // Constructor
    game_size_data(int screen_height, int room_width, int room_height)
    {
        this->screen_height = screen_height;
        this->room_width = room_width;
        this->room_height = room_height;
        this->zoom_level = 1;
    }

    game_size_data() : game_size_data(1080, 40, 30) {}

    // getters and setters
    int get_screen_height() const
    {
        return screen_height;
    }

    int get_room_width() const
    {
        return room_width;
    }

    int get_room_height() const
    {
        return room_height;
    }

    double get_zoom_level() const
    {
        return zoom_level;
    }

    void set_zoom_level(double zoom_level)
    {
        this->zoom_level = zoom_level;
    }

    // screen width is calculated using the screen height and the room width and height
    int get_screen_width() const
    {
        double width = (double)screen_height * ((double)room_width / (double)room_height);
        return (int)width;
    }
};

// data type for coorindates, can be used for both pixel and tile coordinates
// tile coordinates are the coordinates of the tiles in the room (room_data)
struct coordinate
{
    double x, y;

    // to convert tile coorindates to pixel coordinates
    coordinate tile_to_pixel(double tile_size)
    {
        return {x * tile_size, y * tile_size};
    }

    // to convert pixel coordinates to tile coordinates of the room
    coordinate pixel_to_tile(double tile_size)
    {
        double pixel_x = floor(x / tile_size);
        double pixel_y = floor(y / tile_size);
        return {pixel_x, pixel_y};
    }
};

// generate random coordinates
coordinate random_coordinate(const coordinate &max_coords)
{
    return {(double)rnd(max_coords.x), (double)rnd(max_coords.y)};
}

coordinate random_coordinate(const coordinate &min_coords, const coordinate &max_coords)
{
    return {(double)rnd(min_coords.x, max_coords.x), (double)rnd(min_coords.y, max_coords.y)};
}

// class to hold the timing data of the game (mostly for the delta time)
class game_timing_data
{
private:
    double delta_time;             // time between the last frame and the current frame
    double last_update_time;       // the time of the last frame
    double time_rate;              // how many seconds the game should load in one second
    int frame_rate;                // the frame rate of the game
    double last_frame_update_time; // the time of the last frame update

    // get the time it takes for each frame to be displayed to fit frame rate
    double get_frame_delay() const
    {
        return 1000 / frame_rate;
    }

public:
    // Constructor
    game_timing_data()
    {
        last_update_time = 0;
        time_rate = 1;
        frame_rate = 60;
    }

    game_timing_data(int frame_rate)
    {
        last_update_time = 0;
        time_rate = 1;
        this->frame_rate = frame_rate;
    }

    // must be ran inside a game loop in order to update the delta time
    void update_timing()
    {
        delta_time = (double)current_ticks() - (double)last_update_time; // getting delta_time
        last_update_time = (double)current_ticks();                      // setting the last update time
        last_frame_update_time += delta_time;                            // adding the delta time to the last frame update time
        delta_time *= time_rate;                                         // changing the delta time according to the time rate
    }

    // setters and getters
    double get_delta_time() const
    {
        return delta_time;
    }

    // set time rate, changes the time rate of the delta time
    void set_time_rate(double rate)
    {
        time_rate = rate;
    }

    // set frame rate, changes the frame rate of the game, tells when the game should update the frame (calling screen_refresh)
    bool update_frame()
    {
        if (last_frame_update_time >= get_frame_delay())
        {
            last_frame_update_time = 0;
            return true;
        }
        return false;
        // true if the game is ready to update the frame, false if not
    }
};

// TODO: for the zoom camera: tile_data must have its own position, position set when creating floor array
// floor tile struct
struct tile_data
{
    color tile_color;
    bool passable;
    int size;
    rectangle tile;
};

// flooring struct, using tiles as arrays
class room_data
{
private:
    vector<vector<tile_data>> floor_array; // the floor of the room

    // contains the start and end tile coordinates of the walls
    // elements are coordinate vectors of two elements, the first element is the start tile, the second element is the end tiles
    vector<vector<coordinate>> walls_coords_vector;

    vector<rectangle> walls_vector; // rectangles of the walls, can work as hitboxes of walls
    color color_pattern[3];         // 0 and 1 is the floor checkers color pattern, 2 is the walls
    int size_y;                     // room height
    int size_x;                     // room width
    double tile_size;               // size of each tile
    coordinate spawn_coords;

    double zoom_level;       // zoom level of the room
    double zoomed_tile_size; // size of each tile after zooming

    // a function to construct the room, used in the constructor
    void construct_room(int room_width, int room_height, int screen_height, const color &floor_color_1, const color &floor_color_2, const color &wall_color, const coordinate &spawn_tile)
    {
        color_pattern[0] = floor_color_1;
        color_pattern[1] = floor_color_2;
        color_pattern[2] = wall_color;
        this->size_x = room_width;
        this->size_y = room_height;
        this->zoom_level = 1;
        this->tile_size = (double)(screen_height / room_height);
        this->spawn_coords = spawn_tile;

        // update the zoomed tile size (using tile_size and zoom_level)
        update_zoomed_tile_size();

        // setting spawn coordinates, it is passed as a tile coordinate
        this->spawn_coords = this->spawn_coords.tile_to_pixel(zoomed_tile_size);

        // setting the wall (surronding the room)
        set_wall({0, 0}, {(double)(size_x - 1), 0});                                       // top wall
        set_wall({0, 1}, {0, (double)(size_y - 2)});                                       // left wall
        set_wall({(double)(size_x - 1), 1}, {(double)(size_x - 1), (double)(size_y - 2)}); // right wall
        set_wall({0, (double)(size_y - 1)}, {(double)(size_x - 1), (double)(size_y - 1)}); // bottom wall

        // initializing the floor 2d vector
        floor_array = vector<vector<tile_data>>(size_y, vector<tile_data>(size_x));

        build_room();
    }

    // function to update the zoomed tile size (depending if zoom level is changed)
    void update_zoomed_tile_size()
    {
        zoomed_tile_size = this->tile_size * zoom_level;
    }

    // build the floor of the room (setting floor_array with tiles)
    void build_floor()
    {
        for (int y = 0; y < size_y; y++)
        {
            for (int x = 0; x < size_x; x++)
            {
                floor_array[y][x].passable = true;
                floor_array[y][x].size = zoomed_tile_size;

                coordinate tile_coords = {(double)x, (double)y};
                coordinate coords = tile_coords.tile_to_pixel(zoomed_tile_size);

                // setting the tile's position and size (rectangle objects)
                floor_array[y][x].tile = {coords.x, coords.y, zoomed_tile_size, zoomed_tile_size};

                // making the floor as a checked pattern (setting colors for each tile)
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

    // set the walls of the room, updating the floor_array with the walls
    void build_wall()
    {
        // creating the walls vector (rectangles) from the walls coordinates vector
        walls_vector.clear(); // clearing the walls vector to update it with the new walls (walls can change depending on zoom_level)
        for (int i = 0; i < walls_coords_vector.size(); i++)
        {
            // walls_coords_vector contains vectors contains a 2-element array with the start and end tile coordinates of the walls
            coordinate wall_coords_start = walls_coords_vector[i][0].tile_to_pixel(zoomed_tile_size);
            coordinate wall_coords_end = walls_coords_vector[i][1].tile_to_pixel(zoomed_tile_size);

            // adding the size of the tile to the end tile to get the bottom right corner of the wall
            wall_coords_end.x += zoomed_tile_size;
            wall_coords_end.y += zoomed_tile_size;

            // calculate the width and height of the wall
            double wall_width = wall_coords_end.x - wall_coords_start.x;
            double wall_height = wall_coords_end.y - wall_coords_start.y;

            // add the wall to the walls vector (a rectangle object)
            walls_vector.push_back({wall_coords_start.x, wall_coords_start.y, wall_width, wall_height});
            // the rectangle is from the top left corner of the start tile to the bottom right corner of the end tile
        }

        // using the walls_vector to update the floor_array with the walls
        for (int y = 0; y < size_y; y++)
        {
            for (int x = 0; x < size_x; x++)
            {
                for (int i = 0; i < walls_vector.size(); i++)
                {
                    // any tiles in the floor_array that intersects with the walls_vector will be set as a wall
                    if (rectangles_intersect(floor_array[y][x].tile, walls_vector[i]))
                    {

                        // Splashkit's rectangle_intersection counds touching as a collision, we ignore touching as collision
                        rectangle intersection_box = intersection(floor_array[y][x].tile, walls_vector[i]);
                        if (intersection_box.width > 0 && intersection_box.height > 0)
                        {
                            // tiles are walls if they are a wall color (color_pattern[2]) and are not passable
                            floor_array[y][x].tile_color = color_pattern[2];
                            floor_array[y][x].passable = false;
                            break;
                        }
                    }
                }
            }
        }
    }

public:
    // FIXME: each tile should also record its pixel coordinate // FIXME: wait why?
    // Constructor
    room_data(int floor_width, int floor_height, double screen_height)
    {
        // setting room color
        color slate_grey = rgb_color(112, 128, 144);
        color light_slate_grey = rgb_color(132, 144, 153);
        color light_steel_blue = rgb_color(150, 170, 200);

        construct_room(floor_width, floor_height, screen_height, slate_grey, light_slate_grey, light_steel_blue, {(double)(floor_width - 1) / 2, (double)(floor_height - 1) - 2});
    }

    room_data(int floor_width, int floor_height, double screen_height, const coordinate &spawn_coords)
    {
        // setting room color
        color slate_grey = rgb_color(112, 128, 144);
        color light_slate_grey = rgb_color(132, 144, 153);
        color light_steel_blue = rgb_color(150, 170, 200);

        construct_room(floor_width, floor_height, tile_size, slate_grey, light_slate_grey, light_steel_blue, spawn_coords);
    }

    room_data(int floor_width, int floor_height, double screen_height, const color &floor_color_1, const color &floor_color_2, const color &wall_color, const coordinate &spawn_coords)
    {
        construct_room(floor_width, floor_height, screen_height, floor_color_1, floor_color_2, wall_color, spawn_coords);
    }

    // building the room, setting the floor and walls, used when new walls are created (must be called in game loop to watch for changes in zoom_level)
    void build_room()
    {
        // changing the zoomed_tile_size with updated zoom_level
        update_zoomed_tile_size();

        // building the floor
        build_floor();

        // building the walls
        build_wall();
    }

    // draw the room onto the screen
    void draw() const
    {
        for (int y = 0; y < size_y; y++)
        {
            for (int x = 0; x < size_x; x++)
            {
                tile_data tile = floor_array[y][x];

                fill_rectangle(tile.tile_color, tile.tile);
            }
        }

        // FIXME: hide walls
        for (int i = 0; i < walls_vector.size(); i++)
        {
            draw_rectangle(color_red(), walls_vector[i]);
        }
    }

    // check if a tile coords is passable
    bool is_passable(const coordinate &tile_coords) const
    {
        return floor_array[(int)tile_coords.y][(int)tile_coords.x].passable;
    }

    // getters and setters
    const coordinate &get_spawn_coords() const
    {
        return spawn_coords;
    }

    int get_size_x() const
    {
        return size_x;
    }

    int get_size_y() const
    {
        return size_y;
    }

    double get_zoomed_tile_size() const
    {
        return zoomed_tile_size;
    }

    const vector<rectangle> &get_walls_vector() const
    {
        return walls_vector;
    }

    // sets the wall of the room, using a start and end tile coordinate
    //(the wall is the rectangle from the top left corner of start tile to the bottom right corner of end tile)
    void set_wall(const coordinate &start_tile, const coordinate &end_tile)
    {
        coordinate wall_coords_start = start_tile;

        // tile_to_pixel returns the top left corner of the tile, so we need to add the size of the tile to get the bottom right corner
        coordinate wall_coords_end = end_tile;
        vector<coordinate> wall_coords_vector = {wall_coords_start, wall_coords_end};
        walls_coords_vector.push_back(wall_coords_vector);
    }

    void set_zoom_level(double zoom_level)
    {
        this->zoom_level = zoom_level;
    }
};

class character_data
{
private:
    int health;
    double speed; // pixels per second

    rectangle hurtbox; // the box that determines the player's collision
    bitmap character_model;
    bool model_facing_right; // models are drawn facing right, this is used to determine if the model should be flipped
    double model_scaling;    // scaling of the model, character model is scaled by this value (character model is made at 5x10 pixels)

    double zoom_level;           // zoom level of the screen to adjust the size of the model and posiion of the model
    double zoomed_model_scaling; // model scaling after zooming

    coordinate position; // position of the character in correlation to the screen (non-zoomed)

    void update_zoomed_position()
    {
        zoomed_position = {position.x * zoom_level, position.y * zoom_level};
        write_line("zoomed_position: " + std::to_string(zoomed_position.x) + ", " + std::to_string(zoomed_position.y));
        // zoomed_position = position; // FIXME: zoomed_position is not used
    }

protected:
    coordinate zoomed_position; // position of the character in correlation to the room (after zooming), only used for drawing

    character_data(int health, double speed, const bitmap &model, bool model_facing_right, double model_size, const coordinate &spawn_coords)
    {
        // setting player
        character_model = model;
        this->model_facing_right = model_facing_right; // depending on the drawn model, the player might be facing right or left
        this->health = health;
        this->speed = speed; // pixels per milisecond
        zoom_level = 1;

        // setting the model size, calculated by the smallest side of the model
        set_model_size(model_size);

        position = {spawn_coords.x, spawn_coords.y};

        // setting the zoomed model scaling, hurtbox, and zoomed position
        update_zoomed_position();
        update_zoomed_model_scaling();
        update_hurtbox();
    }

    // update the position of the hurtbox as the character moves (align with character's position)
    void update_hurtbox()
    {
        double model_width = bitmap_width(character_model);
        double model_height = bitmap_height(character_model);
        hurtbox = {zoomed_position.x, zoomed_position.y, model_width * zoomed_model_scaling, model_height * zoomed_model_scaling};
    }

    // update the zoomed model scaling (model scaling after zooming)
    void update_zoomed_model_scaling()
    {
        zoomed_model_scaling = model_scaling * zoom_level;
    }

    // getters and setters

    // set the size of the model (scaling of the model)
    void set_model_size(double model_size)
    {
        double model_width = bitmap_width(character_model);
        double model_height = bitmap_height(character_model);

        // determining the scaling of the model, the smallest side will be scaled to the model_size
        if (model_width < model_height)
        {
            model_scaling = model_size / model_width;
        }
        else
        {
            model_scaling = model_size / model_height;
        }
    }

    // return the model (bitmap) of the character
    const bitmap &get_model() const
    {
        return character_model;
    }

    // get the direction the character is facing (direction of the model)
    bool get_is_facing_right() const
    {
        return model_facing_right;
    }

    // get the scaling of the model
    double get_zoomed_model_scaling() const
    {
        return zoomed_model_scaling;
    }

    const rectangle &get_hurtbox() const
    {
        return hurtbox;
    }

    double get_zoom_level() const
    {
        return zoom_level;
    }

public:
    // update the character's position and hurtbox, should always be ran inside the game loop
    void update()
    {
        update_zoomed_position();
        update_zoomed_model_scaling();
        update_hurtbox();
    }

    // move the character in a direction by a distance (overriden by npc_data for better wall walking collision)
    virtual void move(vector_2d &direction, double distance, const room_data &room)
    {
        if (direction.x != 0 || direction.y != 0)
        {
            direction = unit_vector(direction); // convert to unit vector
        }

        vector_2d movement = vector_multiply(direction, distance);
        coordinate new_position = {position.x + movement.x, position.y + movement.y};
        set_position(new_position);

        // array with all the walls in the room
        const vector<rectangle> &walls_vector = room.get_walls_vector();

        // checking the collision of player hurtbox with each walls
        for (int i = 0; i < walls_vector.size(); i++)
        {

            // the box of the collision (intersection of the player hurtbox and the wall)
            rectangle collision_box = intersection(hurtbox, walls_vector[i]);
            if (collision_box.width != 0 && collision_box.height != 0) // if there is a collision
            {
                if (collision_box.height < collision_box.width)
                {

                    // checking if player moves by y + collision_box.height will still cause collision
                    hurtbox.y += collision_box.height;
                    if (intersection(hurtbox, walls_vector[i]).height == 0)
                    {
                        // if no collision, then move the player by y + collision_box.height
                        set_position({position.x, round(position.y + collision_box.height)});
                    }
                    // hurtbox_copy.y = new_position.y - collision_box.height;
                    else
                    {
                        // if collision, then move the player by y - collision_box.height
                        set_position({position.x, round(position.y - collision_box.height)});
                    }
                }

                if (collision_box.height > collision_box.width)
                {
                    // checking if player moves by x + collision_box.width will still cause collision
                    hurtbox.x += collision_box.width;
                    if (intersection(hurtbox, walls_vector[i]).width != 0)
                    {
                        // if no collision, then move the player by x + collision_box.width
                        set_position({round(position.x - collision_box.width), position.y});
                    }
                    // hurtbox_copy.x = new_position.x - collision_box.width;
                    else
                    {
                        // if collision, then move the player by x - collision_box.width
                        set_position({round(position.x + collision_box.width), position.y});
                    }
                }
            }
        }

        // set_position(new_position);
    }

    // draw the character onto the screen
    void draw() const
    {
        double model_width = bitmap_width(get_model());
        double model_height = bitmap_height(get_model());
        double zoomed_model_scaling = get_zoomed_model_scaling();

        // fixing bitmap scaling position
        double pos_x = get_position().x + (((model_width * zoomed_model_scaling) - model_width) / 2);
        double pos_y = get_position().y + (((model_height * zoomed_model_scaling) - model_height) / 2);

        // flip when facing opposite direction
        if (get_is_facing_right())
        {
            draw_bitmap(get_model(), pos_x, pos_y, option_scale_bmp(zoomed_model_scaling, zoomed_model_scaling));
        }
        else
        {
            draw_bitmap(get_model(), pos_x, pos_y, option_flip_y(option_scale_bmp(zoomed_model_scaling, zoomed_model_scaling)));
        }

        draw_rectangle(color_red(), get_hurtbox().x, get_hurtbox().y, get_hurtbox().width, get_hurtbox().height); // FIXME: hide hurtbox
    }

    int get_health() const
    {
        return health;
    }

    double get_speed() const
    {
        return speed;
    }

    // get position of the character in colleration to the room
    const coordinate &get_position() const
    {
        return zoomed_position;
    }

    void set_position(const coordinate &position)
    {
        this->position = position;
    }

    // set the direction the character is facing (true is right, false is left)
    void set_is_facing_right(bool facing_right)
    {
        model_facing_right = facing_right;
    }

    void set_zoom_level(double zoom_level)
    {
        this->zoom_level = zoom_level;
    }
};

// class for npcs in the game
class npc_data : public character_data
{
private:
    coordinate new_position;     // the position the npc is moving to
    int auto_move_max_distance;  // the range at which the new_position can be from its current position (in a square)
    int time_since_new_position; // the time since the npc has a new position
    int new_position_cooldown;   // the time the npc should have a new position

    // move the npc to a random position within a range automatically
    void auto_set_new_position(double delta_time, const room_data &room)
    {
        int new_position_x = (int)new_position.x;
        int new_position_y = (int)new_position.y;

        // determines if NPC is at the destination, allows an error of (+-)1 pixel due to the float to int conversion
        bool x_at_destination = ((int)zoomed_position.x >= new_position_x - 1) && ((int)zoomed_position.x <= new_position_x + 1);
        bool y_at_destination = ((int)zoomed_position.y >= new_position_y - 1) && ((int)zoomed_position.y <= new_position_y + 1);

        time_since_new_position += delta_time;
        bool cooldown_passed = time_since_new_position >= new_position_cooldown;

        // if the npc is at the destination or cooldown for new position passed, generate a new destination
        if ((x_at_destination && y_at_destination) || cooldown_passed)
        {
            coordinate min_coords = {get_position().x - auto_move_max_distance, get_position().y - auto_move_max_distance};
            coordinate max_coords = {get_position().x + auto_move_max_distance, get_position().y + auto_move_max_distance};

            do
            {
                // generating a random position for the npc to move to converting to tile coordinates
                new_position = random_coordinate(min_coords, max_coords).pixel_to_tile(room.get_zoomed_tile_size());
                // checking if the tile is within the room
                if (new_position.x < 0 || new_position.x >= room.get_size_x() || new_position.y < 0 || new_position.y >= room.get_size_y())
                {
                    continue;
                }

                // make sure the npc can fit inside the new position
                int player_tile_height = (int)ceil(get_hurtbox().height / room.get_zoomed_tile_size());
                int player_tile_width = (int)ceil(get_hurtbox().width / room.get_zoomed_tile_size());
                vector<coordinate> player_tiles;

                for (int i = 0; i < player_tile_height; i++)
                {
                    for (int j = 0; j < player_tile_width; j++)
                    {
                        player_tiles.push_back({new_position.x + j, new_position.y + i});
                    }
                }

                // checking for all tiles the npc will fill in the new position
                bool valid_position = true;

                for (int i = 0; i < player_tiles.size(); i++)
                {
                    if (!room.is_passable(player_tiles[i]))
                    {
                        valid_position = false;
                        break;
                    }
                }

                // checking if the npc can fit in the position
                if (valid_position)
                {
                    break;
                }
            } while (true);

            new_position = new_position.tile_to_pixel(room.get_zoomed_tile_size()); // converting back to pixel coordinates for the npc to move to
            time_since_new_position = 0;
        }
    }

    void auto_move(double delta_time, const room_data &room)
    {
        // calculate the direction and distance the npc should move
        vector_2d direction = {0, 0};
        direction.x = new_position.x - zoomed_position.x;
        direction.y = new_position.y - zoomed_position.y;

        // set the direction the npc is facing
        if (direction.x > 0)
        {
            set_is_facing_right(true);
        }
        else
        {
            set_is_facing_right(false);
        }

        // the distance the npc should move according to delta_time
        double distance = get_speed() * (double)delta_time;

        move(direction, distance, room);
    }

public:
    npc_data(double tile_size, double model_size, const coordinate &spawn_coords)
        : character_data(1, 3.75 * tile_size / 1000, load_bitmap("npc_idle", "./image_data/npc/npc_idle.png"), true, model_size, spawn_coords)
    {
        new_position = spawn_coords;
        auto_move_max_distance = 10 * tile_size;
        new_position_cooldown = 10000; // ms
        time_since_new_position = 0;
    }

    // update the npc's position and hurtbox, should always be ran inside the game loop
    void update(double delta_time, const room_data &room)
    {
        auto_set_new_position(delta_time, room);
        draw_rectangle(color_red(), new_position.x, new_position.y, room.get_zoomed_tile_size(), room.get_zoomed_tile_size()); // FIXME: hide new position

        auto_move(delta_time, room);

        // calls update from character_data base class, updates hitbox and model scaling
        character_data::update();
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

// player info struct, players can attack
class player_data : public character_data
{
private:
    int attack_speed; // ms
    bool is_attacking;
    bool create_hitbox;
    rectangle hitbox;
    int hitbox_lasting_time; // ms
    double attack_cooldown;
    bool can_attack;

    sword_data sword;
    double sword_zoomed_model_scaling;

    // main function for attacking, controls the attacking animation, and the time the hitbox is active (according to attack_speed)
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

    // update the attack cooldown, and the attack state
    void update_attack_cooldown(double delta_time)
    {
        attack_cooldown += delta_time;

        if (attack_cooldown >= attack_speed * 2)
        {
            can_attack = true;
            is_attacking = false;
            attack_cooldown = 0;
        }
        else
            can_attack = false;
    }

    // update the sword's hitbox to align with the player's position
    void update_hitbox()
    {
        double player_model_width = bitmap_width(get_model());
        double player_model_height = bitmap_height(get_model());

        double sword_model_width = bitmap_width(sword.sword_draw_model);
        double sword_model_height = bitmap_height(sword.sword_draw_model);
        double hitbox_size_x, hitbox_size_y;

        // player hitbox is 0 if there isnt an attack happening (which is when create_hitbox is false)
        if (create_hitbox)
        {
            hitbox_size_x = sword_model_width * sword_zoomed_model_scaling;
            hitbox_size_y = sword_model_height * sword_zoomed_model_scaling * (player_model_height / player_model_width);
            // playermodelheight / playermodelwidth because model_scaling is derived from player_model_width
        }
        else
        {
            hitbox_size_x = 0;
            hitbox_size_y = 0;
        }

        // align the sword's hitbox with the player's direction
        if (get_is_facing_right())
            hitbox = {zoomed_position.x + (player_model_width * get_zoomed_model_scaling()), zoomed_position.y, hitbox_size_x, hitbox_size_y};
        else
            hitbox = {zoomed_position.x - hitbox_size_x, zoomed_position.y, hitbox_size_x, hitbox_size_y};
    }

    void update_sword_zoomed_model_scaling()
    {
        sword_zoomed_model_scaling = sword.model_scaling * get_zoom_level();
    }

public:
    // Constructor
    player_data(double tile_size, double model_size, const coordinate &spawn_coords)
        : character_data(1, 5.0 * tile_size / 1000, load_bitmap("player_idle", "./image_data/player/player_idle.png"), true, model_size, spawn_coords)
    {
        attack_speed = 1000;       // ms
        hitbox_lasting_time = 100; // ms

        can_attack = true;
        attack_cooldown = 0;
        is_attacking = false;
        create_hitbox = false;

        double model_width = bitmap_width(get_model());
        double model_height = bitmap_height(get_model());

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

        update_sword_zoomed_model_scaling();
        // updating hitbox, hurtbox and model scaling
        character_data::update();
        update_hitbox();
        // calling update to set the sword's position
        update_sword();
    }

    player_data(double tile_size, double model_size)
        : player_data(tile_size, model_size, {0, 0}) {}

    // attack function to call the player to attack (only if the player can attack)
    void attack()
    {
        if (can_attack)
        {
            is_attacking = true;
        }
    }

    // update the sword's position to align with the player's position
    void update_sword()
    {
        update_sword_zoomed_model_scaling();

        double player_model_width = bitmap_width(get_model());
        double player_model_height = bitmap_height(get_model());

        double sword_model_width = bitmap_width(sword.sword_draw_model);

        double model_scaling = get_zoomed_model_scaling();

        // making the sword align with the player
        if (get_is_facing_right())
        {
            sword.position.x = zoomed_position.x + (player_model_width * model_scaling);
            sword.position.y = zoomed_position.y + (player_model_height / (player_model_height / player_model_width) * model_scaling);
        }
        else
        {
            sword.position.x = zoomed_position.x - (sword_model_width * sword_zoomed_model_scaling);
            sword.position.y = zoomed_position.y + (player_model_height / (player_model_height / player_model_width) * model_scaling);
        }
    }

    // must be ran inside game loop, and get delta_time from game_timing_data
    void update(double delta_time)
    {
        character_data::update();
        update_hitbox();
        update_sword();

        if (is_attacking)
        {
            update_attack_cooldown(delta_time);
            attacking();
        }

        draw_rectangle(color_red(), hitbox.x, hitbox.y, hitbox.width, hitbox.height); // FIXME: hide hitbox
    }

    double get_attack_cool_down() const
    {
        return attack_cooldown;
    }

    // draw player's sword onto screen
    void draw_sword() const
    {
        double sword_model_width = bitmap_width(sword.sword_draw_model);
        double sword_model_height = bitmap_height(sword.sword_draw_model);
        double scaling = sword_zoomed_model_scaling;

        double player_model_height = bitmap_height(get_model());
        double player_model_width = bitmap_width(get_model());

        // fixing bitmap scaling position
        double pos_x = sword.position.x + (((sword_model_width * scaling) - sword_model_width) / 2);
        double pos_y = sword.position.y + (((sword_model_height * scaling) - sword_model_height) / 2);

        sword_phase model = sword.phase;

        double model_scaling = get_zoomed_model_scaling();

        if (get_is_facing_right())
        {
            if (model == SWORD_DRAW)
                draw_bitmap(sword.sword_draw_model, pos_x, pos_y, option_scale_bmp(scaling, scaling));
            if (model == SWORD_SWING)
                draw_bitmap(sword.sword_swing_model, pos_x, pos_y - (player_model_height / (player_model_height / player_model_width) * model_scaling), option_scale_bmp(scaling, scaling));
        }
        else
        {
            if (model == SWORD_DRAW)
                draw_bitmap(sword.sword_draw_model, pos_x, pos_y, option_flip_y(option_scale_bmp(scaling, scaling)));
            if (model == SWORD_SWING)
                draw_bitmap(sword.sword_swing_model, pos_x, pos_y - (player_model_height / (player_model_height / player_model_width) * model_scaling), option_flip_y(option_scale_bmp(scaling, scaling)));
        }
    }
};

// function to calculate and change position of player
void move_player(player_data &player, double delta_time, const room_data &room)
{
    // calculating distance using delta_time to avoid game lag issues
    double distance = player.get_speed() * delta_time;

    // setting the direction of the movement
    vector_2d direction = {0, 0};

    if (key_down(W_KEY))
    {
        direction.y -= 1;
    }
    if (key_down(S_KEY))
    {
        direction.y += 1;
    }
    if (key_down(A_KEY))
    {
        direction.x -= 1;
        player.set_is_facing_right(false);
    }
    if (key_down(D_KEY))
    {
        direction.x += 1;
        player.set_is_facing_right(true);
    }

    // call move function in chacter_data to move along the direction and distance
    player.move(direction, distance, room);
}

// controls the player's attack, calls the player to attack if button is pressed
void player_attack(player_data &player)
{
    if ((key_down(SPACE_KEY) || mouse_clicked(LEFT_BUTTON)))
    {
        player.attack();
    }
}

// control to slow time, used for the focusing ability
void slow_time(game_timing_data &game_timing, game_size_data &game_size)
{
    if (key_down(LEFT_SHIFT_KEY) || mouse_down(RIGHT_BUTTON))
    {
        game_timing.set_time_rate(0.5); // FIXME: not correct, time rate is definitely not 0.5
        game_size.set_zoom_level(2);
    }
    else
    {
        game_timing.set_time_rate(1);
        game_size.set_zoom_level(1);
    }
}

// function to control character, must be called in the game loop
void control_player(player_data &player, game_timing_data &game_timing, game_size_data &game_size, const room_data &room)
{
    move_player(player, game_timing.get_delta_time(), room);
    player_attack(player);
    slow_time(game_timing, game_size);
}

int main()
{
    int number_loops = 0;
    int total_delta = 0;
    game_size_data game_size(1080, 40, 30);

    open_window("Find The Imposter.exe", game_size.get_screen_width(), game_size.get_screen_height());

    int npc_count = 5;

    game_timing_data game_timing(120);
    room_data room(40, 30, game_size.get_screen_height());
    double tile_size = room.get_zoomed_tile_size();

    player_data player(tile_size, tile_size, room.get_spawn_coords());

    // FIXME: delete, or implement arrays of npcs

    coordinate min_tile_coords = {1, 1};
    coordinate max_tile_coords = {(double)(game_size.get_room_width() - 2), (double)(game_size.get_room_height() - 2)};
    // npc_data npc(get_tile_size(), coordinate().random_coordinate(max_tile_coords.tile_to_pixel()));

    vector<npc_data *> npcs(npc_count);

    for (int i = 0; i < npc_count; i++)
    {
        npcs[i] = new npc_data(tile_size, tile_size, random_coordinate(min_tile_coords.tile_to_pixel(tile_size), max_tile_coords.tile_to_pixel(tile_size)));
    }

    room.set_wall({10, 10}, {20, 20}); // FIXME: delete test wall

    while (!quit_requested())
    {
        // setting game timing
        game_timing.update_timing();

        // TODO: HERE IS THE CAMERA FUNCTION
        // point_2d pt = {-50, 500};
        // set_camera_position(pt);

        clear_screen(COLOR_WHITE);
        room.set_zoom_level(game_size.get_zoom_level());
        room.build_room();
        room.draw();

        for (int i = 0; i < npc_count; i++)
        {
            npcs[i]->set_zoom_level(game_size.get_zoom_level());
            npcs[i]->update(game_timing.get_delta_time(), room);
            npcs[i]->draw();
        }

        player.set_zoom_level(game_size.get_zoom_level());
        player.update(game_timing.get_delta_time());
        player.draw();

        control_player(player, game_timing, game_size, room);

        if (game_timing.update_frame())
        {
            refresh_screen();
        }

        process_events();
    }

    return 0;
}
