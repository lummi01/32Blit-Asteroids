#include "asteroids.hpp"
#include "assets.hpp"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 120

using namespace blit;

Font font(font3x5);

float PI = (pi / 180);

struct SaveData 
{
    int score;
};

struct GAME
{
    int state = 0;
    short wave;
    short asteroid;
    short live;
    int score;
};

struct PLAYER 
{
    bool is;
    short sprite;
    float x;
    float y;
    float dx;
    float dy;
    bool shot;
};

struct SHOT
{
    float x;
    float y;
    float d;
    short time;
};

struct ASTEROID
{
    short type;
    short color;
    short ani;
    short timer;
    float x;
    float y;
    float dx;
    float dy;
};

struct UFO
{
    bool is;
    float x;
    float y;
    float dx;
    float sx;
    float sy;
    float sd;
    short st;
    short ani;
};

SaveData saveData;
GAME game;
PLAYER p;
SHOT shot[4];
ASTEROID asteroid[54];
UFO ufo;

Tween tween_control;

Timer ani_timer;
Timer ufo_timer;

void ani_update(Timer &t)
{
    for (short a=0; a<54; a++)
    {
        if (asteroid[a].type > 0)
        {    
            asteroid[a].ani++;
            if (asteroid[a].ani > 7)
                asteroid[a].ani = 0;
        }            
    }

    ufo.ani++;
    if (ufo.ani > 7)
        ufo.ani = 0;
}

void new_ufo(Timer &t)
{
    if (rand() %2 == 0)
    {
        ufo.x = 0;
        ufo.dx = .2f;
    }
    else
    {
        ufo.x = SCREEN_WIDTH;
        ufo.dx = -.2f;
    }
    ufo.y = 4 + rand() %(SCREEN_HEIGHT - 8);
    ufo.sx = ufo.x;
    ufo.sy = ufo.y;
    ufo.sd = rand ()%360 * PI;
    ufo.st = 35;
    ufo.is = true;
}

void NewAsteroid(short type, float x, float y, float dx, float dy)
{
    float w = game.wave / 4000 + .001f; 
    for (short a=0; a<54; a++)
    {
        if (asteroid[a].type == 0)
        {
            asteroid[a].type = type;
            asteroid[a].color = rand() %3;
            asteroid[a].ani = rand() %8;
            asteroid[a].x = x;
            asteroid[a].y = y;

            asteroid[a].dx = (rand() %100) * w + .01f;
            if (rand()%2 == 1)
                asteroid[a].dx = -asteroid[a].dx;
            asteroid[a].dy = (rand() %100) * (w + .001f) + .01f;
            if (rand()%2 == 1)
                asteroid[a].dy = -asteroid[a].dy;
            game.asteroid++;
            break;
        }
    }       

}

void NewWave()
{
    game.wave++;
    short a = game.wave + 2;
    if (a > 6)
        a = 6;
    for (short i=0; i<a; i++)
    {
        float x = p.x + (SCREEN_WIDTH / 4) + rand() %(SCREEN_WIDTH / 2);
        if (x > SCREEN_WIDTH)
            x -= SCREEN_WIDTH;
        float y = rand() %SCREEN_HEIGHT;
        NewAsteroid(8, x, y, 0, 0);
    }
}

void NewShot()
{
    for (short i=0; i<4; i++)
    {
        if (shot[i].time == 0)
        {
            shot[i].d = float(p.sprite * 15 * PI);
            shot[i].x = p.x + float(5 * sin(shot[i].d));
            shot[i].y = p.y - float(5 * cos(shot[i].d));
            shot[i].time = 60;
            break;
        }
    }
}

void ShotUpdate()
{
    for (short i=0; i<4; i++)
    {
        if (shot[i].time > 0)
        {
            shot[i].time--;
            shot[i].x += float(sin(shot[i].d));
            shot[i].y += float(-cos(shot[i].d));
/*
            if (shot[i].x < 0)
                shot[i].x += SCREEN_WIDTH;
            else if (shot[i].x > SCREEN_WIDTH)
                shot[i].x -= SCREEN_WIDTH;
            else if (shot[i].y < 0)
                shot[i].y += SCREEN_HEIGHT;
            else if (shot[i].y > SCREEN_HEIGHT)
                shot[i].y -= SCREEN_HEIGHT;
*/
        }
    }
}

void UpdateControl()
{
    if (tween_control.is_finished())
    {
        if (buttons & Button::DPAD_LEFT || joystick.x < 0)
        {
            p.sprite--;
            if (p.sprite < 0)
               p.sprite = 23;
            tween_control.start();
        }
        else if (buttons & Button::DPAD_RIGHT || joystick.x > 0)
        {
            p.sprite++;
            if (p.sprite > 23)
                p.sprite = 0;
            tween_control.start();
        }
    }

    if (buttons & Button::DPAD_UP || joystick.y < 0)
    {
        p.dx += float(sin(p.sprite * 15.0f * PI)) * .01f;
        p.dy += float(-cos(p.sprite * 15.0f * PI)) * .01f;  
    }
    else
    {
        p.dx *= .999f;
        p.dy *= .999f; 
    }

    p.x += p.dx;
    p.y += p.dy;  

    if (p.x < 0)
        p.x += SCREEN_WIDTH;
    else if (p.x > SCREEN_WIDTH)
        p.x -= SCREEN_WIDTH;
    else if (p.y < 0)
        p.y += SCREEN_HEIGHT;
    else if (p.y > SCREEN_HEIGHT)
        p.y -= SCREEN_HEIGHT;


    if (p.shot && buttons & Button::A) // fire
    {
        NewShot();
        p.shot = false;
    }
    if (buttons.pressed & Button::A)
        p.shot = true;

    if (buttons & Button::B) // teleport
    {
        p.is = false;
        p.sprite = rand() %24;
        p.x = 10 + rand() %(SCREEN_WIDTH - 20);
        p.y = 10 + rand() %(SCREEN_HEIGHT - 20);
        p.dx = 0;
        p.dy = 0;
    }
}

void AsteroidUpdate()
{
    for (short a=0; a<54; a++)
    {
        if (asteroid[a].type > 0)
        {    
            asteroid[a].x += asteroid[a].dx;
            if (asteroid[a].x < - asteroid[a].type) 
                asteroid[a].x = SCREEN_WIDTH + asteroid[a].type;       
            if (asteroid[a].x > SCREEN_WIDTH + asteroid[a].type) 
                asteroid[a].x = - asteroid[a].type;  
     
            asteroid[a].y += asteroid[a].dy;
            if (asteroid[a].y < - asteroid[a].type) 
                asteroid[a].y = SCREEN_HEIGHT + asteroid[a].type;       
            if (asteroid[a].y > SCREEN_HEIGHT + asteroid[a].type) 
                asteroid[a].y = - asteroid[a].type;
        }
    }       
}

bool Collision(float x1, float y1, float r1, float x2, float y2, float r2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float hyp = sqrt((dx * dx) + (dy * dy));
    if (hyp < (r1 + r2))
        return true;
    return false;
}

bool AsteroidCollision(float x, float y, short r, short score)
{
    for (short a=0; a<54; a++)
    {
        if (asteroid[a].type > 0)
        {
            if (Collision(asteroid[a].x, asteroid[a].y, asteroid[a].type, x, y, r))
            {
                if (r < 5)
                {
                    game.score += (40 / asteroid[a].type) * score;
                    if (asteroid[a].type > 2)
                        for (short i=0; i<3; i++)
                            NewAsteroid(asteroid[a].type / 2, asteroid[a].x, asteroid[a].y, asteroid[a].dx, asteroid[a].dy);
                    asteroid[a].type = 0;
                    game.asteroid--;
                    if (game.asteroid == 0)
                       NewWave();
                }
                return true;
                break;    
            }
        }
    }
    return false;
}

void ShotCollision()
{
    for (short s=0; s<4; s++)
    {
        if (shot[s].time > 0)
        {
            if (AsteroidCollision(shot[s].x, shot[s].y, 0, 1))
            {
                shot[s].time = 0;
                break;    
            }
            else if (ufo.is && Collision(shot[s].x, shot[s].y, 0, ufo.x, ufo.y, 3))
            {
                game.score += 50;
                ufo.is = false;
                ufo_timer.start();
                shot[s].time = 0;
                break;
            }
        }
    }
}

bool PlayerCollision()
{
    if (AsteroidCollision(p.x, p.y, 3, 1))
        return true;
    else if (ufo.is)
    {
        if (Collision(p.x, p.y, 3, ufo.x, ufo.y, 3))
        {
            game.score += 50;
            ufo.is = false;
            ufo_timer.start();
            return true;
        }
        else if (Collision(p.x, p.y, 3, ufo.sx, ufo.sy, 0))
        {
            ufo.sx = ufo.x;
            ufo.sy = ufo.y;
            ufo.sd = rand ()%360 * PI;
            ufo.st = 35;
            return true;
        }
    }    
    return false;
}

void UpdateUFO()
{
    if (ufo.is)
    {
        ufo.x+=ufo.dx;
        if (ufo.x < 0 || ufo.x > SCREEN_WIDTH || AsteroidCollision(ufo.x, ufo.y, 3, 0))
        {
            ufo.is = false;
            ufo_timer.start();
        }
        ufo.sx += float(sin(ufo.sd));
        ufo.sy += float(-cos(ufo.sd));
        ufo.st--;
        if (ufo.st < 0 || AsteroidCollision(ufo.sx, ufo.sy, 0, 0))
        {
            ufo.sx = ufo.x;
            ufo.sy = ufo.y;
            ufo.sd = rand ()%360 * PI;
            ufo.st = SCREEN_HEIGHT / 2;
        }
        
    }
}

void start()
{
    game.live = 3;
    game.score = 0;
    game.wave = 0;

    p.is = true;
    p.x = SCREEN_WIDTH / 2;
    p.y = SCREEN_HEIGHT / 2;
    p.shot = true;

    NewWave();    

    game.state = 1;
}

void init() 
{
    set_screen_mode(ScreenMode::lores);

    screen.sprites = Surface::load(sprites);

/*
    channels[0].waveforms = Waveform::NOISE; 
    channels[0].frequency = 1000;
    channels[0].attack_ms = 8;
    channels[0].decay_ms = 32;
    channels[0].sustain = 0;

    channels[2].waveforms = Waveform::SINE; // Laser
    channels[2].frequency = 0;
    channels[2].attack_ms = 5;
    channels[2].decay_ms = 500;
    channels[2].sustain = 0;
    channels[2].release_ms = 5;

    channels[5].waveforms = Waveform::TRIANGLE; // PowerUp
    channels[5].frequency = 2000;
    channels[5].attack_ms = 5;
    channels[5].decay_ms = 80;
    channels[5].sustain = 0;
    channels[5].release_ms = 5;
*/
    tween_control.init(tween_linear,0 , 1, 35, 1);
    tween_control.start();

    ani_timer.init(ani_update, 100, -1);
    ani_timer.start();

    ufo_timer.init(new_ufo, 10000, 1);
    ufo_timer.start();
}


// render(time)

void render(uint32_t time) 
{
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.alpha = 255;
    screen.mask = nullptr;
    screen.pen = Pen(255, 255, 255);

    if (game.state == 0) 
    {
        screen.sprite(Rect(0, 12, 16, 4), Point(SCREEN_WIDTH / 2 - 64, SCREEN_HEIGHT / 2 - 32));
        screen.text("Press A to Start", minimal_font, Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT * 2 / 3), true, TextAlign::center_center);
    }
    else
    {
        for (short a=0; a<54; a++) //Asteroids
        {
            if (asteroid[a].type == 8)
                screen.sprite(Rect(asteroid[a].ani * 2, 2 + (asteroid[a].color * 2), 2, 2), Point(asteroid[a].x - 8, asteroid[a].y - 8));
            else if (asteroid[a].type == 4)
                screen.sprite(128 + asteroid[a].ani + (asteroid[a].color * 16), Point(asteroid[a].x - 4, asteroid[a].y - 4));
            else if (asteroid[a].type == 2)
                screen.sprite(136 + asteroid[a].ani + (asteroid[a].color * 16), Point(asteroid[a].x - 4, asteroid[a].y - 4));
        }

        if (ufo.is)
        {
            screen.pixel(Point(ufo.sx, ufo.sy)); //UFO shot
            screen.sprite(176 + ufo.ani,Point(ufo.x - 4, ufo.y - 4)); //UFO
        }

        if (p.is)
            screen.sprite(p.sprite,Point(p.x - 4, p.y - 4)); //Spaceship

        for (short s=0; s<4; s++) //Shots
        {
            if (shot[s].time > 0)
                screen.pixel(Point(shot[s].x, shot[s].y));
        }

        //lives
        for (short s=game.live; s>0; s--)
            screen.sprite(31, Point(SCREEN_WIDTH - (s * 5), SCREEN_HEIGHT - 13));

        // score
/*
        std::string score_txt ("000000");
        std::string score (std::to_string(game.score));
        score_txt.erase(0, score.size());
        screen.text(score_txt + score, font, Point(1, SCREEN_HEIGHT -6), true, TextAlign::buttom_left);        
*/
        screen.text(std::to_string(game.score), font, Point(SCREEN_WIDTH, SCREEN_HEIGHT -6), true, TextAlign::top_right);

        if (game.state == 2)
            screen.text("game over", minimal_font, Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), true, TextAlign::center_center);
    }
}

// update(time)

void update(uint32_t time) 
{
    AsteroidUpdate();
    UpdateUFO();

    if (game.state == 0) //title
    {
        if (buttons.released & Button::A)
            start();        
    }
    else if (game.state == 1)
    {
        if (p.is)
        {
            UpdateControl();
            if (PlayerCollision())
            {
                p.is = false; //died
                if (game.live < 1) //end of game
                    game.state = 2;
            }
        }
        else if(AsteroidCollision(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 16, 0) == false && ufo.is == false)
        {
            game.live--;
            p.sprite = 0;
            p.x = SCREEN_WIDTH / 2;
            p.y = SCREEN_HEIGHT / 2;
            p.dx = 0;
            p.dy = 0;
            p.is = true;
        }
        ShotUpdate();
        ShotCollision();
    }
    else if (game.state == 2) //game over
    {
        if (buttons.released & Button::A)
            game.state = 0;        
    }
}

