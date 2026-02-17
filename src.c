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
#define ENEMY_WIDTH_PX (100)
#define ENEMY_HEIGHT_PX (37)
#define PLAYER_SPEED_XY (300)
#define PLAYER_SIZE_PX (WINDOW_HEIGHT_PX / 8)
#define PLAYER_FIRE_HALF_PULSE_S (0.2)
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
    Vector2D position;
} Player;

typedef struct
{
    int window_height_px;
    int window_width_px;
    int player_size_px;
    int enemy_height_px;
    int enemy_width_px;
    int number_of_enimies;
} EngineParams;

typedef struct
{
    bool player_left;
    bool player_right;
    bool player_up;
    bool player_down;
    bool player_fire;
    bool player_pause;
    bool prev_player_pause;
    bool player_start;
} PlayerAction;

typedef enum
{
    ENEMY_WAITING,
    ENEMY_ALIVE,
    ENEMY_DEAD,
} EnemyState;

typedef struct
{
    int show_time;
    EnemyState state;
    Vector2D position;
} Enemy;

typedef struct
{
    bool on;
    bool hit;
    float fire_time;
} Laser;

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
Player g_player              = {0};
float g_dt                   = 0;
bool g_prev_pause_pressed    = false;
int g_score                  = 0;
float g_scroll               = 0;
Enemy g_enemy_list[]         = {
    (Enemy){.show_time = 2, .state = ENEMY_WAITING, .position = (Vector2D){.x = WINDOW_WIDTH_PX - 100, .y = 100}},
    (Enemy){.show_time = 2, .state = ENEMY_WAITING, .position = (Vector2D){.x = WINDOW_WIDTH_PX - 100, .y = 400}},
    (Enemy){.show_time = 4, .state = ENEMY_WAITING, .position = (Vector2D){.x = WINDOW_WIDTH_PX - 100, .y = 100}},
    (Enemy){.show_time = 4, .state = ENEMY_WAITING, .position = (Vector2D){.x = WINDOW_WIDTH_PX - 100, .y = 400}},
    (Enemy){.show_time = 5, .state = ENEMY_WAITING, .position = (Vector2D){.x = WINDOW_WIDTH_PX - 100, .y = 100}},
    (Enemy){.show_time = 5, .state = ENEMY_WAITING, .position = (Vector2D){.x = WINDOW_WIDTH_PX - 100, .y = 400}},
};
Laser g_laser = {0};

void jsLogVector3D(Vector3D);
void jsLogCStr(char*);
void jsLogInt(int);
void jsLogFloat(float);
float jsGetDt(void);
void jsSetEngineParams(EngineParams);
void jsUpdate(int, float, Vector2D);
void jsUpdateEnemy(int, int, Vector2D);
void jsFire(float);

void engine_init(void)
{
    jsSetEngineParams((EngineParams){
        .window_height_px  = WINDOW_HEIGHT_PX,
        .window_width_px   = WINDOW_WIDTH_PX,
        .player_size_px    = PLAYER_SIZE_PX,
        .enemy_height_px   = ENEMY_HEIGHT_PX,
        .enemy_width_px    = ENEMY_WIDTH_PX,
        .number_of_enimies = sizeof(g_enemy_list) / sizeof(g_enemy_list[0]),
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

void __evolve_enemies(void)
{
    static float total_time = 0;
    total_time += g_dt;
    Enemy* curr_enemy_p = nullptr;
    for (unsigned long enemy_index = 0; enemy_index < sizeof(g_enemy_list) / sizeof(g_enemy_list[0]); enemy_index++)
    {
        curr_enemy_p = &g_enemy_list[enemy_index];
        if (curr_enemy_p->show_time > total_time || curr_enemy_p->state == ENEMY_DEAD)
        {
            continue;
        }

        if (curr_enemy_p->position.x > 0)
        {
            curr_enemy_p->state = ENEMY_ALIVE;
            curr_enemy_p->position.x -= g_dt * PLAYER_SPEED_XY / 2;
            float dx = curr_enemy_p->position.x - g_player.position.x;
            float dy = curr_enemy_p->position.y - g_player.position.y;
            // Player attracts enemies when close enough
            if (dx > 0 && dx < (PLAYER_SIZE_PX * 3))
            {
                if (dy < 0)
                {
                    curr_enemy_p->position.y += g_dt * PLAYER_SPEED_XY / 3.0;
                }
                else if (dy > 0)
                {
                    curr_enemy_p->position.y -= g_dt * PLAYER_SPEED_XY / 3.0;
                }
            }

            if (dx > 0
                && dy > -(ENEMY_HEIGHT_PX / 2 + 10)
                && dy < (ENEMY_HEIGHT_PX / 2 + 10)
                && g_laser.on && !g_laser.hit)
            {
                curr_enemy_p->state = ENEMY_DEAD;
                g_laser.hit         = true;
                g_score++;
            }
        }
        else
        {
            curr_enemy_p->state = ENEMY_DEAD;
        }

        jsUpdateEnemy(enemy_index, curr_enemy_p->state, curr_enemy_p->position);
    }
}

void __evolve_fire(void)
{
    if (g_player_action.player_fire || g_laser.fire_time > 0.0)
    {
        if (g_laser.fire_time <= PLAYER_FIRE_HALF_PULSE_S)
        {
            jsFire(g_laser.fire_time / PLAYER_FIRE_HALF_PULSE_S);
            g_laser.on = true;
        }
        else if (g_laser.fire_time <= 2 * PLAYER_FIRE_HALF_PULSE_S)
        {
            jsFire(2 - g_laser.fire_time / PLAYER_FIRE_HALF_PULSE_S);
        }
        else
        {
            jsFire(0);
            g_laser.on = false;
        }
        if (g_laser.fire_time < PLAYER_MIN_FIRE_PERIOD_S)
        {
            g_laser.fire_time += g_dt;
        }
        else
        {
            g_laser.fire_time = 0.0;
            g_laser.hit       = false;
        }
    }
    else
    {
        g_laser.fire_time = 0.0;
        g_laser.hit       = false;
    }
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

void __evolve_player(void)
{
    g_player.position.x += g_dt * PLAYER_SPEED_XY * (float)(g_player_action.player_right - g_player_action.player_left);
    g_player.position.y += g_dt * PLAYER_SPEED_XY * (float)(g_player_action.player_up - g_player_action.player_down);

    if (g_player.position.x < PLAYER_MIN_POSITION_X)
    {
        g_player.position.x = PLAYER_MIN_POSITION_X;
    }
    if (g_player.position.x > PLAYER_MAX_POSITION_X)
    {
        g_player.position.x = PLAYER_MAX_POSITION_X;
    }
}

void __evolve_scroll(void)
{
    g_scroll += g_dt * SCROLL_SPEED;
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
}

void __evolve(void)
{
    switch (g_game_state)
    {
    case GAME_BEGIN:
    case GAME_OVER:
        if (g_player_action.player_start)
        {
            g_game_state                 = GAME_RUNNING;
            g_player.position            = (Vector2D){.x = PLAYER_MIN_POSITION_X, .y = WINDOW_HEIGHT_PX / 2};
            g_player_action.player_start = false;
            g_laser.fire_time            = 0.0;
            g_score                      = 0;
        }
        break;
    case GAME_RUNNING:
        if (g_player_action.prev_player_pause)
        {
            g_game_state = GAME_PAUSED;
        }
        else
        {
            __evolve_scroll();
            __evolve_player();
            __evolve_fire();
            __evolve_enemies();
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
