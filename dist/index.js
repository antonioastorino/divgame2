let memory = undefined;
let g_get_wall_rect = undefined;
let g_dt = 0;
let g_enemies = [];
let g_window_height = 0;
let g_window_width = 0;
let g_fov_max_z = 0;
let g_fov_min_z = 0;
let g_player_size = 0;
let g_enemy_height = 0;
let g_enemy_width = 0;
let g_num_of_enemies = 0;
let g_canvas = undefined;
let g_scrollerBackContainer = undefined;
let g_scrollerFrontContainer = undefined;
let g_scoreValueDiv = undefined;
let g_beginViewDiv = undefined;
let g_overViewDiv = undefined;
let g_playerDiv = undefined;
let g_fireLaserDiv = undefined;
let g_engine_update_cb = undefined;
let g_finalScore = 0;

const GameState = {
  BEGIN: 0,
  PAUSED: 1,
  RUNNING: 2,
  OVER: 3,
};

const EnemyState = {
  WAITING: 0,
  ALIVE: 1,
  DEAD: 2,
};

class Enemy {
  __enemyDiv = undefined;
  constructor() {
    this.__enemyDiv = document.createElement("div");
    g_canvas.appendChild(this.__enemyDiv);
    this.__enemyDiv.style.backgroundImage = "url(/assets/rocket.png)";
    this.__enemyDiv.style.backgroundSize = "100%";
    this.__enemyDiv.style.position = "absolute";
    this.__enemyDiv.style.width = `${g_enemy_width}px`;
    this.__enemyDiv.style.height = `${g_enemy_height}px`;
    this.hide();
  }

  hide() {
    this.__enemyDiv.style.display = "none";
  }

  show(x, y) {
    this.__enemyDiv.style.display = "block";
    this.__enemyDiv.style.left = `${x - g_enemy_width / 2}px`;
    this.__enemyDiv.style.bottom = `${y - g_enemy_height / 2}px`;
  }
}

const wasmFile = fetch("./src.wasm");

const jsLogCStr = (str_p) => {
  const buffer = new Uint8Array(memory.buffer, str_p);
  let string = "string: '";
  for (let i = 0; buffer[i] != 0; i++) {
    string += String.fromCharCode(buffer[i]);
  }
  string += "'";
  console.log(string);
};

const jsLogVector3D = (v_p) => {
  const buffer = new Float32Array(memory.buffer, v_p, 3);
  console.log(`vector3d: (${buffer[0]}, ${buffer[1]}, ${buffer[2]})`);
};

const jsLogInt = (v) => {
  console.log(`int: ${v}`);
};

const jsLogFloat = (v) => {
  console.log(`float: ${v}`);
};

function jsSetEngineParams(params_p) {
  [g_window_height, g_window_width, g_player_size, g_enemy_height, g_enemy_width, g_num_of_enemies] = new Int32Array(
    memory.buffer,
    params_p,
    6
  );
}

let prevTimeStamp = 0;
function nextFrame(t_ms) {
  const gameState = g_engine_update_cb();
  switch (gameState) {
    case GameState.BEGIN:
      g_beginViewDiv.style.display = "flex";
      g_dt = 0;
      break;
    case GameState.OVER:
      g_overViewDiv.style.display = "flex";
      document.getElementById("final-score").innerText = g_finalScore.toFixed(2);
      g_dt = 0;
      break;
    case GameState.RUNNING:
      if (prevTimeStamp == 0) {
        prevTimeStamp = t_ms;
      }
      g_dt = (t_ms - prevTimeStamp) / 1000;
      if (g_dt > 0.05) {
        g_dt = 0.05;
      }
      prevTimeStamp = t_ms;
      g_beginViewDiv.style.display = "none";
      g_overViewDiv.style.display = "none";
      break;
    case GameState.PAUSED:
      g_dt = 0;
      break;
  }
  requestAnimationFrame(nextFrame);
}

function jsGetDt() {
  return g_dt;
}

function jsUpdate(score, scroll, position_p) {
  g_finalScore = score;
  g_scoreValueDiv.innerText = score;

  g_scrollerFrontContainer.scroll(scroll, 0);
  g_scrollerBackContainer.scroll(scroll / 2, 0);

  const [x, y] = new Float32Array(memory.buffer, position_p, 2);
  g_playerDiv.style.left = `${x - g_player_size / 2}px`;
  g_playerDiv.style.bottom = `${y - g_player_size / 2}px`;
  g_fireLaserDiv.style.left = `${x + g_player_size / 2}px`;
  g_fireLaserDiv.style.bottom = `${y - 10}px`;
}

function jsUpdateEnemy(index, state, position_p) {
  if (state != EnemyState.ALIVE) {
    g_enemies[index].hide();
    return;
  }
  const [x, y] = new Float32Array(memory.buffer, position_p, 2);
  g_enemies[index].show(x, y);
}

function jsFire(fire) {
  g_fireLaserDiv.style.opacity = fire;
}

const importObj = {
  env: {
    jsLogVector3D,
    jsLogCStr,
    jsLogInt,
    jsLogFloat,
    jsGetDt,
    jsSetEngineParams,
    jsUpdate,
    jsUpdateEnemy,
    jsFire,
  },
};

window.onload = () => {
  g_canvas = document.getElementById("canvas");
  const scrollerFront = document.getElementById("scroller-front");
  const scrollerBack = document.getElementById("scroller-back");
  const scoreDiv = document.getElementById("score");
  const body = document.getElementById("body");

  g_scrollerFrontContainer = document.getElementById("scroller-front-container");
  g_scrollerBackContainer = document.getElementById("scroller-back-container");
  g_playerDiv = document.getElementById("player");
  g_fireLaserDiv = document.getElementById("fire-laser");
  g_scoreValueDiv = document.getElementById("score-value");
  g_beginViewDiv = document.getElementById("game-begin-view");
  g_overViewDiv = document.getElementById("game-over-view");

  scrollerBack.style.position = "absolute";
  scrollerBack.style.height = "100%";
  scrollerBack.style.backgroundRepeat = "no-repeat";
  scrollerBack.style.backgroundImage = "url(/assets/clouds.jpg)";
  scrollerBack.style.backgroundSize = "100% 100%";

  scrollerFront.style.position = "absolute";
  scrollerFront.style.height = "100%";
  scrollerFront.style.backgroundImage = "url(/assets/mountains.png)";
  scrollerFront.style.backgroundRepeat = "no-repeat";
  scrollerFront.style.backgroundSize = "100% 100%";

  body.style.display = "flex";
  body.style.position = "absolute";
  body.style.justifyContent = "center";
  body.style.alignItems = "center";
  body.style.width = "100%";
  body.style.height = "100%";
  body.style.backgroundColor = "#101010";
  body.style.overflow = "hidden";

  g_canvas.style.display = "block";
  g_canvas.style.position = "absolute";
  g_canvas.style.overflow = "hidden";

  g_beginViewDiv.style.position = "absolute";
  g_scrollerFrontContainer.style.position = "absolute";
  g_scrollerFrontContainer.style.overflow = "hidden";
  g_scrollerFrontContainer.style.width = "100%";
  g_scrollerFrontContainer.style.height = "100%";
  g_scrollerBackContainer.style.position = "absolute";
  g_scrollerBackContainer.style.overflow = "hidden";
  g_scrollerBackContainer.style.width = "100%";
  g_scrollerBackContainer.style.height = "100%";

  g_beginViewDiv.style.width = "100%";
  g_beginViewDiv.style.height = "100%";
  g_beginViewDiv.style.backgroundColor = "blue";
  g_beginViewDiv.style.color = "white";
  g_beginViewDiv.style.justifyContent = "center";
  g_beginViewDiv.style.alignItems = "center";
  g_beginViewDiv.style.zIndex = 100000;
  g_beginViewDiv.style.fontSize = "xx-large";
  g_beginViewDiv.style.fontFamily = "monospace";

  g_overViewDiv.style.position = "absolute";
  g_overViewDiv.style.width = "100%";
  g_overViewDiv.style.height = "100%";
  g_overViewDiv.style.backgroundColor = "red";
  g_overViewDiv.style.opacity = 0.7;
  g_overViewDiv.style.color = "white";
  g_overViewDiv.style.justifyContent = "center";
  g_overViewDiv.style.alignItems = "center";
  g_overViewDiv.style.zIndex = 100000;
  g_overViewDiv.style.fontSize = "xx-large";
  g_overViewDiv.style.fontFamily = "monospace";
  g_overViewDiv.style.textAlign = "center";
  g_overViewDiv.style.display = "none";

  scoreDiv.style.position = "fixed";
  scoreDiv.style.display = "block";
  scoreDiv.style.zIndex = 99999;
  scoreDiv.style.fontFamily = "monospace";

  g_playerDiv.style.position = "absolute";
  g_playerDiv.style.zIndex = 100000;
  g_playerDiv.style.backgroundImage = "url(/assets/player.png)";
  g_playerDiv.style.backgroundSize = "contain";

  g_fireLaserDiv.style.position = "absolute";
  g_fireLaserDiv.style.zIndex = 100000;
  g_fireLaserDiv.style.height = "20px";
  g_fireLaserDiv.style.backgroundImage = "linear-gradient(rgba(255, 0, 0, 0), rgba(255, 0, 0, 1), rgba(255, 0, 0, 0))";
  g_fireLaserDiv.style.animationProperty = "opacity";
  g_fireLaserDiv.style.opacity = 0;

  WebAssembly.instantiateStreaming(wasmFile, importObj).then((result) => {
    memory = result.instance.exports.memory;
    result.instance.exports.engine_init();

    for (let i = 0; i < g_num_of_enemies; i++) {
      g_enemies.push(new Enemy());
    }
    scrollerFront.style.width = `${2 * g_window_width}px`;
    scrollerBack.style.width = `${2 * g_window_width}px`;
    g_canvas.style.width = `${g_window_width}px`;
    g_canvas.style.height = `${g_window_height}px`;
    g_playerDiv.style.width = `${g_player_size}px`;
    g_playerDiv.style.height = `${g_player_size}px`;
    g_fireLaserDiv.style.width = `${g_window_width}px`;
    body.onkeydown = (ev) => {
      ev.preventDefault();
      result.instance.exports.engine_key_down(ev.keyCode);
    };
    body.onkeyup = (ev) => {
      ev.preventDefault();
      result.instance.exports.engine_key_up(ev.keyCode);
    };
    g_engine_update_cb = result.instance.exports.engine_update;
    requestAnimationFrame(nextFrame);
  });
};
