// Platformer Game
static int score = 0;
static bool init = false;
static bool title_screen = true;
static bool game_over_screen = false;
static bool level_complete_screen = false;
static int next_world = 1;
static int next_level = 1;
static int title_blink = 0;
static bool big = false;
static bool fire = false;
static int mario_facing = 1;
static int walk_frame = 0;
static bool mario_dead = false;
static int death_timer = 0;
static float death_vy = 0;
static int lives = 3;
static int coins_collected = 0;
static int world = 1;
static int level = 1;
static int timer = 300;
static unsigned long last_time = 0;
static int level_seed = 0;
static const int LEVEL_LENGTH = 2000;
static const int FLAG_X = 1900;
static bool flag_reached = false;
static int flag_y = 20;
static int flag_anim_timer = 0;
static float cloud_x[5] = {30, 80, 150, 220, 300};
static int cloud_y[5] = {16, 14, 18, 15, 17};
static int bush_x[8] = {50, 120, 180, 250, 320, 400, 480, 550};
static int particle_x[10], particle_y[10], particle_vx[10], particle_vy[10], particle_timer[10];
static int particle_count = 0;
static int score_popup_x[5], score_popup_y[5], score_popup_val[5], score_popup_timer[5];
static int score_popup_count = 0;
static bool fireballs[5];
static float fireball_x[5], fireball_y[5], fireball_vx[5], fireball_vy[5];
static int fireball_timer[5];
static int fireball_count = 0;
static int firework_x[20], firework_y[20], firework_vx[20], firework_vy[20], firework_timer[20], firework_exploded[20];
static int firework_count = 0;
static int explosion_x[30], explosion_y[30], explosion_vx[30], explosion_vy[30], explosion_timer[30];
static int explosion_count = 0;

struct Platform { int x, y, w, h; };
struct Pipe { int x, h; };
struct Hole { int x, w; };
static Platform platforms[30];
static Pipe pipes[15];
static bool pipe_has_plant[15];
static float plant_height[15];
static int plant_dir[15];
static Hole holes[10];
static int platform_count = 0;
static int pipe_count = 0;
static int hole_count = 0;

static bool coins[40];
static int coin_x[40], coin_y[40];
static int coin_count = 0;
static bool mushroom_spawned = false;
static bool flower_spawned = false;
static bool question_hit = false;
static float flower_x = 0, flower_y = 0;
static bool bricks[50];
static int brick_x[50], brick_y[50], brick_item[50];
static int brick_count = 0;
static float mushroom_x = 0, mushroom_y = 0;
static bool goombas[15];
static float goomba_x[15], goomba_y[15], goomba_vy[15], goomba_start_x[15];
static int goomba_dir[15];
static int goomba_count = 0;
static bool koopas[10];
static float koopa_x[10], koopa_y[10], koopa_vy[10], koopa_start_x[10];
static int koopa_dir[10];
static bool koopa_shell[10];
static int koopa_count = 0;

#ifdef ESPHOMARIO_GAME_LOGIC
// Level Complete screen
if (level_complete_screen) {
  title_blink++;
  it.filled_rectangle(0, 0, 128, 64, COLOR_OFF);
  it.printf(64, 16, get_font_md(), TextAlign::CENTER, "LEVEL");
  it.printf(64, 28, get_font_md(), TextAlign::CENTER, "COMPLETE!");
  it.printf(64, 44, get_font_sm(), TextAlign::CENTER, "World %d-%d", next_world, next_level);
  if ((title_blink / 30) % 2 == 0) {
    it.printf(64, 56, get_font_xxs(), TextAlign::CENTER, "PRESS JUMP");
  }
  if (get_btn_down_held()) {
    level_complete_screen = false;
    world = next_world;
    level = next_level;
    init = false;
  }
  return;
}

// Game Over screen
if (game_over_screen) {
  title_blink++;
  it.filled_rectangle(0, 0, 128, 64, COLOR_OFF);
  it.printf(64, 20, get_font_md(), TextAlign::CENTER, "GAME OVER");
  it.printf(64, 34, get_font_sm(), TextAlign::CENTER, "Score: %d", score);
  if ((title_blink / 30) % 2 == 0) {
    it.printf(64, 50, get_font_xxs(), TextAlign::CENTER, "PRESS JUMP");
  }
  if (get_btn_down_held()) {
    game_over_screen = false;
    title_screen = true;
    score = 0;
    lives = 3;
    world = 1;
    level = 1;
    stop_music();
  }
  return;
}

// Title screen
if (title_screen) {
  title_blink++;
  it.filled_rectangle(0, 0, 128, 64, COLOR_OFF);
  it.printf(64, 10, get_font_md(), TextAlign::CENTER, "SUPER");
  it.printf(64, 22, get_font_md(), TextAlign::CENTER, "ESPHOMARIO");
  it.printf(64, 36, get_font_sm(), TextAlign::CENTER, "BROS.");
  if ((title_blink / 30) % 2 == 0) {
    it.printf(64, 50, get_font_xxs(), TextAlign::CENTER, "PRESS JUMP");
  }
  // Draw Mario sprite
  it.filled_rectangle(56, 42, 4, 1);
  it.filled_rectangle(55, 43, 6, 2);
  it.draw_pixel_at(56, 43, COLOR_OFF);
  it.draw_pixel_at(59, 43, COLOR_OFF);
  it.filled_rectangle(56, 45, 4, 2);
  it.filled_rectangle(55, 47, 6, 3);
  it.line(57, 47, 57, 49);
  it.line(58, 47, 58, 49);
  // Check for jump button
  if (get_btn_down_held()) {
    title_screen = false;
    game_over_screen = false;
    level_complete_screen = false;
    score = 0;
    lives = 3;
    world = 1;
    level = 1;
  }
  return;
}

if (!init) {
  set_mario_x(20); set_mario_y(40); set_mario_vy(0); set_mario_scroll(0);
  init = true; big = false; fire = false; mario_dead = false; death_timer = 0; title_blink = 0; mushroom_spawned = false; flower_spawned = false; question_hit = false;
  fireball_count = 0;
  timer = 300; last_time = millis();
  level_seed++;
  flag_reached = false; flag_y = 20; flag_anim_timer = 0;
  particle_count = 0;
  play_music(1);
  
  // Generate level
  platform_count = 0; pipe_count = 0; hole_count = 0; coin_count = 0; brick_count = 0; goomba_count = 0; koopa_count = 0;
  int x = 100;
  unsigned int rng = level_seed * 12345;
  
  while (x < LEVEL_LENGTH - 200) {
    rng = (rng * 1103515245 + 12345);
    int type = (rng >> 16) % 100;
    
    if (type < 20 && pipe_count < 15) {
      pipes[pipe_count].x = x;
      pipes[pipe_count].h = 12 + (rng % 3) * 4;
      pipe_has_plant[pipe_count] = (rng % 3) == 0; // 33% chance
      plant_height[pipe_count] = 0;
      plant_dir[pipe_count] = 1;
      pipe_count++;
      x += 24 + (rng % 30);
    } else if (type < 35 && hole_count < 10) {
      holes[hole_count].x = x;
      holes[hole_count].w = 24 + (rng % 3) * 8;
      hole_count++;
      x += holes[hole_count-1].w + 16;
    } else if (type < 55 && brick_count < 48) {
      // Create platform from bricks
      int pw = 16 + (rng % 4) * 8;
      int py = 30 - (rng % 3) * 6;
      for (int bx = 0; bx < pw && brick_count < 48; bx += 8) {
        brick_x[brick_count] = x + bx;
        brick_y[brick_count] = py;
        bricks[brick_count] = true;
        int item_rng = (rng >> (bx + 4)) % 20;
        brick_item[brick_count] = (item_rng < 15) ? 0 : (item_rng < 18) ? 1 : 2; // 75% nothing, 15% coin, 10% mushroom
        brick_count++;
      }
      x += pw + 8;
    } else {
      x += 20 + (rng % 25);
    }
    
    if ((rng % 8) < 3 && brick_count < 48) {
      int bx = x - 12;
      // Check if brick would be too close to a pipe
      bool too_close_to_pipe = false;
      for (int p = 0; p < pipe_count; p++) {
        if (bx > pipes[p].x - 20 && bx < pipes[p].x + 20) {
          too_close_to_pipe = true;
          break;
        }
      }
      if (!too_close_to_pipe) {
        brick_x[brick_count] = bx;
        brick_y[brick_count] = 28;
        bricks[brick_count] = true;
        int item_rng = (rng >> 8) % 10;
        brick_item[brick_count] = (item_rng < 4) ? 1 : (item_rng < 7) ? 2 : 3; // 40% coin, 30% mushroom, 30% flower
        brick_count++;
      }
    }
    if ((rng % 7) < 3 && coin_count < 40) {
      coin_x[coin_count] = x - 8;
      coin_y[coin_count] = 25 + (rng % 4) * 5;
      coins[coin_count] = true;
      coin_count++;
    }
    if ((rng % 12) < 3 && (goomba_count < 15 || koopa_count < 10)) {
      bool spawn_koopa = (rng % 2) == 0 && koopa_count < 10;
      // Check if goomba would spawn inside a pipe
      bool inside_pipe = false;
      for (int p = 0; p < pipe_count; p++) {
        if (x + 6 > pipes[p].x && x < pipes[p].x + 16) {
          inside_pipe = true;
          break;
        }
      }
      // Check if goomba would spawn over a hole
      bool over_hole = false;
      for (int h = 0; h < hole_count; h++) {
        if (x + 3 > holes[h].x && x + 3 < holes[h].x + holes[h].w) {
          over_hole = true;
          break;
        }
      }
      if (!inside_pipe && !over_hole) {
        if (spawn_koopa) {
          koopa_x[koopa_count] = x;
          koopa_y[koopa_count] = 48;
          koopa_vy[koopa_count] = 0;
          koopa_start_x[koopa_count] = x;
          koopas[koopa_count] = true;
          koopa_shell[koopa_count] = false;
          koopa_dir[koopa_count] = -1;
          koopa_count++;
        } else if (goomba_count < 15) {
          goomba_x[goomba_count] = x;
          goomba_y[goomba_count] = 48;
          goomba_vy[goomba_count] = 0;
          goomba_start_x[goomba_count] = x;
          goombas[goomba_count] = true;
          goomba_dir[goomba_count] = -1;
          goomba_count++;
        }
      }
    }
  }
  
  // Reposition bushes to avoid holes
  for (int i = 0; i < 8; i++) {
    bool over_hole = false;
    for (int h = 0; h < hole_count; h++) {
      if (bush_x[i] >= holes[h].x - 16 && bush_x[i] <= holes[h].x + holes[h].w) {
        over_hole = true;
        break;
      }
    }
    if (over_hole) bush_x[i] = (i * 80) + 50;
  }
}

// Timer
if (millis() - last_time > 1000) {
  last_time = millis();
  timer--;
  if (timer <= 0) {
    lives--;
    if (lives <= 0) { game_over_screen = true; lives = 3; }
    init = false;
    return;
  }
}

float mx = get_mario_x(), my = get_mario_y(), vy = get_mario_vy(), scroll = get_mario_scroll();
int mh = big ? 12 : 8;
bool on_ground = false;
bool on_pipe = false;
bool over_hole = false;
int qx = 200 - (int)scroll, qy = 32;

// Death sequence
if (mario_dead) {
  death_timer++;
  my += death_vy;
  death_vy += 0.3;
  if (death_timer == 1) {
    stop_music();
    play_music(3);
  }
  if (death_timer > 90) {
    if (lives <= 0) { game_over_screen = true; lives = 3; }
    init = false;
    return;
  }
  set_mario_y(my);
  // Skip rest of game logic during death
  goto draw_scene;
}

// Check flag
if (mx + scroll > FLAG_X && !flag_reached) {
  flag_reached = true;
  flag_anim_timer = 0;
  play_sfx("flagpole");
  stop_music();
  play_music(2);
  // Spawn random firework rockets (3-8)
  firework_count = 0;
  explosion_count = 0;
  int num_fireworks = 3 + (millis() % 6);
  for (int f = 0; f < num_fireworks && firework_count < 20; f++) {
    firework_x[firework_count] = FLAG_X - 20 + (millis() * (f + 1)) % 60;
    firework_y[firework_count] = 53;
    firework_vx[firework_count] = 0;
    firework_vy[firework_count] = -7 - (millis() * (f + 4)) % 4;
    firework_timer[firework_count] = 90;
    firework_exploded[firework_count] = 0;
    firework_count++;
  }
}

if (flag_reached) {
  flag_anim_timer++;
  if (flag_y < 48) flag_y++;
  if (flag_anim_timer > 120) {
    score += timer * 10;
    next_level = level + 1;
    next_world = world;
    if (next_level > 4) { next_level = 1; next_world++; }
    level_complete_screen = true;
    return;
  }
}

if (!flag_reached) {
  // Horizontal movement with run speed
  float old_mx = mx;
  float speed = get_btn_run_held() ? 2.5 : 1.5;
  if (get_btn_left_held()) { mx -= speed; mario_facing = -1; if (on_ground) walk_frame++; }
  if (get_btn_right_held()) { mx += speed; mario_facing = 1; if (on_ground) walk_frame++; }
  if (!get_btn_left_held() && !get_btn_right_held()) walk_frame = 0;
  
  // Check pipe horizontal collision for Mario
  for (int i = 0; i < pipe_count; i++) {
    int pipe_world_x = pipes[i].x;
    int py = 54 - pipes[i].h;
    if (mx + scroll + 6 > pipe_world_x && mx + scroll < pipe_world_x + 16 && my + mh > py) {
      mx = old_mx; // Block movement into pipe
      break;
    }
  }
  
  // Gravity
  vy += 0.35;
  my += vy;
}



// Pipe collision
for (int i = 0; i < pipe_count; i++) {
  int px = pipes[i].x - (int)scroll;
  int py = 54 - pipes[i].h;
  if (mx + 6 > px && mx < px + 16 && my + mh >= py && my + mh <= py + 4) {
    my = py - mh;
    vy = 0;
    on_ground = true;
    on_pipe = true;
  }
}

// Platform collision
for (int i = 0; i < platform_count; i++) {
  int px = platforms[i].x - (int)scroll;
  int py = platforms[i].y;
  int pw = platforms[i].w;
  int ph = platforms[i].h;
  
  if (mx + 6 > px && mx < px + pw && my + mh >= py && my + mh <= py + ph + 2 && vy >= 0) {
    my = py - mh;
    vy = 0;
    on_ground = true;
  }
  // Bottom collision - reject Mario downward
  if (mx + 6 > px && mx < px + pw && my < py + ph && my + mh > py + ph && vy < 0) {
    my = py + ph;
    vy = 0;
  }
}

// Ground and hole collision
over_hole = false;
for (int i = 0; i < hole_count; i++) {
  int hx = holes[i].x - (int)scroll;
  if (mx + 3 > hx && mx + 3 < hx + holes[i].w && my + mh >= 54) {
    over_hole = true;
    break;
  }
}

if (!over_hole && !on_pipe && my > 54 - mh) { my = 54 - mh; vy = 0; on_ground = true; }
if (over_hole && my > 64) { 
  if (!mario_dead) { mario_dead = true; death_timer = 0; death_vy = -5; lives--; }
}
if (my < 0) my = 0;

// Brick collision - standing on top and hitting from below
for (int i = 0; i < brick_count; i++) {
  if (bricks[i]) {
    int bx = brick_x[i] - (int)scroll;
    int by = brick_y[i];
    // Standing on top of brick
    if (mx + 6 > bx && mx < bx + 8 && my + mh >= by && my + mh <= by + 3 && vy >= 0) {
      my = by - mh;
      vy = 0;
      on_ground = true;
    }
    // Hitting brick from below
    if (vy < 0 && mx + 6 > bx && mx < bx + 8 && my <= by + 6 && my >= by) {
      if (big) { 
        bricks[i] = false; score += 50;
        play_sfx("brickbreak");
        // Spawn item from brick
        int item = brick_item[i];
        if (item == 1) { // Coin
          if (coin_count < 40) {
            coin_x[coin_count] = brick_x[i];
            coin_y[coin_count] = brick_y[i] - 8;
            coins[coin_count] = true;
            coin_count++;
          }
        } else if (item == 2) { // Mushroom
          if (!mushroom_spawned) {
            mushroom_spawned = true;
            mushroom_x = brick_x[i];
            mushroom_y = brick_y[i] - 8;
          }
        } else if (item == 3 && !fire) { // Flower (only if don't have fire)
          if (!flower_spawned && big) {
            flower_spawned = true;
            flower_x = brick_x[i];
            flower_y = brick_y[i] - 8;
          }
        }
        // Better particle effect
        for (int p = 0; p < 4 && particle_count < 10; p++) {
          particle_x[particle_count] = brick_x[i] + (p % 2) * 6;
          particle_y[particle_count] = brick_y[i] + (p / 2) * 4;
          particle_vx[particle_count] = (p % 2 == 0) ? -2 : 2;
          particle_vy[particle_count] = -4 - (p / 2);
          particle_timer[particle_count] = 40;
          particle_count++;
        }
      }
      vy = 0; my = by + 6;
    }
  }
}

// Question block collision - standing on top and hitting from below
if (mx + 6 > qx && mx < qx + 8) {
  // Standing on top of question block
  if (my + mh >= qy && my + mh <= qy + 3 && vy >= 0) {
    my = qy - mh;
    vy = 0;
    on_ground = true;
  }
  // Hitting question block from below
  if (!question_hit && vy < 0 && my <= qy + 6 && my >= qy) {
    question_hit = true;
    if (big && !fire) {
      flower_spawned = true;
      flower_x = 200; flower_y = qy - 6;
    } else if (!big) {
      mushroom_spawned = true;
      mushroom_x = 200; mushroom_y = qy - 6;
    }
    vy = 0; my = qy + 6;
  }
}

// Jump control after collision detection
if (!flag_reached && get_btn_down_held() && on_ground && vy >= 0) {
  vy = -4.2;
  play_sfx("jump");
}

// Shoot fireball with run button when fire power
static bool last_run = false;
if (!flag_reached && fire && get_btn_run_held() && !last_run && fireball_count < 5) {
  for (int i = 0; i < 5; i++) {
    if (!fireballs[i]) {
      fireballs[i] = true;
      fireball_x[i] = mx + scroll + (mario_facing > 0 ? 6 : -2);
      fireball_y[i] = my + 3;
      fireball_vx[i] = mario_facing * 2.0;
      fireball_vy[i] = -0.5;
      fireball_timer[i] = 120;
      fireball_count++;
      play_sfx("fireball");
      break;
    }
  }
}
last_run = get_btn_run_held();

if (mushroom_spawned && mushroom_y < 26) mushroom_y += 0.3;
if (flower_spawned && flower_y < 26) flower_y += 0.3;

// Smooth camera scrolling - starts earlier and follows more closely
if (mx > 50) {
  scroll += (mx - 50);
  mx = 50;
}
if (mx < 10) mx = 10;

if (!flag_reached) {
  for (int i = 0; i < goomba_count; i++) {
    if (goombas[i]) {
      float old_gx = goomba_x[i];
      goomba_x[i] += goomba_dir[i] * 0.5;
      
      // Apply gravity
      goomba_vy[i] += 0.2;
      goomba_y[i] += goomba_vy[i];
      
      // Ground collision
      bool goomba_over_hole = false;
      for (int h = 0; h < hole_count; h++) {
        if (goomba_x[i] + 3 > holes[h].x && goomba_x[i] + 3 < holes[h].x + holes[h].w) {
          goomba_over_hole = true;
          break;
        }
      }
      
      if (!goomba_over_hole && goomba_y[i] >= 48) {
        goomba_y[i] = 48;
        goomba_vy[i] = 0;
      }
      
      // Fall into holes
      if (goomba_y[i] > 64) {
        goombas[i] = false;
      }
      
      // Check for pipe collision
      bool hit_pipe = false;
      for (int p = 0; p < pipe_count; p++) {
        int pipe_x = pipes[p].x;
        int pipe_y = 54 - pipes[p].h;
        if (goomba_x[i] + 6 > pipe_x && goomba_x[i] < pipe_x + 16) {
          if (goomba_y[i] + 6 >= pipe_y) {
            goomba_x[i] = old_gx;
            goomba_dir[i] *= -1;
            hit_pipe = true;
            break;
          }
        }
      }
      
      if (!hit_pipe) {
        // Patrol range
        if (goomba_x[i] < goomba_start_x[i] - 30) goomba_dir[i] = 1;
        if (goomba_x[i] > goomba_start_x[i] + 30) goomba_dir[i] = -1;
      }
    }
  }
}

// Update fireballs
for (int i = 0; i < 5; i++) {
  if (fireballs[i]) {
    fireball_timer[i]--;
    if (fireball_timer[i] <= 0) {
      fireballs[i] = false;
      fireball_count--;
      continue;
    }
    fireball_x[i] += fireball_vx[i];
    fireball_y[i] += fireball_vy[i];
    fireball_vy[i] += 0.2;
    // Check if over hole
    bool fireball_over_hole = false;
    for (int h = 0; h < hole_count; h++) {
      if (fireball_x[i] + 2 > holes[h].x && fireball_x[i] + 2 < holes[h].x + holes[h].w) {
        fireball_over_hole = true;
        break;
      }
    }
    // Bounce on ground only if not over hole
    if (!fireball_over_hole && fireball_y[i] >= 52) {
      fireball_y[i] = 52;
      fireball_vy[i] = -1.2;
    }
    // Bounce off bricks
    for (int b = 0; b < brick_count; b++) {
      if (bricks[b]) {
        if (fireball_x[i] + 3 > brick_x[b] && fireball_x[i] < brick_x[b] + 8 &&
            fireball_y[i] + 3 > brick_y[b] && fireball_y[i] < brick_y[b] + 6) {
          if (fireball_vy[i] > 0) fireball_vy[i] = -1.2;
          break;
        }
      }
    }
    // Bounce off pipes
    for (int p = 0; p < pipe_count; p++) {
      int pipe_y = 54 - pipes[p].h;
      if (fireball_x[i] + 3 > pipes[p].x && fireball_x[i] < pipes[p].x + 16 &&
          fireball_y[i] + 3 > pipe_y && fireball_y[i] < 54) {
        // Horizontal bounce
        if (fireball_x[i] + 1 < pipes[p].x + 2 || fireball_x[i] + 2 > pipes[p].x + 14) {
          fireball_vx[i] *= -1;
        }
        // Vertical bounce
        if (fireball_y[i] + 1 < pipe_y + 2 && fireball_vy[i] > 0) {
          fireball_vy[i] = -1.2;
        }
        break;
      }
    }
    // Remove if off screen
    if (fireball_x[i] < scroll - 10 || fireball_x[i] > scroll + 140 || fireball_y[i] > 64) {
      fireballs[i] = false;
      fireball_count--;
    }
    // Check koopa collision
    for (int k = 0; k < koopa_count; k++) {
      if (koopas[k]) {
        if (fireball_x[i] + 3 > koopa_x[k] && fireball_x[i] < koopa_x[k] + 6 &&
            fireball_y[i] + 3 > koopa_y[k] && fireball_y[i] < koopa_y[k] + 6) {
          koopas[k] = false;
          fireballs[i] = false;
          fireball_count--;
          score += 100;
          for (int p = 0; p < 3 && particle_count < 10; p++) {
            particle_x[particle_count] = (int)koopa_x[k] + p * 2;
            particle_y[particle_count] = (int)koopa_y[k] + p;
            particle_vx[particle_count] = -1 + p;
            particle_vy[particle_count] = -2 - p;
            particle_timer[particle_count] = 25;
            particle_count++;
          }
          break;
        }
      }
    }
    // Check goomba collision
    for (int g = 0; g < goomba_count; g++) {
      if (goombas[g]) {
        if (fireball_x[i] + 3 > goomba_x[g] && fireball_x[i] < goomba_x[g] + 6 &&
            fireball_y[i] + 3 > goomba_y[g] && fireball_y[i] < goomba_y[g] + 6) {
          goombas[g] = false;
          fireballs[i] = false;
          fireball_count--;
          score += 100;
          // Add particle effect
          for (int p = 0; p < 3 && particle_count < 10; p++) {
            particle_x[particle_count] = (int)goomba_x[g] + p * 2;
            particle_y[particle_count] = (int)goomba_y[g] + p;
            particle_vx[particle_count] = -1 + p;
            particle_vy[particle_count] = -2 - p;
            particle_timer[particle_count] = 25;
            particle_count++;
          }
          break;
        }
      }
    }
    // Check piranha plant collision
    for (int p = 0; p < pipe_count; p++) {
      if (pipe_has_plant[p] && plant_height[p] > 0) {
        int plant_x = pipes[p].x + 8;
        int plant_y = 54 - pipes[p].h - (int)plant_height[p];
        if (fireball_x[i] + 3 > plant_x - 3 && fireball_x[i] < plant_x + 3 &&
            fireball_y[i] + 3 > plant_y && fireball_y[i] < plant_y + (int)plant_height[p]) {
          pipe_has_plant[p] = false;
          fireballs[i] = false;
          fireball_count--;
          score += 200;
          // Add particle effect
          for (int pt = 0; pt < 4 && particle_count < 10; pt++) {
            particle_x[particle_count] = plant_x + pt - 2;
            particle_y[particle_count] = plant_y + pt * 2;
            particle_vx[particle_count] = -2 + pt;
            particle_vy[particle_count] = -3 - pt;
            particle_timer[particle_count] = 30;
            particle_count++;
          }
          break;
        }
      }
    }
  }
}

// Update koopas
for (int i = 0; i < koopa_count; i++) {
  if (koopas[i]) {
    if (!koopa_shell[i]) {
      float old_kx = koopa_x[i];
      koopa_x[i] += koopa_dir[i] * 0.4;
      
      // Apply gravity
      koopa_vy[i] += 0.35;
      koopa_y[i] += koopa_vy[i];
      
      // Ground collision
      bool koopa_over_hole = false;
      for (int h = 0; h < hole_count; h++) {
        if (koopa_x[i] + 3 > holes[h].x && koopa_x[i] + 3 < holes[h].x + holes[h].w) {
          koopa_over_hole = true;
          break;
        }
      }
      if (!koopa_over_hole && koopa_y[i] >= 48) {
        koopa_y[i] = 48;
        koopa_vy[i] = 0;
      }
      if (koopa_y[i] > 64) koopas[i] = false;
      
      // Pipe collision
      for (int p = 0; p < pipe_count; p++) {
        if (koopa_x[i] + 6 > pipes[p].x && koopa_x[i] < pipes[p].x + 16) {
          koopa_x[i] = old_kx;
          koopa_dir[i] *= -1;
          break;
        }
      }
      
      // Patrol range
      if (koopa_x[i] < koopa_start_x[i] - 30) koopa_dir[i] = 1;
      if (koopa_x[i] > koopa_start_x[i] + 30) koopa_dir[i] = -1;
    }
  }
}

// Update piranha plants
for (int i = 0; i < pipe_count; i++) {
  if (pipe_has_plant[i]) {
    // Emerge when Mario is near (surprise attack)
    bool mario_near = (mx + scroll > pipes[i].x - 20 && mx + scroll < pipes[i].x + 36);
    if (mario_near && plant_height[i] < 12) {
      plant_height[i] += 0.4;
      if (plant_height[i] > 12) plant_height[i] = 12;
    } else if (!mario_near) {
      plant_height[i] -= 0.3;
      if (plant_height[i] < 0) plant_height[i] = 0;
    }
  }
}

// Update particles
for (int i = 0; i < particle_count; i++) {
  if (particle_timer[i] > 0) {
    particle_x[i] += particle_vx[i];
    particle_y[i] += particle_vy[i];
    particle_vy[i] += 0.4;
    particle_timer[i]--;
    if (particle_timer[i] == 0) {
      for (int j = i; j < particle_count - 1; j++) {
        particle_x[j] = particle_x[j + 1];
        particle_y[j] = particle_y[j + 1];
        particle_vx[j] = particle_vx[j + 1];
        particle_vy[j] = particle_vy[j + 1];
        particle_timer[j] = particle_timer[j + 1];
      }
      particle_count--;
      i--;
    }
  }
}

// Update fireworks
for (int i = 0; i < firework_count; i++) {
  if (firework_timer[i] > 0) {
    firework_x[i] += firework_vx[i];
    firework_y[i] += firework_vy[i];
    firework_vy[i] += 0.2;
    firework_timer[i]--;
    // Explode when velocity changes direction (at peak)
    if (!firework_exploded[i] && firework_vy[i] >= 0) {
      firework_exploded[i] = 1;
      // Create explosion particles (6-10 per firework)
      int num_particles = 6 + (millis() * (i + 1)) % 5;
      for (int p = 0; p < num_particles && explosion_count < 30; p++) {
        explosion_x[explosion_count] = firework_x[i];
        explosion_y[explosion_count] = firework_y[i];
        explosion_vx[explosion_count] = -2 + (millis() * (p + 1)) % 5;
        explosion_vy[explosion_count] = -2 + (millis() * (p + 2)) % 5;
        explosion_timer[explosion_count] = 20 + (millis() * (p + 3)) % 15;
        explosion_count++;
      }
    }
    if (firework_timer[i] == 0) {
      for (int j = i; j < firework_count - 1; j++) {
        firework_x[j] = firework_x[j + 1];
        firework_y[j] = firework_y[j + 1];
        firework_vx[j] = firework_vx[j + 1];
        firework_vy[j] = firework_vy[j + 1];
        firework_timer[j] = firework_timer[j + 1];
        firework_exploded[j] = firework_exploded[j + 1];
      }
      firework_count--;
      i--;
    }
  }
}

// Update score popups
for (int i = 0; i < score_popup_count; i++) {
  if (score_popup_timer[i] > 0) {
    score_popup_y[i]--;
    score_popup_timer[i]--;
    if (score_popup_timer[i] == 0) {
      for (int j = i; j < score_popup_count - 1; j++) {
        score_popup_x[j] = score_popup_x[j + 1];
        score_popup_y[j] = score_popup_y[j + 1];
        score_popup_val[j] = score_popup_val[j + 1];
        score_popup_timer[j] = score_popup_timer[j + 1];
      }
      score_popup_count--;
      i--;
    }
  }
}

// Update explosion particles
for (int i = 0; i < explosion_count; i++) {
  if (explosion_timer[i] > 0) {
    explosion_x[i] += explosion_vx[i];
    explosion_y[i] += explosion_vy[i];
    explosion_vy[i] += 0.15;
    explosion_timer[i]--;
    if (explosion_timer[i] == 0) {
      for (int j = i; j < explosion_count - 1; j++) {
        explosion_x[j] = explosion_x[j + 1];
        explosion_y[j] = explosion_y[j + 1];
        explosion_vx[j] = explosion_vx[j + 1];
        explosion_vy[j] = explosion_vy[j + 1];
        explosion_timer[j] = explosion_timer[j + 1];
      }
      explosion_count--;
      i--;
    }
  }
}

set_mario_x(mx); set_mario_y(my); set_mario_vy(vy); set_mario_scroll(scroll);

draw_scene:
// Draw sky background
it.filled_rectangle(0, 0, 128, 54, COLOR_OFF);

// Draw scrolling clouds
for (int i = 0; i < 5; i++) {
  int cx = (int)(cloud_x[i] - scroll * 0.3) % 200;
  if (cx < -20) cx += 220;
  int cy = cloud_y[i];
  if (cx > -20 && cx < 148) {
    // Cloud shape
    it.filled_circle(cx + 4, cy + 2, 3);
    it.filled_circle(cx + 8, cy + 1, 4);
    it.filled_circle(cx + 12, cy + 2, 3);
    it.filled_circle(cx + 6, cy + 4, 2);
    it.filled_circle(cx + 10, cy + 4, 2);
  }
}

// Draw ground with texture
it.filled_rectangle(0, 54, 128, 10);
for (int x = 0; x < 128; x += 8) {
  it.line(x, 54, x, 63);
}

// Draw bushes
for (int i = 0; i < 8; i++) {
  int bx = bush_x[i] - (int)scroll;
  if (bx > -16 && bx < 144) {
    // Bush shape
    it.filled_circle(bx + 4, 52, 3);
    it.filled_circle(bx + 8, 51, 4);
    it.filled_circle(bx + 12, 52, 3);
    it.line(bx + 8, 54, bx + 8, 50);
  }
}

// Draw holes
for (int i = 0; i < hole_count; i++) {
  int hx = holes[i].x - (int)scroll;
  if (hx + holes[i].w > 0 && hx < 128) {
    it.filled_rectangle(hx, 54, holes[i].w, 10, COLOR_OFF);
  }
}

// Draw pipes
for (int i = 0; i < pipe_count; i++) {
  int px = pipes[i].x - (int)scroll;
  int py = 54 - pipes[i].h;
  if (px + 16 > 0 && px < 128) {
    it.filled_rectangle(px, py, 16, pipes[i].h);
    it.rectangle(px, py, 16, pipes[i].h);
    it.filled_rectangle(px-2, py-2, 20, 3);
    it.line(px+8, py, px+8, py+pipes[i].h-1);
    
    // Draw piranha plant
    if (pipe_has_plant[i] && plant_height[i] > 0) {
      int ph = (int)plant_height[i];
      int plant_y = py - ph;
      // Stem with spots
      it.filled_rectangle(px+6, plant_y+2, 4, ph-2);
      it.draw_pixel_at(px+7, plant_y+3, COLOR_OFF);
      it.draw_pixel_at(px+8, plant_y+5, COLOR_OFF);
      // Head (round)
      it.filled_circle(px+8, plant_y, 3);
      // Spots on head
      it.draw_pixel_at(px+6, plant_y-1, COLOR_OFF);
      it.draw_pixel_at(px+10, plant_y-1, COLOR_OFF);
      it.draw_pixel_at(px+8, plant_y+2, COLOR_OFF);
      // Animated mouth (Pac-Man style) - faces Mario
      int mouth_open = ((int)(millis() / 150) % 2);
      bool mario_on_left = (mx + scroll < px + 8);
      if (mouth_open) {
        if (mario_on_left) {
          it.draw_pixel_at(px+6, plant_y-1, COLOR_OFF);
          it.draw_pixel_at(px+6, plant_y+1, COLOR_OFF);
          it.draw_pixel_at(px+7, plant_y, COLOR_OFF);
        } else {
          it.draw_pixel_at(px+10, plant_y-1, COLOR_OFF);
          it.draw_pixel_at(px+10, plant_y+1, COLOR_OFF);
          it.draw_pixel_at(px+9, plant_y, COLOR_OFF);
        }
      } else {
        if (mario_on_left) {
          it.line(px+6, plant_y-1, px+6, plant_y+1, COLOR_OFF);
        } else {
          it.line(px+10, plant_y-1, px+10, plant_y+1, COLOR_OFF);
        }
      }
      
      // Check collision with Mario
      if (mx + 6 > px + 5 && mx < px + 11 && my + mh > plant_y - 3 && my < plant_y + ph) {
        if (big) { big = false; fire = false; } 
        else if (!mario_dead) { mario_dead = true; death_timer = 0; death_vy = -5; lives--; }
      }
    }
  }
}

// Draw platforms
for (int i = 0; i < platform_count; i++) {
  int px = platforms[i].x - (int)scroll;
  int py = platforms[i].y;
  int pw = platforms[i].w;
  int ph = platforms[i].h;
  if (px + pw > 0 && px < 128) {
    it.filled_rectangle(px, py, pw, ph);
    for (int j = 0; j < pw; j += 8) it.line(px+j, py, px+j, py+ph-1);
  }
}

// Draw Mario sprite with better detail
int mx_i = (int)mx, my_i = (int)my;
bool ducking = get_btn_duck_held() && on_ground;
int anim = (walk_frame / 6) % 3;
if (ducking) {
  // Ducked Mario - shorter sprite
  if (mh == 8) {
    it.filled_rectangle(mx_i+1, my_i+2, 4, 1);
    it.filled_rectangle(mx_i, my_i+3, 6, 2);
    it.draw_pixel_at(mx_i+1, my_i+3, COLOR_OFF);
    it.draw_pixel_at(mx_i+4, my_i+3, COLOR_OFF);
    it.filled_rectangle(mx_i, my_i+5, 6, 3);
  } else {
    it.filled_rectangle(mx_i+1, my_i+4, 4, 1);
    it.filled_rectangle(mx_i, my_i+5, 6, 2);
    it.draw_pixel_at(mx_i+1, my_i+5, COLOR_OFF);
    it.draw_pixel_at(mx_i+4, my_i+5, COLOR_OFF);
    it.filled_rectangle(mx_i, my_i+7, 6, 5);
  }
} else if (mh == 8) {
  // Small Mario
  it.filled_rectangle(mx_i+1, my_i, 4, 1); // hat
  it.filled_rectangle(mx_i, my_i+1, 6, 2); // face
  it.draw_pixel_at(mx_i+1, my_i+1, COLOR_OFF);
  it.draw_pixel_at(mx_i+4, my_i+1, COLOR_OFF);
  it.draw_pixel_at(mx_i+2, my_i+2, COLOR_OFF);
  it.filled_rectangle(mx_i+1, my_i+3, 4, 2); // shirt
  it.filled_rectangle(mx_i+1, my_i+5, 4, 1); // overalls
  // Arms
  if (anim == 0 || !on_ground) {
    it.draw_pixel_at(mx_i, my_i+3);
    it.draw_pixel_at(mx_i+5, my_i+3);
  } else if (anim == 1) {
    it.draw_pixel_at(mx_i, my_i+4);
    it.draw_pixel_at(mx_i+5, my_i+2);
  } else {
    it.draw_pixel_at(mx_i, my_i+2);
    it.draw_pixel_at(mx_i+5, my_i+4);
  }
  // Legs and feet
  if (anim == 0 || !on_ground) {
    it.filled_rectangle(mx_i+1, my_i+6, 2, 1);
    it.filled_rectangle(mx_i+3, my_i+6, 2, 1);
    it.draw_pixel_at(mx_i, my_i+7);
    it.draw_pixel_at(mx_i+5, my_i+7);
  } else if (anim == 1) {
    it.filled_rectangle(mx_i+2, my_i+6, 2, 1);
    it.draw_pixel_at(mx_i+1, my_i+7);
    it.draw_pixel_at(mx_i+4, my_i+7);
    it.draw_pixel_at(mx_i, my_i+7);
  } else {
    it.filled_rectangle(mx_i+2, my_i+6, 2, 1);
    it.draw_pixel_at(mx_i+2, my_i+7);
    it.draw_pixel_at(mx_i+5, my_i+7);
    it.draw_pixel_at(mx_i+6, my_i+7);
  }
} else {
  // Big Mario
  it.filled_rectangle(mx_i+1, my_i, 4, 2); // hat
  it.filled_rectangle(mx_i, my_i+2, 6, 3); // face
  it.draw_pixel_at(mx_i+1, my_i+2, COLOR_OFF);
  it.draw_pixel_at(mx_i+4, my_i+2, COLOR_OFF);
  it.draw_pixel_at(mx_i+2, my_i+3, COLOR_OFF);
  it.filled_rectangle(mx_i+1, my_i+5, 4, 2); // shirt
  it.filled_rectangle(mx_i+1, my_i+7, 4, 2); // overalls
  // Arms
  if (anim == 0 || !on_ground) {
    it.filled_rectangle(mx_i-1, my_i+5, 2, 2);
    it.filled_rectangle(mx_i+5, my_i+5, 2, 2);
  } else if (anim == 1) {
    it.filled_rectangle(mx_i-1, my_i+6, 2, 2);
    it.filled_rectangle(mx_i+5, my_i+4, 2, 2);
  } else {
    it.filled_rectangle(mx_i-1, my_i+4, 2, 2);
    it.filled_rectangle(mx_i+5, my_i+6, 2, 2);
  }
  // Legs and feet
  if (anim == 0 || !on_ground) {
    it.filled_rectangle(mx_i+1, my_i+9, 2, 2);
    it.filled_rectangle(mx_i+3, my_i+9, 2, 2);
    it.draw_pixel_at(mx_i, my_i+11);
    it.draw_pixel_at(mx_i+5, my_i+11);
  } else if (anim == 1) {
    it.filled_rectangle(mx_i+2, my_i+9, 2, 2);
    it.filled_rectangle(mx_i+1, my_i+11, 2, 1);
    it.filled_rectangle(mx_i+4, my_i+11, 2, 1);
    it.draw_pixel_at(mx_i, my_i+11);
  } else {
    it.filled_rectangle(mx_i+2, my_i+9, 2, 2);
    it.filled_rectangle(mx_i+2, my_i+11, 2, 1);
    it.filled_rectangle(mx_i+5, my_i+11, 2, 1);
    it.draw_pixel_at(mx_i+6, my_i+11);
  }
}

for (int i = 0; i < coin_count; i++) {
  if (coins[i]) {
    int cx = coin_x[i] - (int)scroll;
    int cy = coin_y[i];
    if (cx > -4 && cx < 128) {
      it.circle(cx+2, cy+2, 2);
      if (mx + 6 > cx && mx < cx + 4 && my + mh > cy && my < cy + 4) {
        coins[i] = false; score += 10;
        coins_collected++;
        if (coins_collected % 5 == 0) lives++;
        play_sfx("coin");
      }
    }
  }
}

// Draw bricks with proper texture
for (int i = 0; i < brick_count; i++) {
  if (bricks[i]) {
    int bx = brick_x[i] - (int)scroll;
    int by = brick_y[i];
    if (bx > -8 && bx < 128) {
      it.filled_rectangle(bx, by, 8, 6);
      // Brick pattern
      it.line(bx, by+2, bx+8, by+2);
      it.line(bx+4, by, bx+4, by+2);
      it.line(bx+2, by+2, bx+2, by+6);
      it.line(bx+6, by+2, bx+6, by+6);
      it.rectangle(bx, by, 8, 6);
    }
  }
}

qx = 200 - (int)scroll;
if (qx > -8 && qx < 128) {
  it.rectangle(qx, qy, 8, 6);
  if (!question_hit) it.print(qx+4, qy+2, get_font_xxs(), TextAlign::CENTER, "?");
}

// Draw mushroom powerup
if (mushroom_spawned) {
  int mux = (int)mushroom_x - (int)scroll;
  int muy = (int)mushroom_y;
  if (mux > -6 && mux < 128) {
    it.filled_rectangle(mux, muy+2, 6, 3);
    it.filled_circle(mux+3, muy+2, 2);
    it.filled_rectangle(mux+2, muy+4, 2, 2);
    it.draw_pixel_at(mux+1, muy+3, COLOR_OFF);
    it.draw_pixel_at(mux+4, muy+2, COLOR_OFF);
    if (mx + 6 > mux && mx < mux + 6 && my + mh > muy && my < muy + 6) {
      mushroom_spawned = false; big = true; score += 100;
      play_sfx("powerup");
    }
  }
}

// Draw fire flower
if (flower_spawned) {
  int flx = (int)flower_x - (int)scroll;
  int fly = (int)flower_y;
  if (flx > -6 && flx < 128) {
    it.filled_rectangle(flx+2, fly+3, 2, 3);
    it.draw_pixel_at(flx+1, fly+2);
    it.draw_pixel_at(flx+4, fly+2);
    it.draw_pixel_at(flx+1, fly+5);
    it.draw_pixel_at(flx+4, fly+5);
    it.draw_pixel_at(flx, fly+3);
    it.draw_pixel_at(flx+5, fly+3);
    if (mx + 6 > flx && mx < flx + 6 && my + mh > fly && my < fly + 6) {
      flower_spawned = false; fire = true; score += 200;
      play_sfx("powerup");
    }
  }
}

// Draw fireballs
for (int i = 0; i < 5; i++) {
  if (fireballs[i]) {
    int fbx = (int)fireball_x[i] - (int)scroll;
    int fby = (int)fireball_y[i];
    if (fbx > -3 && fbx < 131) {
      it.filled_circle(fbx+1, fby+1, 2);
    }
  }
}

// Draw goombas with proper shape
for (int i = 0; i < goomba_count; i++) {
  if (goombas[i]) {
    int gx = (int)goomba_x[i] - (int)scroll;
    int gy = (int)goomba_y[i];
    if (gx > -6 && gx < 128) {
      // Goomba body
      it.filled_rectangle(gx, gy+2, 6, 4);
      // Goomba head
      it.filled_circle(gx+3, gy+1, 2);
      // Eyes
      it.draw_pixel_at(gx+1, gy+1, COLOR_OFF);
      it.draw_pixel_at(gx+4, gy+1, COLOR_OFF);
      // Angry eyebrows
      it.line(gx+1, gy, gx+2, gy);
      it.line(gx+3, gy, gx+4, gy);
      // Feet
      it.filled_rectangle(gx, gy+5, 2, 1);
      it.filled_rectangle(gx+4, gy+5, 2, 1);
      
      if (mx + 6 > gx && mx < gx + 6 && my + mh > gy && my < gy + 6) {
        if (vy > 0 && my < gy + 2) {
          goombas[i] = false; score += 100; vy = -3.0;
          play_sfx("jumpon");
          if (score_popup_count < 5) {
            score_popup_x[score_popup_count] = (int)goomba_x[i];
            score_popup_y[score_popup_count] = (int)goomba_y[i];
            score_popup_val[score_popup_count] = 100;
            score_popup_timer[score_popup_count] = 30;
            score_popup_count++;
          }
        } else {
          if (big) { big = false; fire = false; } 
          else if (!mario_dead) { mario_dead = true; death_timer = 0; death_vy = -5; lives--; }
        }
      }
    }
  }
}

// Draw koopas
for (int i = 0; i < koopa_count; i++) {
  if (koopas[i]) {
    int kx = (int)koopa_x[i] - (int)scroll;
    int ky = (int)koopa_y[i];
    if (kx > -6 && kx < 128) {
      if (koopa_shell[i]) {
        // Shell
        it.filled_rectangle(kx, ky+3, 6, 3);
        it.rectangle(kx, ky+3, 6, 3);
      } else {
        // Body
        it.filled_rectangle(kx, ky+2, 6, 4);
        // Shell pattern
        it.rectangle(kx, ky+2, 6, 4);
        it.line(kx+3, ky+2, kx+3, ky+5);
        // Head
        it.filled_rectangle(kx+1, ky, 4, 2);
        it.draw_pixel_at(kx+1, ky, COLOR_OFF);
        it.draw_pixel_at(kx+4, ky, COLOR_OFF);
      }
      
      // Collision with Mario
      if (mx + 6 > kx && mx < kx + 6 && my + mh > ky && my < ky + 6) {
        if (vy > 0 && my < ky + 2 && !koopa_shell[i]) {
          koopa_shell[i] = true;
          score += 100;
          vy = -3.0;
          play_sfx("jumpon");
          if (score_popup_count < 5) {
            score_popup_x[score_popup_count] = (int)koopa_x[i];
            score_popup_y[score_popup_count] = (int)koopa_y[i];
            score_popup_val[score_popup_count] = 100;
            score_popup_timer[score_popup_count] = 30;
            score_popup_count++;
          }
        } else if (!koopa_shell[i]) {
          if (big) { big = false; fire = false; }
          else if (!mario_dead) { mario_dead = true; death_timer = 0; death_vy = -5; lives--; }
        }
      }
    }
  }
}

// Draw particles
for (int i = 0; i < particle_count; i++) {
  if (particle_timer[i] > 0) {
    int px = particle_x[i] - (int)scroll;
    int py = particle_y[i];
    if (px > -2 && px < 130) {
      it.filled_rectangle(px, py, 2, 2);
    }
  }
}

// Draw score popups
for (int i = 0; i < score_popup_count; i++) {
  if (score_popup_timer[i] > 0) {
    int spx = score_popup_x[i] - (int)scroll;
    int spy = score_popup_y[i];
    if (spx > -10 && spx < 138) {
      it.printf(spx, spy, get_font_xxs(), "%d", score_popup_val[i]);
    }
  }
}

// Draw firework rockets (before explosion)
for (int i = 0; i < firework_count; i++) {
  if (firework_timer[i] > 0 && !firework_exploded[i]) {
    int fx = firework_x[i] - (int)scroll;
    int fy = firework_y[i];
    if (fx > -2 && fx < 130 && fy > -2 && fy < 66) {
      it.filled_rectangle(fx, fy, 2, 3);
    }
  }
}

// Draw explosion particles
for (int i = 0; i < explosion_count; i++) {
  if (explosion_timer[i] > 0) {
    int ex = explosion_x[i] - (int)scroll;
    int ey = explosion_y[i];
    if (ex > -2 && ex < 130 && ey > -2 && ey < 66) {
      it.draw_pixel_at(ex, ey);
      if (explosion_timer[i] > 10) {
        it.draw_pixel_at(ex-1, ey);
        it.draw_pixel_at(ex+1, ey);
        it.draw_pixel_at(ex, ey-1);
        it.draw_pixel_at(ex, ey+1);
      }
    }
  }
}

// Draw flag
int fx = FLAG_X - (int)scroll;
if (fx > -8 && fx < 136) {
  it.line(fx, 20, fx, 54); // flagpole
  it.filled_rectangle(fx+1, flag_y, 6, 4); // flag
  it.rectangle(fx+1, flag_y, 6, 4);
}

// Draw castle
int cx = FLAG_X + 20 - (int)scroll;
if (cx > -24 && cx < 128) {
  it.filled_rectangle(cx, 38, 24, 16);
  it.rectangle(cx, 38, 24, 16);
  it.filled_rectangle(cx+8, 46, 8, 8, COLOR_OFF);
  it.filled_rectangle(cx, 34, 6, 4);
  it.filled_rectangle(cx+9, 34, 6, 4);
  it.filled_rectangle(cx+18, 34, 6, 4);
}

// Draw UI on single top line
it.printf(2, 0, get_font_xxs(), "W%d-%d", world, level);
it.printf(32, 0, get_font_xxs(), "S:%d", score);
it.printf(64, 0, get_font_xxs(), "C:%d", coins_collected);
it.printf(88, 0, get_font_xxs(), "L:%d", lives);
it.printf(104, 0, get_font_xxs(), "T:%d", timer);
#endif // ESPHOMARIO_GAME_LOGIC