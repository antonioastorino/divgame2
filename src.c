#define KEY_BASE (65)
#define KEY_MAX (90)
#define KEY_W (87)
#define KEY_A (65)
#define KEY_S (83)
#define KEY_D (68)
#define KEY_P (80)
#define KEY_G (71)
#define _UP_MASK (1 << (KEY_W - KEY_BASE))
#define _LEFT_MASK (1 << (KEY_A - KEY_BASE))
#define _DOWN_MASK (1 << (KEY_S - KEY_BASE))
#define _RIGHT_MASK (1 << (KEY_D - KEY_BASE))
#define _PAUSE_MASK (1 << (KEY_P - KEY_BASE))
#define _START_MASK (1 << (KEY_G - KEY_BASE))
#define WINDOW_WIDTH_PX (1000)
#define WINDOW_HEIGHT_PX (600)

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
    Vector3D speed;
    bool alive;
    bool animated;
} Entity;

typedef struct
{
    int window_height_px;
    int window_width_px;
} EngineParams;

typedef struct
{
    int ticks;
    int inc_x;
    int inc_y;
} PathElement;

typedef struct
{
    bool player_left;
    bool player_right;
    bool player_up;
    bool player_down;
    bool player_pause;
    bool player_start;
} PlayerAction;

typedef enum
{
    GAME_BEGIN,
    GAME_PAUSED,
    GAME_RUNNING,
    GAME_OVER,
} GameState;

GameState g_game_state = GAME_BEGIN;
int g_keys_pressed     = 0;

void jsLogVector3D(Vector3D);
void jsLogCStr(char*);
void jsLogInt(int);
void jsLogFloat(float);
float jsGetDt(void);
void jsSetEngineParams(EngineParams);

void engine_init(void)
{
    jsSetEngineParams((EngineParams){
        .window_height_px = WINDOW_HEIGHT_PX,
        .window_width_px  = WINDOW_WIDTH_PX,
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
}

void __evolve(void)
{
}

void __update_output(void)
{
}

GameState engine_update(void)
{
    __read_input();
    __evolve();
    __update_output();
    return g_game_state;
}
