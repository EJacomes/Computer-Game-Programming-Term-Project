#include "DirectX.h"

//game is gonna be a wave based survival game
//max waves: 10
//User will start out with a slow speed, slow fire rate, low health
//User will have the chance to upgrade those elements after the end of each round
//The amount of enemy that user will have to face is based on starting wave + (wave num * 5)
//enemies will only have one health and will give the player a certain amount of money to spend for upgrades


/*Checklist to complete:
Please mark the date of successfull completion of item

1. implement placeholder background and player movement (3/29/26, update this when you can make the model small) (update: 3/30/26)
2. implement random tiles (stones, boxes, sticks, etc) (4/5/26)
3. implement bigger background and adjust the code for it (4/1/26)
4. implement gunfire and explosion (look at the tank game) (have them set to the lowest firerate) (4/5/26)
5. intialize speed, fire rate, health for later on when the upgrading features is being worked on (4/5/26)
6. implement enemy function and complete enemy design 
7. implement reset feature where the enemies and tiles are spawned in after all do the enemies are defeated and the user allows the game to continue (use message box)
8. implement the UI for the game (pausing, wave over screen, buttons to lead to upgrade shop or next round, game over screen with score and money
and buttons to reset or quit), upgrade shop, health bar that changes during game play
9. Work on getting the upgrade shop working to where the upgrade are fully working
10. Get the next wave feature working and shown to the user
11. enemy movement should be more dynamic

*if there is anymore, then it should be added onto the list
*/

//game settings
Model2D Player, background;
Keyboard::KeyboardStateTracker keyboardtracker;
Mouse::ButtonStateTracker mousetracker;
std::unique_ptr<SpriteFont> spriteFont;

//tile settings
//stones settings
const int STONES_NUMBER = 20;
const int stone_width = 43;
const int stone_height = 25;
const int stone_rows = Height / stone_height;
const int stone_columns = Width / stone_width;
Model2D stone[STONES_NUMBER];
bool stone_valid[STONES_NUMBER];

//box settings
const int BOXES_NUMBER = 20;
const int box_width = 43;
const int box_height = 41;
const int box_rows = Height / box_height;
const int box_columns = Width / box_width;
Model2D box[BOXES_NUMBER];
bool box_valid[BOXES_NUMBER];

//stick settings
const int STICKS_NUMBER = 20;
const int stick_width = 43;
const int stick_height = 30;
const int stick_rows = Height / stick_height;
const int stick_columns = Width / stick_width;
Model2D stick[STICKS_NUMBER];
bool stick_valid[STICKS_NUMBER];

//world setting
//as of right now, these are placeholder lengths
const int Width = 1920;
const int Height = 912;
int world_width = 1920;
int world_height = 912;
const float framerate = 30;
bool gameover;

//may add the road_pos just for the player to have one spot to constantly spawn in
int road_pos_y = 700;
int road_pos_x = 935;

//upgradeable features
//this player_speed variable can be changed as it gets upgraded
//start out slow, each upgrade increases the speed by +1
float player_speed = 5;

//the variable cooldown is related to the rate of fire that the player has (starting firerate: 1000, max firerate: 50)
//enemies have a seperate cooldown
int player_cooldown = 1000; // allow to fire every x ms

//the variable player_health will start at 3 but will have a max of 15
int player_health = 3;

//gunplay setting
std::unique_ptr<SoundEffect> gunfire;
std::unique_ptr<SoundEffect> explode;
long cooling = 0;

//player bullets and explosion
const int PLAYER_BULLETS_NUMBER = 50;
Model2D bullet[PLAYER_BULLETS_NUMBER];
bool bullet_valid[PLAYER_BULLETS_NUMBER];
int last_bullet = -1;
float bullet_speed = 2 * player_speed;

Model2D explosion[PLAYER_BULLETS_NUMBER];
bool explosion_valid[PLAYER_BULLETS_NUMBER];
int last_explosion = -1;

//may have to make an enemy bullets and explosion
//when an enemy bullet hits a player, it should take a player life and then play an explosion
//once the player loses all their live then the game will end
//intialize the enemy bullets and explosion variable
//intialize the enemy variables and then make the enemy bullets and explosion method and then make enemy methods

void LoadPlayer()
{
	Player = CreateModel2D(L"player_model.png", 4, 1);
    //Player = CreateModel2D(L"tank.png", 4, 1);
    if (Player.texture == NULL)
    {
        MessageBox(NULL, L"Loading player_model.png error", L"Error", MB_OK | MB_ICONERROR);
    }

    Player.x = road_pos_x;
    Player.y = road_pos_y;
}

void LoadStones()
{
    for (int i = 0; i < STONES_NUMBER; i++)
    {   // randomly build stones
        int row = rand() % stone_rows;
        int column = rand() % stone_columns;
        stone[i] = CreateModel2D(L"stonepng_copy.png");
        stone[i].x = column * stone_width;
        stone[i].y = row * stone_height;
        stone_valid[i] = true;
        // if the new created stone collided with tank, delete this stone
        if (CheckModel2DCollided(Player, stone[i]))
        {
            ZeroMemory(&stone[i], sizeof(Model2D));
            stone_valid[i] = false;
        }
    }
}

void LoadBoxes()
{
    for (int i = 0; i < BOXES_NUMBER; i++)
    {
        int row = rand() % box_rows;
        int column = rand() % box_columns;
        box[i] = CreateModel2D(L"box.png");
        box[i].x = column * box_width;
        box[i].y = row * box_height;
        box_valid[i] = true;

        if (CheckModel2DCollided(Player, box[i]))
        {
            ZeroMemory(&box[i], sizeof(Model2D));
            box_valid[i] = false;
        }
    }
}

void LoadSticks()
{
    for (int i = 0; i < STICKS_NUMBER; i++)
    {
        int row = rand() % stick_rows;
        int column = rand() % stick_columns;
        stick[i] = CreateModel2D(L"sticks.png");
        stick[i].x = column * stick_width;
        stick[i].y = row * stick_height;
        stick_valid[i] = true;

        if (CheckModel2DCollided(Player, stick[i]))
        {
            ZeroMemory(&stick[i], sizeof(Model2D));
            stick_valid[i] = false;
        }
    }
}

void LoadPlayerBullets()
{
    for (int i = 0; i < PLAYER_BULLETS_NUMBER; i++)
    {
        bullet[i] = CreateModel2D(L"bullet.png");
        bullet[i].move_x = 0;
        bullet[i].move_y = 0;
        bullet_valid[i] = false;
    }
}

void LoadPlayerExplsions()
{
    for (int i = 0; i < PLAYER_BULLETS_NUMBER; i++)
    {
        explosion[i] = CreateModel2D(L"explosion.png", 8, 8);
        explosion_valid[i] = false;
    }
}

void UpdatePlayer()
{
    auto kb = keyboard->GetState();

    Player.move_x = 0;
    Player.move_y = 0;

    if (kb.W)
    {
        Player.move_y = -player_speed;
        Player.frame = 0;
    }
    if (kb.S)
    {
        Player.move_y = player_speed;
        Player.frame = 2;
    }
    if (kb.A)
    {
        Player.move_x = -player_speed;
        Player.frame = 3;
    }
    if (kb.D)
    {
        Player.move_x = player_speed;
        Player.frame = 1;
    }

    Player.x += Player.move_x;
    Player.y += Player.move_y;

    if (Player.x < 0)
    {
        Player.x = 0;
    }
    if (Player.x > Width - Player.frame_width)
    {
        Player.x = Width - Player.frame_width;
    } 
    if (Player.y < 0)
    {
        Player.y = 0;
    }
    if (Player.y > Height - Player.frame_height)
    {
        Player.y = Height - Player.frame_height;
    }   

    // if collided with stone, box, and stick player move back to previous position
    for (int j = 0; j < STONES_NUMBER; j++)
    {
        if (stone_valid[j] && CheckModel2DCollided(Player, stone[j]))
        {
            Player.x -= Player.move_x;
            Player.y -= Player.move_y;
        }
    }

    for (int j = 0; j < BOXES_NUMBER; j++)
    {
        if (box_valid[j] && CheckModel2DCollided(Player, box[j]))
        {
            Player.x -= Player.move_x;
            Player.y -= Player.move_y;
        }
    }

    for (int j = 0; j < STICKS_NUMBER; j++)
    {
        if (stick_valid[j] && CheckModel2DCollided(Player, stick[j]))
        {
            Player.x -= Player.move_x;
            Player.y -= Player.move_y;
        }
    }
}

void UpdatePlayerBullets()
{
    if (mouse->GetState().leftButton && GetTickCount() - cooling > player_cooldown)
    {
        gunfire->Play();
        cooling = GetTickCount();
        last_bullet++;  // a new bullet is fired

        // if has reached the end of the bullet array
        // go back to the beginning the array and restart
        if (last_bullet >= PLAYER_BULLETS_NUMBER)
        {
            last_bullet = 0;
        }

        // the new bullet is fired from the center of the tank
        bullet[last_bullet].x = Player.x + Player.frame_width / 2 - bullet[last_bullet].frame_width/2;
        bullet[last_bullet].y = Player.y + Player.frame_height / 2 - bullet[last_bullet].frame_height/2;
        bullet_valid[last_bullet] = true;

        // the new bullet direction is same with tank direction
        switch (Player.frame)
        {
        case 0:
            bullet[last_bullet].move_x = 0;
            bullet[last_bullet].move_y = -bullet_speed;
            break;
        case 1:
            bullet[last_bullet].move_x = bullet_speed;
            bullet[last_bullet].move_y = 0;
            break;
        case 2:
            bullet[last_bullet].move_x = 0;
            bullet[last_bullet].move_y = bullet_speed;
            break;
        case 3:
            bullet[last_bullet].move_x = -bullet_speed;
            bullet[last_bullet].move_y = 0;
            break;
        }

        bullet[last_bullet].x += bullet[last_bullet].move_x;
        bullet[last_bullet].y += bullet[last_bullet].move_y;
    }

    for (int i = 0; i < PLAYER_BULLETS_NUMBER; i++)
    {
        bullet[i].x += bullet[i].move_x;
        bullet[i].y += bullet[i].move_y;

        // if bullet fly out of screen
        if (bullet[i].x<0 || bullet[i].x>Width || bullet[i].y<0 || bullet[i].y>Height)
        {
            bullet[i].move_x = 0;
            bullet[i].move_y = 0;
            bullet_valid[i] = false;
        }

        for (int j = 0; j < STONES_NUMBER; j++)
        {
            if (bullet_valid[i] && stone_valid[j] && CheckModel2DCollided(bullet[i], stone[j]))
            {
                bullet[i].move_x = 0;  
                bullet[i].move_y = 0;
                bullet_valid[i] = false;  
                stone_valid[j] = false;

                last_explosion++;
                if (last_explosion >= PLAYER_BULLETS_NUMBER)
                {
                    last_explosion = 0;
                }
                 
                explosion[last_explosion].x = stone[j].x;
                explosion[last_explosion].y = stone[j].y;
                explosion[last_explosion].frame = 0;
                explosion_valid[last_explosion] = true;
                explode->Play();
            }
        }

        for (int j = 0; j < BOXES_NUMBER; j++)
        {
            if (bullet_valid[i] && box_valid[j] && CheckModel2DCollided(bullet[i], box[j]))
            {
                bullet[i].move_x = 0;
                bullet[i].move_y = 0;
                bullet_valid[i] = false;
                box_valid[j] = false;

                last_explosion++;
                if (last_explosion >= PLAYER_BULLETS_NUMBER)
                {
                    last_explosion = 0;
                }

                explosion[last_explosion].x = box[j].x;
                explosion[last_explosion].y = box[j].y;
                explosion_valid[last_explosion] = true;
                explode->Play();
            }
        }

        for (int j = 0; j < STICKS_NUMBER; j++)
        {
            if (bullet_valid[i] && stick_valid[j] && CheckModel2DCollided(bullet[i], stick[j]))
            {
                bullet[i].move_x = 0;
                bullet[i].move_y = 0;
                bullet_valid[i] = false;
                stick_valid[j] = false;

                last_explosion++;
                if (last_explosion >= PLAYER_BULLETS_NUMBER)
                {
                    last_explosion = 0;
                }

                explosion[last_explosion].x = stick[j].x;
                explosion[last_explosion].y = stick[j].y;
                explosion_valid[last_explosion] = true;
                explode->Play();
            }
        }

    }
}

void UpdatePlayerExplosions()
{
    for (int i = 0; i < PLAYER_BULLETS_NUMBER; i++)
    {
        if (explosion_valid[i])
        {
            explosion[i].frame++;
        }

        if (explosion[i].frame >= explosion[i].frame_total)
        {
            explosion_valid[i] = false;
        }
    }
}

bool Game_Init(HWND hwnd)
{
    gameover = false;
    InitD3D(hwnd);
    InitInput(hwnd);
    InitSound();

    srand(GetTickCount());

    LoadPlayer();
    LoadStones();
    LoadBoxes();
    LoadSticks();
    LoadPlayerBullets();
    LoadPlayerExplsions();

    //placeholder
    background = CreateModel2D(L"background.png");
    if (background.texture == NULL)
    {
        MessageBox(NULL, L"Loading background.png error", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    gunfire = LoadSound(L"gunfire.wav");
    if (gunfire == nullptr)
    {
        MessageBox(NULL, L"Loading gunfire.wav error", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    explode = LoadSound(L"explode.wav");
    if (gunfire == nullptr)
    {
        MessageBox(NULL, L"Loading explode.wav error", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

void Game_Run()
{
    long static start = 0;
    float frame_interval = 1000.0 / framerate;

    if (keyboard->GetState().Escape)
    {
        gameover = true;
    }

    if (GetTickCount() - start >= frame_interval)
    {
        start = GetTickCount();
        ClearScreen();

        UpdatePlayer();
        UpdatePlayerBullets();
        UpdatePlayerExplosions();

        spriteBatch->Begin();

        DrawModel2D(background);
        DrawModel2D(Player);

        for (int i = 0; i < STONES_NUMBER; i++)
        {
            if (stone_valid[i])
            {
                DrawModel2D(stone[i]);
            }
        }

        for (int i = 0; i < BOXES_NUMBER; i++)
        {
            if (box_valid[i])
            {
                DrawModel2D(box[i]);
            }
        }

        for (int i = 0; i < STICKS_NUMBER; i++)
        {
            if (stick_valid[i])
            {
                DrawModel2D(stick[i]);
            }
        }

        for (int i = 0; i < PLAYER_BULLETS_NUMBER; i++)
        {
            if (bullet_valid[i])
            {
                DrawModel2D(bullet[i]);
            }
        }

        for (int i = 0; i < PLAYER_BULLETS_NUMBER; i++)
        {
            if (explosion_valid[i])
            {
                DrawModel2D(explosion[i]);
            }
        }

        spriteBatch->End();
        swapchain->Present(0, 0);
    }
}

void Game_End()
{
    Player.texture->Release();
    background.texture->Release();
    gunfire.release();
    explode.release();

    for (int i = 0; i < STONES_NUMBER; i++)
    {
        stone[i].texture->Release();
    }

    for (int i = 0; i < BOXES_NUMBER; i++)
    {
        box[i].texture->Release();
    }

    for (int i = 0; i < STICKS_NUMBER; i++)
    {
        stick[i].texture->Release();
    }

    for (int i = 0; i < PLAYER_BULLETS_NUMBER; i++)
    {
        bullet[i].texture->Release();
    }

    CleanD3D();
}

