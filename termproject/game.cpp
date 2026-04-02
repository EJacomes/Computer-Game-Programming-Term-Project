#include "DirectX.h"

//game is gonna be a wave based survival game
//max waves: 10
//User will start out with a slow speed, slow fire rate, slow damage, low health
//User will have the chance to upgrade those elements after the end of each round
//The amount of enemy that user will have to face is based on starting wave + (wave num * 5)
//enemies will only have one health and will give the player a certain amount of money to spend for upgrades


/*Checklist to complete:
Please mark the date of successfull completion of item

1. implement placeholder background and player movement (3/29/26, update this when you can make the model small) (update: 3/30/26)
2. implement random tiles (stones, boxes, sticks, etc) 
3. implement bigger background and adjust the code for it (4/1/26)
5. implement gunfire and explosion (look at the tank game) (have them set to the lowest firerate)
4. intialize speed, fire rate, damage, health for later on when the upgrading features is being worked on
5. implement enemy function and complete enemy design
6. implement reset feature where the enemies and tiles are spawned in after all do the enemies are defeated and the user allows the game to continue (use message box)
7. implement the UI for the game (pausing, wave over screen, buttons to lead to upgrade shop or next round, game over screen with score and money
and buttons to reset or quit), upgrade shop, health bar that changes during game play
8. Work on getting the upgrade shop working to where the upgrade are fully working
9. Get the next wave feature working and shown to the user
10. enemy movement should be more dynamic

*if there is anymore, then it should be added onto the list
*/

//game settings
Model2D Player, background;
Keyboard::KeyboardStateTracker keyboardtracker;
Mouse::ButtonStateTracker mousetracker;
std::unique_ptr<SpriteFont> spriteFont;

//tile settings
//stones settings
const int STONES_NUMBER = 40;
const int stone_width = 50;
const int stone_height = 29;
const int stone_rows = Height / stone_height;
const int stone_columns = Width / stone_width;
Model2D stone[STONES_NUMBER];
bool stone_valid[STONES_NUMBER];

//box settings
//stick settings
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
//this player_speed variable can be changed as it gets upgraded
//start out slow, each upgrade increases the speed by +1
float player_speed = 5;

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

    // if collided with stone, tank move back to previous position
    for (int j = 0; j < STONES_NUMBER; j++)
    {
        if (stone_valid[j] && CheckModel2DCollided(Player, stone[j]))
        {
            Player.x -= Player.move_x;
            Player.y -= Player.move_y;
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

    //placeholder
    background = CreateModel2D(L"background.png");
    if (background.texture == NULL)
    {
        MessageBox(NULL, L"Loading background.png error", L"Error", MB_OK | MB_ICONERROR);
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

        spriteBatch->End();
        swapchain->Present(0, 0);
    }
}

void Game_End()
{
    Player.texture->Release();
    background.texture->Release();

    for (int i = 0; i < STONES_NUMBER; i++)
    {
        stone[i].texture->Release();
    }

    CleanD3D();
}

