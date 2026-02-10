#define KEY_BASE (65)
#define KEY_MAX (90)
#define KEY_W (87)
#define KEY_A (65)
#define KEY_S (83)
#define KEY_D (68)
#define KEY_P (80)
#define KEY_G (71)
#define KEY_J (74)
#define _UP_MASK (1 << (KEY_W - KEY_BASE))
#define _LEFT_MASK (1 << (KEY_A - KEY_BASE))
#define _DOWN_MASK (1 << (KEY_S - KEY_BASE))
#define _RIGHT_MASK (1 << (KEY_D - KEY_BASE))
#define _FIRE_MASK (1 << (KEY_J - KEY_BASE))
#define _PAUSE_MASK (1 << (KEY_P - KEY_BASE))
#define _START_MASK (1 << (KEY_G - KEY_BASE))
#define WINDOW_WIDTH_PX (1000)
#define WINDOW_HEIGHT_PX (600)
#define PLAYER_SPEED_XY (300)
#define PLAYER_SIZE_PX (WINDOW_HEIGHT_PX / 8)
#define PLAYER_MIN_FIRE_PERIOD_S (0.5)
#define PLAYER_MIN_POSITION_X (100)
#define PLAYER_MAX_POSITION_X (WINDOW_WIDTH_PX - PLAYER_MIN_POSITION_X)
#define SCROLL_MAX (WINDOW_WIDTH_PX)
#define SCROLL_SPEED (200)

typedef struct
{
    float x;
    float y;
} Vector2D;

typedef struct
{
    float x;
    float y;
    float z;
} Vector3D;

typedef struct
{
    float w;
    float h;
} Size2D;

typedef struct
{
    Vector3D position;
    Size2D size;
} Rect;

typedef struct
{
    Vector3D position;
    bool alive;
    bool animated;
} Entity;

typedef struct
{
    int window_height_px;
    int window_width_px;
    int player_size_px;
} EngineParams;

typedef struct
{
    bool player_left;
    bool player_right;
    bool player_up;
    bool player_down;
    bool player_fire;
    float player_fire_time;
    bool player_pause;
    bool prev_player_pause;
    bool player_start;
} PlayerAction;

typedef enum
{
    GAME_BEGIN,
    GAME_PAUSED,
    GAME_RUNNING,
    GAME_OVER,
} GameState;

GameState g_game_state       = GAME_BEGIN;
int g_keys_pressed           = 0;
PlayerAction g_player_action = {0};
Entity g_player              = {0};
float g_dt                   = 0;
bool g_prev_pause_pressed    = false;
int g_score                  = 0;
float g_scroll               = 0;

void jsLogVector3D(Vector3D);
void jsLogCStr(char*);
void jsLogInt(int);
void jsLogFloat(float);
float jsGetDt(void);
void jsSetEngineParams(EngineParams);
void jsUpdate(int, float, Vector3D);
void jsFire(void);

void engine_init(void)
{
    jsSetEngineParams((EngineParams){
        .window_height_px = WINDOW_HEIGHT_PX,
        .window_width_px  = WINDOW_WIDTH_PX,
        .player_size_px   = PLAYER_SIZE_PX,
    });
}

void engine_key_down(int key_code)
{
    if (key_code < KEY_BASE || key_code > KEY_MAX)
    {
        return;
    }
    g_keys_pressed |= 1 << (key_code - KEY_BASE);
}

void engine_key_up(int key_code)
{
    if (key_code < KEY_BASE || key_code > KEY_MAX)
    {
        return;
    }
    g_keys_pressed &= ~(1 << (key_code - KEY_BASE));
}

void __read_input(void)
{
    g_dt                         = jsGetDt();
    g_player_action.player_up    = (g_keys_pressed & _UP_MASK);
    g_player_action.player_down  = (g_keys_pressed & _DOWN_MASK);
    g_player_action.player_right = (g_keys_pressed & _RIGHT_MASK);
    g_player_action.player_left  = (g_keys_pressed & _LEFT_MASK);
    g_player_action.player_fire  = (g_keys_pressed & _FIRE_MASK);

    bool curr_pause_pressed = (g_keys_pressed & _PAUSE_MASK) && (g_game_state == GAME_RUNNING || g_game_state == GAME_PAUSED);
    if (!g_prev_pause_pressed && curr_pause_pressed)
    {
        g_player_action.prev_player_pause = !g_player_action.prev_player_pause;
    }
    g_prev_pause_pressed         = curr_pause_pressed;
    g_player_action.player_start = (g_keys_pressed & _START_MASK) && (g_game_state == GAME_BEGIN || g_game_state == GAME_OVER);
}

void __evolve(void)
{
    switch (g_game_state)
    {
    case GAME_BEGIN:
    case GAME_OVER:
        if (g_player_action.player_start)
        {
            g_game_state                     = GAME_RUNNING;
            g_player.position                = (Vector3D){.x = PLAYER_MIN_POSITION_X, .y = WINDOW_HEIGHT_PX / 2, .z = 0};
            g_player_action.player_start     = false;
            g_player_action.player_fire_time = 0.0;
            g_score                          = 0;
            jsUpdate(g_score, g_scroll, g_player.position);
        }
        break;
    case GAME_RUNNING:
        if (g_player_action.prev_player_pause)
        {
            g_game_state = GAME_PAUSED;
        }
        break;
    case GAME_PAUSED:
        if (!g_player_action.prev_player_pause)
        {
            g_game_state = GAME_RUNNING;
        }
        break;
    } // switch(g_game_state)
}

void __update_output(void)
{
    if (g_game_state == GAME_RUNNING)
    {
        g_player.position.x += g_dt * PLAYER_SPEED_XY * (float)(g_player_action.player_right - g_player_action.player_left);
        g_player.position.y += g_dt * PLAYER_SPEED_XY * (float)(g_player_action.player_up - g_player_action.player_down);
        g_scroll += g_dt * SCROLL_SPEED;

        if (g_player.position.x < PLAYER_MIN_POSITION_X)
        {
            g_player.position.x = PLAYER_MIN_POSITION_X;
        }
        if (g_player.position.x > PLAYER_MAX_POSITION_X)
        {
            g_player.position.x = PLAYER_MAX_POSITION_X;
        }
        if (g_scroll < 0)
        {
            // Infinite negative scroll
            g_scroll = SCROLL_MAX;
        }
        if (g_scroll > SCROLL_MAX)
        {
            // Infinite positive scroll
            g_scroll = 0;
        }
        if (g_player_action.player_fire)
        {
            if (g_player_action.player_fire_time == 0.0)
            {
                jsFire();
            }
            if (g_player_action.player_fire_time < PLAYER_MIN_FIRE_PERIOD_S)
            {
                g_player_action.player_fire_time += g_dt;
            }
            else
            {
                g_player_action.player_fire_time = 0.0;
            }
        }
        else
        {
            g_player_action.player_fire_time = 0.0;
        }
        jsUpdate(g_score, g_scroll, g_player.position);
    }
}

GameState engine_update(void)
{
    __read_input();
    __evolve();
    __update_output();
    return g_game_state;
}
