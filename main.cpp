#include "splashkit.h"
#include <iostream>
#include <cmath>
#include <vector>

// increase the value of the input x by an ease out quint, until it reaches 1
double ease_out_quint(double x)
{
    return 1 - std::pow(1 - x, 5);
}

// use to increase values by an ease function,
struct ease_data
{
private:
    double initial_value; // initial value of the variable (only used for ease_value without variable pointer)
    double change_by;     // the amount the value needs to be change from initial to end
    double increase;      // the amount the value has been increased by (x value of the ease function)

    double calculate_ease(double time, double delta_time)
    {
        int max_increase = 1; // increase should only go from 0 to 1

        double increment_rate = max_increase / (time / delta_time);
        increase += increment_rate; // incrementing the increase by the rate

        if (increase >= max_increase)
        {
            increase = max_increase; // making sure increase does not go over 1
        }

        return ease_func(increase); // returning the ease function of the increase
    }

public:
    double value; // the value that is being eased (only used for ease_value without variable pointer) default is 0
    // should be changed if the value to start increasing is not 0
    // acts as a pointer to the variable that is being eased (syncs the value with the variable)

    double time_to_ease;         // the time it takes for the value to ease to the end value (used when change_by is positive)
    double time_to_release;      // the time it takes for the value to release to the initial value (used when change_by is negative)
    double (*ease_func)(double); // the ease function to be used

    // Constructor
    ease_data()
    {
        increase = 0;
        change_by = 0;
        initial_value = 0;
        value = 0;             // default value is 0
        time_to_ease = 800;    // default time to ease is 800 ms
        time_to_release = 800; // default time to release is 800 ms
    }

    // eases the value of the variable to the to_value directly (using pointers)
    void ease_value(double *variable, double to_value, double delta_time)
    {
        double time;

        if (this->change_by != to_value - *variable)
        {
            increase = 0;
        }

        if (increase == 0)
        {
            initial_value = *variable;
            change_by = to_value - *variable;
        }

        if (change_by < 0)
        {
            time = time_to_release;
        }
        else if (change_by > 0)
        {
            time = time_to_ease;
        }

        double eased = calculate_ease(time, delta_time);

        *variable = initial_value + eased * change_by;
    }

    // returns the eased value of the initial value to the to_value
    double ease_value(double to_value, double delta_time)
    {
        double time;

        if (this->change_by != to_value - value)
        {
            increase = 0;
        }

        if (increase == 0)
        {
            this->initial_value = value;
            change_by = to_value - value;
        }

        if (change_by < 0)
        {
            time = time_to_release;
        }
        else if (change_by > 0)
        {
            time = time_to_ease;
        }

        double eased = calculate_ease(time, delta_time);

        value = initial_value + eased * change_by;

        return value;
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
        double pixel_x = ceil(x / tile_size) - 1;
        double pixel_y = ceil(y / tile_size) - 1;
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

// make a game size struct
class game_size_data
{
private:
    int screen_height; // the height of the screen
    int screen_width;  // the width of the screen
    int room_width;    // the number of tiles in the room on the x axis
    int room_height;   // the number of tiles in the room on the y axis
    double zoom_level; // the zoom level of the game or screen

public:
    // Constructor
    game_size_data(int screen_height, int screen_width, int room_width, int room_height)
    {
        this->screen_height = screen_height;
        this->screen_width = screen_width;
        this->room_width = room_width;
        this->room_height = room_height;
        this->zoom_level = 1;
    }

    game_size_data() : game_size_data(1080, 1920, 40, 30) {}

    // getters and setters
    int get_screen_height() const
    {
        return screen_height;
    }
    // screen width is calculated using the screen height and the room width and height
    int get_screen_width() const
    {
        return screen_width;
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

    // set the zoom level of the game
    void set_zoom_level(double zoom_level)
    {
        this->zoom_level = zoom_level;
    }

    void set_zoom_level(double zoom_level, ease_data &ease, double delta_time)
    {
        ease.ease_value(&this->zoom_level, zoom_level, delta_time);
    }

    // get the camera position that would keep the parameter coorindate in the center of the screen
    coordinate get_camera_position(coordinate center_position) const
    {
        return {center_position.x - (get_screen_width() / 2), center_position.y - (screen_height / 2)};
    }
};

// class to hold the timing data of the game (mostly for the delta time)
class game_timing_data
{
private:
    double delta_time;             // time between the last frame and the current frame
    double time_rate;              // how many seconds the game should load in one second
    double time_difference;        // delta time wihtout the time rate
    double last_update_time;       // the time of the last frame
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
        delta_time = 0;
        time_rate = 1;
        time_difference = 0;
        last_update_time = 0;
        frame_rate = 60;
        last_frame_update_time = 0;
    }

    game_timing_data(int frame_rate)
    {
        delta_time = 0;
        time_rate = 1;
        time_difference = 0;
        last_update_time = 0;
        last_frame_update_time = 0;
        this->frame_rate = frame_rate;
    }

    // must be ran inside a game loop in order to update the delta time
    void update_timing()
    {
        time_difference = (double)timer_ticks("Main timer") - (double)last_update_time; // getting delta_time
        last_update_time = (double)timer_ticks("Main timer");                           // setting the last update time
        last_frame_update_time += time_difference;                                      // adding the delta time to the last frame update time
        delta_time = time_difference * time_rate;                                       // changing the delta time according to the time rate
    }

    // setters and getters
    double get_delta_time() const
    {
        return delta_time;
    }

    double get_time_difference() const
    {
        return time_difference;
    }

    // set time rate, changes the time rate of the delta time
    void set_time_rate(double rate)
    {
        time_rate = rate;
    }

    void set_time_rate(double time_rate, ease_data &ease, double delta_time)
    {
        ease.ease_value(&this->time_rate, time_rate, delta_time);
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
    void construct_room(int room_width, int room_height, int screen_width, int screen_height, const color &floor_color_1, const color &floor_color_2, const color &wall_color, const coordinate &spawn_tile)
    {
        color_pattern[0] = floor_color_1;
        color_pattern[1] = floor_color_2;
        color_pattern[2] = wall_color;
        this->size_x = room_width;
        this->size_y = room_height;
        this->zoom_level = 1;

        double size1 = (double)screen_width / (double)room_width;
        double size2 = (double)screen_height / (double)room_height;
        if (size1 >= size2)
            this->tile_size = size1;
        else
            this->tile_size = size2;

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
        zoomed_tile_size = tile_size * zoom_level;
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
    // Constructor
    room_data(int floor_width, int floor_height, int screen_width, int screen_height)
    {
        // setting room color
        color slate_grey = rgb_color(112, 128, 144);
        color light_slate_grey = rgb_color(132, 144, 153);
        color light_steel_blue = rgb_color(150, 170, 200);

        construct_room(floor_width, floor_height, screen_width, screen_height, slate_grey, light_slate_grey, light_steel_blue, {(double)(floor_width - 1) / 2, (double)(floor_height - 1) - 2});
    }

    room_data(int floor_width, int floor_height, int screen_width, int screen_height, const coordinate &spawn_coords)
    {
        // setting room color
        color slate_grey = rgb_color(112, 128, 144);
        color light_slate_grey = rgb_color(132, 144, 153);
        color light_steel_blue = rgb_color(150, 170, 200);

        construct_room(floor_width, floor_height, screen_width, screen_height, slate_grey, light_slate_grey, light_steel_blue, spawn_coords);
    }

    room_data(int floor_width, int floor_height, int screen_width, int screen_height, const color &floor_color_1, const color &floor_color_2, const color &wall_color, const coordinate &spawn_coords)
    {
        construct_room(floor_width, floor_height, screen_width, screen_height, floor_color_1, floor_color_2, wall_color, spawn_coords);
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

    double get_tile_size() const
    {
        return tile_size;
    }

    const color *get_color_pattern() const
    {
        return color_pattern;
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

    void set_color_pattern(const color &floor_color_1, const color &floor_color_2, const color &wall_color)
    {
        color_pattern[0] = floor_color_1;
        color_pattern[1] = floor_color_2;
        color_pattern[2] = wall_color;
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

    coordinate position;        // position of the character in correlation to the screen (non-zoomed)
    coordinate zoomed_position; // position of the character in correlation to the room (after zooming), only used for drawing

    void update_zoomed_position()
    {
        zoomed_position = {position.x * zoom_level, position.y * zoom_level};
    }

protected:
    // constructor
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

    void set_zoomed_position(const coordinate &zoomed_position)
    {
        this->zoomed_position = zoomed_position;
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

    double get_zoom_level() const
    {
        return zoom_level;
    }

    void set_position(const coordinate &position)
    {
        this->position = position;
    }

    void set_health(int health)
    {
        this->health = health;
    }

    // get position of the character in colleration to the room
    const coordinate &get_zoomed_position() const
    {
        return zoomed_position;
    }

public:
    // update the character's position and hurtbox, should always be ran inside the game loop
    void update()
    {
        // no need to update if the character is dead
        if (get_health() <= 0)
        {
            return;
        }

        update_zoomed_position();
        update_zoomed_model_scaling();
        update_hurtbox();
    }

    // move the character in a direction by a distance
    void move(vector_2d &direction, double distance, const room_data &room)
    {
        if (direction.x != 0 || direction.y != 0)
        {
            direction = unit_vector(direction); // convert to unit vector
        }

        vector_2d movement = vector_multiply(direction, distance);
        coordinate new_position = {position.x + movement.x, position.y + movement.y};
        // set_position(new_position);

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
                        new_position = {new_position.x, (new_position.y + collision_box.height)};
                    }
                    // hurtbox_copy.y = new_position.y - collision_box.height;
                    else
                    {
                        // if collision, then move the player by y - collision_box.height
                        new_position = {new_position.x, (new_position.y - collision_box.height)};
                    }
                }

                if (collision_box.height > collision_box.width)
                {
                    // checking if player moves by x + collision_box.width will still cause collision
                    hurtbox.x += collision_box.width;
                    if (intersection(hurtbox, walls_vector[i]).width != 0)
                    {
                        // if no collision, then move the player by x + collision_box.width
                        new_position = {(new_position.x - collision_box.width), new_position.y};
                    }
                    // hurtbox_copy.x = new_position.x - collision_box.width;
                    else
                    {
                        // if collision, then move the player by x - collision_box.width
                        new_position = {(new_position.x + collision_box.width), new_position.y};
                    }
                }
            }
        }

        set_position(new_position);
    }

    // draw the character onto the screen
    virtual void draw() const
    {
        if (get_health() <= 0)
        {
            return;
        }

        double model_width = bitmap_width(get_model());
        double model_height = bitmap_height(get_model());
        double zoomed_model_scaling = get_zoomed_model_scaling();

        // fixing bitmap scaling position
        double pos_x = get_zoomed_position().x + (((model_width * zoomed_model_scaling) - model_width) / 2);
        double pos_y = get_zoomed_position().y + (((model_height * zoomed_model_scaling) - model_height) / 2);

        // flip when facing opposite direction
        if (get_is_facing_right())
        {
            draw_bitmap(get_model(), pos_x, pos_y, option_scale_bmp(zoomed_model_scaling, zoomed_model_scaling));
        }
        else
        {
            draw_bitmap(get_model(), pos_x, pos_y, option_flip_y(option_scale_bmp(zoomed_model_scaling, zoomed_model_scaling)));
        }
    }

    // check if the character's hurtbox is colliding with a hitbox
    void check_hitbox_collision(const rectangle &hitbox)
    {
        rectangle collision_box = intersection(hurtbox, hitbox);
        if (collision_box.width != 0 && collision_box.height != 0)
        {
            // if there is a collision, decrease the health of the character
            health--;
        }
    }
    // getters and setters
    double get_speed() const
    {
        return speed;
    }

    int get_health() const
    {
        return health;
    }

    const coordinate &get_position() const
    {
        return position;
    }

    // set the direction the character is facing (true is right, false is left)
    void set_is_facing_right(bool facing_right)
    {
        model_facing_right = facing_right;
    }

    // set character's zoom level (to zoom with the screen)
    void set_zoom_level(double zoom_level)
    {
        this->zoom_level = zoom_level;
    }

    // get the player's center position on the screen
    coordinate get_center_position() const
    {
        return {zoomed_position.x + (bitmap_width(get_model()) * get_zoomed_model_scaling()) / 2, zoomed_position.y + (bitmap_height(get_model()) * get_zoomed_model_scaling()) / 2};
    }

    // get the hurtbox of the character
    const rectangle &get_hurtbox() const
    {
        return hurtbox;
    }
};

// class for npcs in the game
class npc_data : public character_data
{
private:
    coordinate new_position;        // the position the npc is moving to
    coordinate zoomed_new_position; // the position the npc is moving to after zooming

    double auto_move_max_distance;        // the range at which the new_position can be from its current position (in a square)
    double zoomed_auto_move_max_distance; // the range at which the new_position can be from its current position (in a square) after zooming

    int time_since_new_position; // the time since the npc has a new position
    int new_position_cooldown;   // the time the npc should have a new position

    // generates a random position for the npc, that is valid for the npc to move to, or stay at
    void update_new_position(const room_data &room, coordinate min_coords, coordinate max_coords)
    {
        min_coords = min_coords.pixel_to_tile(room.get_zoomed_tile_size());
        max_coords = max_coords.pixel_to_tile(room.get_zoomed_tile_size());

        coordinate rand_position;

        do
        {
            // generating a random position for the npc to move to converting to tile coordinates
            rand_position = random_coordinate(min_coords, max_coords);
            // checking if the tile is within the room
            if (rand_position.x < 0 || rand_position.x >= room.get_size_x() || rand_position.y < 0 || rand_position.y >= room.get_size_y())
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
                    player_tiles.push_back({rand_position.x + j, rand_position.y + i});
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

        set_new_position(rand_position);
    }

    // move the npc to a random position within a range automatically
    void auto_set_new_position(double delta_time, const room_data &room)
    {
        coordinate position = get_zoomed_position();

        // determines if NPC is at the destination, allows an error of (+-)10 pixel due to the float to int conversion, and other scalings
        bool x_at_destination = ((int)position.x >= zoomed_new_position.x - 10) && ((int)position.x <= zoomed_new_position.x + 10);
        bool y_at_destination = ((int)position.y >= zoomed_new_position.y - 10) && ((int)position.y <= zoomed_new_position.y + 10);

        time_since_new_position += delta_time;
        bool cooldown_passed = time_since_new_position >= new_position_cooldown;

        // if the npc is at the destination or cooldown for new position passed, generate a new destination
        if ((x_at_destination && y_at_destination) || cooldown_passed)
        {
            coordinate min_coords = {get_zoomed_position().x - zoomed_auto_move_max_distance, get_zoomed_position().y - zoomed_auto_move_max_distance};
            coordinate max_coords = {get_zoomed_position().x + zoomed_auto_move_max_distance, get_zoomed_position().y + zoomed_auto_move_max_distance};

            update_new_position(room, min_coords, max_coords);

            new_position = new_position.tile_to_pixel(room.get_tile_size()); // converting back to pixel coordinates for the npc to move to
            time_since_new_position = 0;
        }
    }

    void auto_move(double delta_time, const room_data &room)
    {
        // calculate the direction and distance the npc should move
        vector_2d direction = {0, 0};
        direction.x = zoomed_new_position.x - get_zoomed_position().x;
        direction.y = zoomed_new_position.y - get_zoomed_position().y;

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

    // update values that are affected by zoom level
    void update_zoomed_auto_move_max_distance()
    {
        zoomed_auto_move_max_distance = auto_move_max_distance * get_zoom_level();
    }

    // update values that are affected by zoom level
    void update_zoomed_new_position()
    {
        zoomed_new_position = {new_position.x * get_zoom_level(), new_position.y * get_zoom_level()};
    }

    // setting new position of the npc (that it will auto move to)
    void set_new_position(const coordinate &new_position)
    {
        this->new_position = new_position;
    }

public:
    // Constructor
    npc_data(double tile_size, double model_size, const room_data &room, string bitmap_name)
        : character_data(1, 4 * tile_size / 1000, bitmap_named(bitmap_name), true, model_size, {0, 0})
    {
        auto_move_max_distance = 10 * tile_size;
        new_position_cooldown = 5000; // ms
        time_since_new_position = 0;  // ms

        coordinate max = {(double)(room.get_size_x() - 1), (double)(room.get_size_y() - 1)};
        coordinate min = {0, 0};
        update_new_position(room, min.tile_to_pixel(room.get_zoomed_tile_size()), max.tile_to_pixel(room.get_zoomed_tile_size()));

        new_position = new_position.tile_to_pixel(room.get_zoomed_tile_size());

        set_position(new_position);
        update_zoomed_auto_move_max_distance();
        update_zoomed_new_position();
    }

    // update the npc's position and hurtbox, should always be ran inside the game loop
    void update(double delta_time, const room_data &room)
    {
        // no need to update if the npc is dead
        if (get_health() <= 0)
        {
            return;
        }

        update_zoomed_auto_move_max_distance();
        update_zoomed_new_position();
        auto_set_new_position(delta_time, room);

        auto_move(delta_time, room);

        // calls update from character_data base class, updates hitbox and model scaling
        character_data::update();
    }

    void set_health(int health)
    {
        character_data::set_health(health);
    }

    // friends with monster_data to allow monster_data to access npc_data's private members (used as disguise)
    friend class monster_data;
};

enum sword_phase
{
    SWORD_DRAW = 0,
    SWORD_SWING,
    NO_SWORD
};

// sword info stuct
struct sword_data
{
    bitmap sword_draw_model;  // bitmap of the sword when the player is drawing the sword
    bitmap sword_swing_model; // bitmap of the sword when the player is swinging the sword
    double model_scaling;     // scaling of the sword model
    coordinate position;      // position of the sword
    sword_phase phase;        // the phase of the sword (drawing, swinging, or no sword) (used to determine which bitmap to draw)
};

// player info struct, players can attack
class player_data : public character_data
{
private:
    int attack_speed;        // ms
    bool is_attacking;       // if true, the player is attacking, used to check with timing and drawing
    bool create_hitbox;      // if true, the player's sword will have a hitbox
    rectangle hitbox;        // the hitbox of the sword, the hitbox is active when the player is attacking
    int hitbox_lasting_time; // ms
    double attack_cooldown;  // ms
    bool can_attack;         // if true, the player can attack, if false, the player is in cooldown

    sword_data sword;
    double sword_zoomed_model_scaling; // scaling of the sword when zoomed, derived from the sword's model scaling

    // main function for attacking, controls the attacking animation, and the time the hitbox is active (according to attack_speed)
    void attacking()
    {
        // if the player is not attacking, the sword phase is set to no sword
        if (!is_attacking)
        {
            sword.phase = NO_SWORD; // doesnt draw the sword on the screen if the phase is no sword
            return;
        }

        // NOTE: sword animation and progress takes 2 * attack_speed

        // for the start of the animaion, the player is drawing the sword for attack_speed
        if (attack_cooldown < attack_speed)
        {
            sword.phase = SWORD_DRAW;
            return;
        }

        // after attakc_speed time, the player is swinging the sword for attack_speed * 0.5
        if (attack_cooldown < attack_speed * 1.5)
        {
            sword.phase = SWORD_SWING;

            // create hitbox when the player is swinging the sword, only for hitbox_lasting_time
            if (attack_cooldown < attack_speed + hitbox_lasting_time)
                create_hitbox = true;
            else
                create_hitbox = false;

            return;
        }
        sword.phase = NO_SWORD; // doesnt draw the sword on the screen if the phase is no sword
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
            hitbox = {get_zoomed_position().x + (player_model_width * get_zoomed_model_scaling()), get_zoomed_position().y, hitbox_size_x, hitbox_size_y};
        else
            hitbox = {get_zoomed_position().x - hitbox_size_x, get_zoomed_position().y, hitbox_size_x, hitbox_size_y};
    }

    // update the sword's zoom to fit with the zoom_level
    void update_sword_zoomed_model_scaling()
    {
        sword_zoomed_model_scaling = sword.model_scaling * get_zoom_level();
    }

public:
    // Constructor
    player_data(double tile_size, double model_size, const coordinate &spawn_coords, string bitmap_name)
        : character_data(1, 5.0 * tile_size / 1000, bitmap_named(bitmap_name), true, model_size, spawn_coords)
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
        sword.sword_draw_model = bitmap_named("sword_draw");
        sword.sword_swing_model = bitmap_named("sword_swing");

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

    player_data(double tile_size, double model_size, string bitmap_name)
        : player_data(tile_size, model_size, {0, 0}, bitmap_name) {}

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
        sword.phase = NO_SWORD; // default phase is no sword

        update_sword_zoomed_model_scaling();

        double player_model_width = bitmap_width(get_model());
        double player_model_height = bitmap_height(get_model());

        double sword_model_width = bitmap_width(sword.sword_draw_model);

        double model_scaling = get_zoomed_model_scaling();

        // making the sword align with the player
        if (get_is_facing_right())
        {
            sword.position.x = get_zoomed_position().x + (player_model_width * model_scaling);
            sword.position.y = get_zoomed_position().y + (player_model_height / (player_model_height / player_model_width) * model_scaling);
        }
        else
        {
            sword.position.x = get_zoomed_position().x - (sword_model_width * sword_zoomed_model_scaling);
            sword.position.y = get_zoomed_position().y + (player_model_height / (player_model_height / player_model_width) * model_scaling);
        }
    }

    // must be ran inside game loop, and get delta_time from game_timing_data
    void update(double delta_time)
    {
        // no need to update if the player is dead
        if (get_health() <= 0)
        {
            return;
        }

        character_data::update();
        update_hitbox();
        update_sword();

        if (is_attacking)
        {
            // if the player is attacking, update the attack cooldown and the attack state
            update_attack_cooldown(delta_time);
            // continue the attacking process
            attacking();
        }
    }

    // get the player's sword hitbox
    const rectangle &get_hitbox() const
    {
        return hitbox;
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

        // flip when facing opposite direction
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

    void draw() const
    {

        // no need to draw if the player is dead
        if (get_health() <= 0)
        {
            return;
        }

        character_data::draw();
        draw_sword();
    }
};

// monster class, inherits from character_data, can disguise as npc (friendship with npc_data)
class monster_data : public character_data
{
private:
    // outline of the monster when disguised (or highlight)
    bool show_outline;
    ease_data ease_outline;

    // if the monster is exposed, it will be drawn, if not, the disguise will be drawn
    bool expose_self;

    // the range at which the monster will detect the player
    double player_detection_range;
    bool escaped_player; // if the monster has escaped the player out of detection range, it is true, if not, it is false

    npc_data *disguise; // the npc object disguise of the monster
    rectangle hitbox;   // the hitbox of the monster, the hitbox is active when the monster is exposed

    // escape the player by moving away from the player
    void escape_player(const player_data &player)
    {
        // getting direction between the player and the monster
        vector_2d direction = {0, 0};
        direction.x = get_position().x - player.get_position().x;
        direction.y = get_position().y - player.get_position().y;

        // the distance between the player and the monster, if the distance is greater than the player_detection_range, the monster has escaped the player
        double distance = vector_magnitude(direction);
        if (distance > player_detection_range)
        {
            if (!escaped_player)
            {
                // this is to reset the disguise's position to activate the new position for npc object to automatically move to
                // in the NPC class, if the new position is the same as the current position, the npc will get a random new position and move to it
                disguise->set_new_position(get_position());
            }

            escaped_player = true;
            return;
        }

        // if the monster has not escapted the player, it will move away from the player
        escaped_player = false;
        if (direction.x != 0 || direction.y != 0)
        {
            // convert to unit vector
            direction = unit_vector(direction);
        }

        // monster's disguise will move away from the player at the shortest distance
        disguise->set_new_position({get_position().x + direction.x, get_position().y + direction.y});
    }

    // chase the player by moving directly towards the player, used when the monster is exposed
    void chase_player(int delta_time, const player_data &player, const room_data &room)
    {
        vector_2d direction = {0, 0};
        direction.x = player.get_center_position().x - get_center_position().x;
        direction.y = player.get_center_position().y - get_center_position().y;

        // set the direction the monster is facing, for drawing
        if (direction.x > 0)
        {
            set_is_facing_right(true);
        }
        else
        {
            set_is_facing_right(false);
        }

        // the distance the monster will move according to delta_time
        double distance = get_speed() * (double)delta_time;

        move(direction, distance, room);
    }

    // update the monster's hitbox
    void update_hitbox()
    {
        // the monster's hitbox the same as its hurtbox
        hitbox = get_hurtbox();

        // if the monster is not exposed, the hitbox is 0
        if (!expose_self)
        {
            hitbox.height = 0;
            hitbox.width = 0;
        }
    }

public:
    // Constructor
    monster_data(double tile_size, double model_disguise_size, double model_size, const room_data &room, string bitmap_disguise_name, string bitmap_name)
        : character_data(1, 10 * tile_size / 1000, bitmap_named(bitmap_name), true, model_size, {0, 0})
    {
        show_outline = false;
        expose_self = false;

        // creating an npc object for the monster to disguise as
        disguise = new npc_data(tile_size, model_disguise_size, room, bitmap_disguise_name);
        player_detection_range = 4 * tile_size;
        escaped_player = true;
        update_hitbox();
    }

    // set zoom level for itself and the disguise object
    void set_zoom_level(double zoom_level)
    {
        character_data::set_zoom_level(zoom_level);
        disguise->set_zoom_level(zoom_level);
    }

    // update itself and the disguise object, must be called in the game loop
    void update(double delta_time, const room_data &room, const player_data &player)
    {

        // no need to update if the monster is dead
        if (get_health() <= 0)
        {
            return;
        }

        disguise->update(delta_time, room);
        character_data::update();
        update_hitbox();

        // syncronize stats of the monster and the disguise
        if (!expose_self)
        {
            // if the monster is not exposed, the monster will take the disguise's position and health
            set_position(disguise->get_position());
            set_health(disguise->get_health());
            escape_player(player);
        }
        else
        {
            // if the monster is exposed, the disguised will take the monster's position and health
            disguise->set_position(get_position());
            disguise->set_health(get_health());
            chase_player(delta_time, player, room);
        }
    }

    // draw the monster onto the screen, with easing for the outline
    void draw(ease_data &ease, double delta_time)
    {
        // no need to draw if the monster is dead
        if (get_health() <= 0)
        {
            return;
        }

        if (expose_self)
        {
            // if the monster is exposed, the monster will be drawn
            character_data::draw();
        }
        else
        {
            // if the monster is not exposed, the disguise will be drawn
            disguise->draw();

            // drawing the outline of the disguise, if show_outline is true, the outline has an easing effect for its visibility
            if (show_outline)
            {
                fill_rectangle(rgba_color(150.0, 170.0, 200.0, ease.ease_value(0.5, delta_time)), disguise->get_hurtbox());
            }
            else
            {
                fill_rectangle(rgba_color(150.0, 170.0, 200.0, ease.ease_value(0.0, delta_time)), disguise->get_hurtbox());
            }
        }
    }

    // check if the monster's hitbox is colliding with a hitbox
    void check_hitbox_collision(const rectangle &hitbox)
    {
        if (expose_self)
        {
            character_data::check_hitbox_collision(hitbox);
        }
        else
        {
            disguise->check_hitbox_collision(hitbox);
        }
    }

    // getters and setters
    void set_show_outline(bool show_outline)
    {
        this->show_outline = show_outline;
    }

    void set_expose_self(bool expose_self)
    {
        this->expose_self = expose_self;
    }

    const rectangle &get_hitbox() const
    {
        return hitbox;
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

// function to draw the vignette on the screen
void draw_vignette(double scale = 1)
{
    // camera position
    point_2d camera_pos = camera_position();

    // lengths and widths
    double vignette_width = bitmap_width("vignette");
    double vignette_height = bitmap_height("vignette");
    double screen_x = screen_width();
    double screen_y = screen_height();

    // scaling the vignette to fit the screen and custom scaling
    double scale_x = (screen_x / vignette_width) * scale;
    double scale_y = (screen_y / vignette_height) * scale;

    // getting the camera's cetner
    double camera_center_x = camera_pos.x + (screen_x / 2);
    double camera_center_y = camera_pos.y + (screen_y / 2);

    // getting the distance of the center of the vignette
    double vignette_center_x = (vignette_width * scale_x) / 2;
    double vignette_center_y = (vignette_height * scale_y) / 2;

    // getting the x and y position to draw the vignette (with aligning error fix from bitmap scaling)
    double x = (camera_center_x - vignette_center_x) + (((vignette_width * scale_x) - vignette_width) / 2);
    double y = (camera_center_y - vignette_center_y) + (((vignette_height * scale_y) - vignette_height) / 2);

    draw_bitmap("vignette", x, y, (option_scale_bmp(scale_x, scale_y)));
}

// control to slow time, used for the focusing ability
void control_ability(game_timing_data &game_timing, game_size_data &game_size, monster_data &monster, ease_data &time_rate_ease, ease_data &zoom_level_ease, ease_data &filter_ease)
{
    point_2d camera_pos = camera_position();

    if (key_down(LEFT_SHIFT_KEY) || mouse_down(RIGHT_BUTTON))
    {
        // slowing time and zooming in with easing
        game_timing.set_time_rate(0.35, time_rate_ease, game_timing.get_time_difference());
        game_size.set_zoom_level(2.5, zoom_level_ease, game_timing.get_time_difference());

        // drawing effects on screen
        // vignetted screen
        draw_vignette();
        // color to desaturate the screen
        fill_rectangle(rgba_color(150.0, 170.0, 200.0, filter_ease.ease_value(0.5, game_timing.get_time_difference())), camera_pos.x, camera_pos.y, screen_width(), screen_height());
        // outline or highlight the monster
        monster.set_show_outline(true);
    }
    else
    {
        // returning time and zoom to normal with easing
        game_timing.set_time_rate(1, time_rate_ease, game_timing.get_time_difference());
        game_size.set_zoom_level(1, zoom_level_ease, game_timing.get_time_difference());

        // removing destauration on screen
        fill_rectangle(rgba_color(150.0, 170.0, 200.0, filter_ease.ease_value(0.0, game_timing.get_time_difference())), camera_pos.x, camera_pos.y, screen_width(), screen_height());
        monster.set_show_outline(false);
    }
}

// function to control character, must be called in the game loop
void control_player(player_data &player, game_timing_data &game_timing, const room_data &room)
{
    move_player(player, game_timing.get_delta_time(), room);
    player_attack(player);
}

// function to contol counter for the game, minimum is 0
double timer_countdown(int time_limit, bool timer_over)
{
    if (timer_over)
    {
        return 0;
    }

    double timer = (double)time_limit - (double)timer_ticks("Main timer");
    if (timer <= 0)
    {
        timer = 0;
    }
    return timer;
}

// change color of the room as timer goes down, change from initial color to new color array
void change_color(double time_left, int time_start_warning, room_data &room, color initial_color_array[3], color new_color_array[3])
{
    // floor 1
    float floor_1_r_change = (new_color_array[0].r - initial_color_array[0].r) * (1 - (time_left / time_start_warning));
    float floor_1_g_change = (new_color_array[0].g - initial_color_array[0].g) * (1 - (time_left / time_start_warning));
    float floor_1_b_change = (new_color_array[0].b - initial_color_array[0].b) * (1 - (time_left / time_start_warning));
    color floor_1_change = rgb_color(initial_color_array[0].r + floor_1_r_change, initial_color_array[0].g + floor_1_g_change, initial_color_array[0].b + floor_1_b_change);

    // floor 2
    float floor_2_r_change = (new_color_array[1].r - initial_color_array[1].r) * (1 - (time_left / time_start_warning));
    float floor_2_g_change = (new_color_array[1].g - initial_color_array[1].g) * (1 - (time_left / time_start_warning));
    float floor_2_b_change = (new_color_array[1].b - initial_color_array[1].b) * (1 - (time_left / time_start_warning));
    color floor_2_change = rgb_color(initial_color_array[1].r + floor_2_r_change, initial_color_array[1].g + floor_2_g_change, initial_color_array[1].b + floor_2_b_change);

    // wall
    float wall_r_change = (new_color_array[2].r - initial_color_array[2].r) * (1 - (time_left / time_start_warning));
    float wall_g_change = (new_color_array[2].g - initial_color_array[2].g) * (1 - (time_left / time_start_warning));
    float wall_b_change = (new_color_array[2].b - initial_color_array[2].b) * (1 - (time_left / time_start_warning));
    color wall_change = rgb_color(initial_color_array[2].r + wall_r_change, initial_color_array[2].g + wall_g_change, initial_color_array[2].b + wall_b_change);

    // setting the new color as timer goes down
    room.set_color_pattern(floor_1_change, floor_2_change, wall_change);
}

// visual warnings as timer goes down, change color and draw vignette zooming in
void count_down_warning(double time_left, int time_start_warning, room_data &room, color initial_color_array[3], ease_data &ease, double delta_time)
{
    if (time_left < time_start_warning)
    {
        color new_color_array[3] = {
            rgb_color(0, 0, 0),
            rgb_color(68, 68, 68),
            rgb_color(139, 0, 0),
        };
        change_color(time_left, time_start_warning, room, initial_color_array, new_color_array);

        // vignette zooming in as timer goes down, from initial scale to final scale
        double initial_vignette_scale = 5;
        double final_vignette_scale = 1.5;
        double scale = initial_vignette_scale - ((initial_vignette_scale - final_vignette_scale) * (1 - (time_left / time_start_warning)));
        draw_vignette(scale);
        if (time_left <= 0)
        {
            point_2d camera_pos = camera_position();
            fill_rectangle(rgba_color(new_color_array[2].r, new_color_array[2].g, new_color_array[2].b, ease.ease_value(0.1, delta_time)), camera_pos.x, camera_pos.y, screen_width(), screen_height());
        }
    }
}

void draw_timer(double time_left, double window_width, double window_height)
{
    // drawing the time left on the screen (timer countdown)
    string time_left_string = std::to_string(time_left);

    int text_h = text_height(std::to_string(time_left), get_system_font(), window_height * 0.05);
    int text_w = text_width(std::to_string(time_left), get_system_font(), window_height * 0.05);

    double text_pos_x = camera_position().x + ((double)window_width / 2.0);
    double text_pos_y = camera_position().y + ((double)window_height * 0.05);

    double text_center_x = text_pos_x - (text_w / 2);
    double text_center_y = text_pos_y - (text_h / 2);

    draw_text(std::to_string(time_left), color_white(), get_system_font(), window_height * 0.05, text_center_x, text_center_y);
}

void timer_out(vector<npc_data *> &npcs, monster_data &monster)
{
    // if the timer is out, the monster will be exposed
    monster.set_expose_self(true);

    // if the timer is out, the npcs will be exposed
    for (int i = 0; i < npcs.size(); i++)
    {
        npcs[i]->set_health(0);
    }
}

// draw the end screen, either game won or game lost
void draw_end_screen(bool game_won, bool game_lost, double window_width, double window_height)
{
    // setting font sizes
    double font_size = window_height * 0.15;
    double font_size_small = font_size * 0.2;

    // setting the position of the text according to the camera
    double pos_x = camera_position().x + ((double)window_width / 2.0);
    double pos_y = camera_position().y + ((double)window_height / 3.0);

    // pos_y2 is for the subtext
    double pos_y2 = camera_position().y + (((double)window_height / 3.0) * 2);

    // text and subtext will have the same x position

    if (game_won)
    {
        string text = "Level Complete!";
        double text_h = text_height(text, get_system_font(), font_size);
        double text_w = text_width(text, get_system_font(), font_size);

        double text_center_x = pos_x - (text_w / 2);
        double text_center_y = pos_y - (text_h / 2);

        string sub_text = "Press Esc to Continue";

        double sub_text_h = text_height(sub_text, get_system_font(), font_size_small);
        double sub_text_w = text_width(sub_text, get_system_font(), font_size_small);

        double sub_text_center_x = pos_x - (sub_text_w / 2);
        double sub_text_center_y = pos_y2 - (sub_text_h / 2);

        fill_rectangle(rgba_color(255.0, 255.0, 255.0, 0.5), camera_position().x, camera_position().y, window_width, window_height);
        draw_text(text, color_white(), get_system_font(), font_size, text_center_x, text_center_y);
        draw_text(sub_text, color_white(), get_system_font(), font_size_small, sub_text_center_x, sub_text_center_y);
    }
    if (game_lost)
    {
        string text = "Game Over!";

        double text_h = text_height(text, get_system_font(), font_size);
        double text_w = text_width(text, get_system_font(), font_size);

        double text_center_x = pos_x - (text_w / 2);
        double text_center_y = pos_y - (text_h / 2);

        string sub_text = "Press Esc to Restart";

        double sub_text_h = text_height(sub_text, get_system_font(), font_size_small);
        double sub_text_w = text_width(sub_text, get_system_font(), font_size_small);

        double sub_text_center_x = pos_x - (sub_text_w / 2);
        double sub_text_center_y = pos_y2 - (sub_text_h / 2);

        fill_rectangle(rgba_color(129.0, 0.0, 0.0, 0.5), camera_position().x, camera_position().y, window_width, window_height);
        draw_text(text, color_white(), get_system_font(), font_size, text_center_x, text_center_y);
        draw_text(sub_text, color_white(), get_system_font(), font_size_small, sub_text_center_x, sub_text_center_y);
    }
}

int main()
{
    // load bitmaps
    load_bitmap("vignette", "./image_data/vignette/vignette.png");
    load_bitmap("player_idle", "./image_data/player/player_idle.png");
    load_bitmap("sword_draw", "./image_data/sword/sword_1.png");
    load_bitmap("sword_swing", "./image_data/sword/sword_2.png");
    load_bitmap("npc_idle", "./image_data/npc/npc_idle.png");
    load_bitmap("monster", "./image_data/monster/monster.png");

    // set up game variables
    int game_level = 1; // starts at level 1, increases by 1 each level
    int window_width = 1920;
    int window_height = 1080;
    int frame_rate = 120;

    // open window
    open_window("Find The Fake", window_width, window_height);
    create_timer("Main timer");
    start_timer("Main timer");

    while (!quit_requested())
    {

        int time_limit = 60000;  // ms
        bool timer_over = false; // when timer_over is true, timer runs down to 0 instantly
        bool game_won = false;
        bool game_lost = false;

        int room_width = rnd(20, 60);
        int room_height = rnd(20, 60);
        // time limit for the game, to find the monster
        // number of npcs in the room
        int npc_count = game_level + 1; // number of npcs increases by 1 each level

        // creating game objects
        game_size_data game_size(window_height, window_width, room_width, room_height);
        game_timing_data game_timing(frame_rate);

        // building the room object
        room_data room(game_size.get_room_width(), game_size.get_room_height(), game_size.get_screen_width(), game_size.get_screen_height());
        room.set_zoom_level(game_size.get_zoom_level());
        room.build_room();
        const color *color_array = room.get_color_pattern();

        double tile_size = room.get_zoomed_tile_size();

        // creating player and npc objects
        double player_model_size = tile_size;
        player_data player(tile_size, player_model_size, room.get_spawn_coords(), "player_idle");

        double npc_model_size = tile_size;
        vector<npc_data *> npcs(npc_count); // keeping all the npcs in the room in a vector (there are multiple npcs in the room)
        for (int i = 0; i < npc_count; i++)
        {
            npcs[i] = new npc_data(tile_size, npc_model_size, room, "npc_idle");
            npcs[i]->set_zoom_level(game_size.get_zoom_level());
            npcs[i]->update(game_timing.get_delta_time(), room);
        }

        // creating monster object
        double monster_model_size = tile_size * 2.4;
        monster_data monster(tile_size, npc_model_size, monster_model_size, room, "npc_idle", "monster");
        monster.set_zoom_level(game_size.get_zoom_level());
        monster.update(game_timing.get_delta_time(), room, player);

        // setting up easing functions and objects to be used
        ease_data highlight_ease;
        highlight_ease.ease_func = &ease_out_quint;
        highlight_ease.time_to_ease = 10000;

        ease_data time_rate_ease;
        time_rate_ease.ease_func = &ease_out_quint;

        ease_data zoom_level_ease;
        zoom_level_ease.ease_func = &ease_out_quint;

        ease_data filter_ease;
        filter_ease.ease_func = &ease_out_quint;

        ease_data timer_warning_ease;
        timer_warning_ease.ease_func = &ease_out_quint;
        timer_warning_ease.time_to_release = 1000;
        timer_warning_ease.value = 0.7;

        // initial, unupdated color of room, used to show timer countdown warnings
        color initial_color_array[3] = {color_array[0], color_array[1], color_array[2]};

        // game loop
        while (!quit_requested())
        {
            // setting game timing
            game_timing.update_timing();
            // skip frame if no time has passed (fast computers can have time difference of 0)
            if (game_timing.get_time_difference() == 0)
            {
                continue;
            }

            // clear screen
            clear_screen(color_array[2]);

            // updating player, npcs, and monster by calling their update functions, setting their zoom level, and checking for hitbox collision
            for (int i = 0; i < npc_count; i++)
            {
                if (npcs[i]->get_health() <= 0)
                {
                    timer_over = true;
                    break;
                }

                npcs[i]->set_zoom_level(game_size.get_zoom_level());
                npcs[i]->update(game_timing.get_delta_time(), room);
                npcs[i]->check_hitbox_collision(player.get_hitbox());
            }

            player.set_zoom_level(game_size.get_zoom_level());
            player.update(game_timing.get_delta_time());
            player.check_hitbox_collision(monster.get_hitbox());

            monster.set_zoom_level(game_size.get_zoom_level());
            monster.update(game_timing.get_delta_time(), room, player);
            monster.check_hitbox_collision(player.get_hitbox());

            // rebuilding the room to update the zoom level
            room.set_zoom_level(game_size.get_zoom_level());
            room.build_room();

            // setting the camera position to the player's center position
            coordinate center_pos = game_size.get_camera_position(player.get_center_position());
            set_camera_position({center_pos.x, center_pos.y});

            // drawing the room, npcs, player, and monster
            room.draw();
            for (int i = 0; i < npc_count; i++)
            {
                // only draw the npc if it is on the screen
                if (rect_on_screen(npcs[i]->get_hurtbox()))
                {
                    npcs[i]->draw();
                }
            }
            // only draw the player if it is on the screen
            if (rect_on_screen(player.get_hurtbox()))
            {
                player.draw();
            }
            // only draw the monster if it is on the screen
            if (rect_on_screen(monster.get_hurtbox()))
            {
                monster.draw(highlight_ease, game_timing.get_time_difference());
            }

            // control functions for player and ability (focusing)
            control_player(player, game_timing, room);
            control_ability(game_timing, game_size, monster, time_rate_ease, zoom_level_ease, filter_ease);

            // drawing the timer countdown on the screen
            draw_timer(timer_countdown(time_limit, timer_over) / 1000, window_width, window_height);
            // creating visual warnings as timer goes down
            count_down_warning(timer_countdown(time_limit, timer_over), 30000, room, initial_color_array, timer_warning_ease, game_timing.get_time_difference());
            // if one of the npcs is dead, the timer will run down to 0 instantly

            if (timer_countdown(time_limit, timer_over) <= 0)
            {
                timer_out(npcs, monster);
            }

            if (player.get_health() <= 0)
            {
                game_lost = true;
                break;
            }
            if (monster.get_health() <= 0)
            {
                game_won = true;
                break;
            }

            // limit refresh screen for frame rate
            if (game_timing.update_frame())
            {
                refresh_screen();
            }

            process_events();
        }

        for (int i = 0; i < npc_count; i++)
        {
            delete npcs[i];
        }
        npcs.clear();

        if (game_won)
        {
            game_level++;
        }

        if (game_lost)
        {
            game_level = 1;
        }

        draw_end_screen(game_won, game_lost, window_width, window_height);
        refresh_screen();

        while (!quit_requested() && !key_typed(ESCAPE_KEY))
        {
            process_events();
        }

        process_events();
        reset_timer("Main timer");
    }
    // TODO: set random walls
    return 0;
}
