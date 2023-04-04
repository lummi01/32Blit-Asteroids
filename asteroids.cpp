#include <list>

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
    float dx = .2f;
	float dy = .1f;
    float sx;
    float sy;
    float sdx;
	float sdy;
    short st;
    short ani;
};

struct EXPLOSION
{
	Vec2 pos;
	Vec2 vel;
	short alpha;
};

SaveData saveData;
GAME game;
PLAYER p;
SHOT shot[4];
ASTEROID asteroid[54];
UFO ufo;

static std::list<EXPLOSION> particles;

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
    ufo.y = 4 + rand() %(SCREEN_HEIGHT - 8);
    ufo.sx = ufo.x;
    ufo.sy = ufo.y;
    ufo.st = 0;
    ufo.is = true;
}

void NewExplosion(float x, float y, short number)
{
	for(short n=0; n<number; n++) 
	{
	    EXPLOSION e;
        float d = rand() %360 * PI;		
        e.pos = Vec2(x, y);
        e.vel = Vec2 (sin(d), cos(d)) * Vec2(.2f, .2f);
        e.alpha = 50 + rand() %200;
        particles.push_back(e);
	}
}

void UpdateExplosion()
{
    for(auto e = particles.begin(); e != particles.end();) 
	{
        if(e->alpha < 5) 
		{
            e = particles.erase(e);
            continue;
        }
        e->pos += e->vel;
        e->alpha -= 5;
        ++e;
    }
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
    for (short a=0; a<54; a++)
        asteroid[a].type = 0;

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

	    EXPLOSION e;
        float dx = float(-sin(p.sprite * 15.0f * PI)) * .1f;
        float dy = float(cos(p.sprite * 15.0f * PI)) * .1f;  
        e.vel = Vec2(dx, dy);
        e.pos = Vec2(p.x, p.y) + (e.vel * Vec2(35, 35));
        e.alpha = 150 + rand() %100;
        particles.push_back(e);
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
/*
    if (buttons & Button::B) // teleport
    {
        p.is = false;
        p.sprite = rand() %24;
        p.x = 10 + rand() %(SCREEN_WIDTH - 20);
        p.y = 10 + rand() %(SCREEN_HEIGHT - 20);
        p.dx = 0;
        p.dy = 0;
    }
*/
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
					NewExplosion(asteroid[a].x, asteroid[a].y, 12);
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
				NewExplosion(ufo.x, ufo.y, 16);
                game.score += 50;
                ufo.is = false;
				ufo.dx > 0? ufo.x = 0: ufo.x = SCREEN_WIDTH;
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
			NewExplosion(p.x, p.y, 16);
			NewExplosion(ufo.x, ufo.y, 16);
            game.score += 50;
            ufo.is = false;
			ufo.dx > 0? ufo.x = 0: ufo.x = SCREEN_WIDTH;
            ufo_timer.start();
            return true;
        }
        else if (Collision(p.x, p.y, 3, ufo.sx, ufo.sy, 0))
        {
			NewExplosion(p.x, p.y, 16);
            ufo.st = 0;
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
		ufo.y+=ufo.dy;
        if (ufo.x < 0 || ufo.x > SCREEN_WIDTH)
        {
			ufo.dx =- ufo.dx;
            ufo.is = false;
            ufo_timer.start();
        }
		else if (AsteroidCollision(ufo.x, ufo.y, 3, 0))
		{
			ufo.dx > 0? ufo.x = 0: ufo.x = SCREEN_WIDTH;
            ufo.is = false;
            ufo_timer.start();
		}
        if (ufo.y < 4 || ufo.y > SCREEN_HEIGHT - 4)
			ufo.dy=-ufo.dy;

        ufo.sx += ufo.sdx;//float(sin(ufo.sd));
        ufo.sy += ufo.sdy;//float(-cos(ufo.sd));
        ufo.st--;
        if (ufo.st < 0 || AsteroidCollision(ufo.sx, ufo.sy, 0, 0))
        {

			float dx = p.x - ufo.x;
			float dy = p.y - ufo.y;
			float hyp = 100;
			if (p.is)
				hyp = sqrt((dx * dx) + (dy * dy));
			for (short a=0; a<54; a++)
			{
				if (asteroid[a].type > 0)
				{
					float adx = asteroid[a].x - ufo.x;
					float ady = asteroid[a].y - ufo.y;
					float ahyp = sqrt((adx * adx) + (ady * ady));
					if (ahyp < hyp)
					{
						dx = adx;
						dy = ady;
						hyp = ahyp;
					}
				}
			}
            ufo.sx = ufo.x;
            ufo.sy = ufo.y;
			ufo.sdx = dx / hyp;
			ufo.sdy = dy / hyp;
            ufo.st = 45;
        }
    }
}

void start()
{
    game.live = 2;
    game.score = 0;
    game.wave = 0;

    p.is = true;
    p.sprite = 0;
    p.x = SCREEN_WIDTH / 2;
    p.y = SCREEN_HEIGHT / 2;
    p.dx = 0;
    p.dy = 0;
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

	    for(auto &p : particles)
		{
			screen.alpha = p.alpha;
	        screen.pixel(Point(p.pos.x, p.pos.y));
		}
		screen.alpha = 255;

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
//        screen.text(std::to_string(game.asteroid), font, Point(0, 0), true, TextAlign::top_left);

        if (game.state == 2)
            screen.text("game over", minimal_font, Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), true, TextAlign::center_center);
    }
}

// update(time)

void update(uint32_t time) 
{
    AsteroidUpdate();
    UpdateUFO();
	UpdateExplosion();

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
				{
					for (short s=0; s<4; s++)
						shot[s].time = 0;
                    game.state = 2;
				}
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
        if (buttons.released & Button::B)
            game.state = 0;        
    }
}

