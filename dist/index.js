let memory = undefined;
let g_get_wall_rect = undefined;
let g_dt = 0;
let g_walls = [];
let g_window_height = 0;
let g_window_width = 0;
let g_fov_max_z = 0;
let g_fov_min_z = 0;
let g_num_of_walls = 0;
let g_player_size = 0;
let g_canvas = undefined;
let g_canvasBack = undefined;
let g_scoreValueDiv = undefined;
let g_beginViewDiv = undefined;
let g_overViewDiv = undefined;
let g_playerDiv = undefined;
let g_engine_update_cb = undefined;
let g_finalScore = 0;

const GameState = {
  BEGIN: 0,
  PAUSED: 1,
  RUNNING: 2,
  OVER: 3,
};

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
  [g_window_height, g_window_width, g_player_size] = new Int32Array(memory.buffer, params_p, 3);
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

function jsUpdateScore(score) {
  g_finalScore = score;
  g_scoreValueDiv.innerText = score;
}

function jsUpdatePlayerPosition(position_p) {
  const [x, y] = new Float32Array(memory.buffer, position_p, 2);
  g_playerDiv.style.left = `${x - g_player_size / 2}px`;
  g_playerDiv.style.bottom = `${y}px`;
}

function jsUpdateScroll(scroll) {
  g_canvas.scroll(scroll, 0);
  g_canvasBack.scroll(scroll / 2, 0);
}

function jsFire() {
  console.log("fire");
}

const importObj = {
  env: {
    jsLogVector3D,
    jsLogCStr,
    jsLogInt,
    jsLogFloat,
    jsGetDt,
    jsSetEngineParams,
    jsUpdateScore,
    jsUpdatePlayerPosition,
    jsUpdateScroll,
    jsFire,
  },
};

window.onload = () => {
  g_canvasBack = document.getElementById("canvas-back");
  g_canvas = document.getElementById("canvas");
  g_playerDiv = document.getElementById("player");
  const scoreDiv = document.getElementById("score");
  g_scoreValueDiv = document.getElementById("score-value");
  const body = document.getElementById("body");
  g_beginViewDiv = document.getElementById("game-begin-view");
  g_overViewDiv = document.getElementById("game-over-view");
  const scrollerBackDiv = document.getElementById("scroller-back");
  scrollerBackDiv.style.position = "static";
  scrollerBackDiv.style.height = "100%";
  scrollerBackDiv.style.backgroundRepeat = "no-repeat";
  scrollerBackDiv.style.backgroundImage = "url(/assets/clouds.jpg)";
  scrollerBackDiv.style.backgroundSize = "100% 100%";

  const scrollerDiv = document.getElementById("scroller");
  scrollerDiv.style.position = "static";
  scrollerDiv.style.height = "100%";
  scrollerDiv.style.backgroundImage = "url(/assets/mountains.png)";
  scrollerDiv.style.backgroundRepeat = "no-repeat";
  scrollerDiv.style.backgroundSize = "100% 100%";

  body.style.backgroundColor = "#101010";
  body.style.overflow = "hidden";
  g_beginViewDiv.style.position = "relative";
  g_canvasBack.style.position = "absolute";
  g_canvasBack.style.overflow = "hidden";
  g_canvas.style.position = "absolute";
  g_canvas.style.overflow = "hidden";
  g_beginViewDiv.style.width = "100%";
  g_beginViewDiv.style.height = "100%";
  g_beginViewDiv.style.backgroundColor = "blue";
  g_beginViewDiv.style.color = "white";
  g_beginViewDiv.style.justifyContent = "center";
  g_beginViewDiv.style.alignItems = "center";
  g_beginViewDiv.style.zIndex = 100000;
  g_beginViewDiv.style.fontSize = "xx-large";
  g_beginViewDiv.style.fontFamily = "monospace";

  g_overViewDiv.style.position = "relative";
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

  scoreDiv.style.position = "fixed";
  scoreDiv.style.display = "block";
  scoreDiv.style.zIndex = 99999;
  scoreDiv.style.fontFamily = "monospace";

  g_playerDiv.style.position = "sticky";
  g_playerDiv.style.zIndex = 100000;
  g_playerDiv.style.backgroundImage = "url(/assets/player.png)";
  g_playerDiv.style.backgroundSize = "contain";

  WebAssembly.instantiateStreaming(wasmFile, importObj).then((result) => {
    memory = result.instance.exports.memory;
    result.instance.exports.engine_init();

    for (let i = 0; i < g_num_of_walls; i++) {
      g_walls.push(new Wall());
    }
    scrollerBackDiv.style.width = `${2 * g_window_width}px`;
    scrollerDiv.style.width = `${2 * g_window_width}px`;
    g_canvas.style.width = `${g_window_width}px`;
    g_canvas.style.height = `${g_window_height}px`;
    g_canvas.style.top = `calc(50% - ${g_window_height / 2}px)`;
    g_canvas.style.left = `calc(50% - ${g_window_width / 2}px)`;
    g_canvasBack.style.width = `${g_window_width}px`;
    g_canvasBack.style.height = `${g_window_height}px`;
    g_canvasBack.style.top = `calc(50% - ${g_window_height / 2}px)`;
    g_canvasBack.style.left = `calc(50% - ${g_window_width / 2}px)`;
    g_playerDiv.style.width = `${g_player_size}px`;
    g_playerDiv.style.height = `${g_player_size}px`;
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
