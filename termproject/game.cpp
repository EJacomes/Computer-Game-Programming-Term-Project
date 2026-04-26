#include "DirectX.h"

//game is gonna be a wave based survival game
//max waves: 10
//User will start out with a slow speed, slow fire rate, low health
//User will have the chance to upgrade those elements after the end of each round
//The amount of enemy that user will have to face is based on starting starting amount of enemies + (wave num - 1 * 5) (starting amount is 10)
//enemies will only have one health and will give the player a certain amount of money to spend for upgrades


/*Checklist to complete:
Please mark the date of successfull completion of item

1. implement placeholder background and player movement (3/29/26, update this when you can make the model small) (update: 3/30/26)
2. implement random tiles (stones, boxes, sticks, etc) (4/5/26)
3. implement bigger background and adjust the code for it (4/1/26)
4. implement gunfire and explosion (look at the tank game) (have them set to the lowest firerate) (4/5/26)
5. intialize speed, fire rate, health for later on when the upgrading features is being worked on (4/5/26)
6. implement enemy function and complete enemy design (4/11/26)
6.5. Fix the health issue where the player loses health 
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

//variables that will change as the rounds keep on going
//wave 0 - 9 is the same as wave 1 - 10
int waves = 0;
//since the Model2D, I will have to declare the amount of numbers per wave manually and set all of those as consts;
//4/8/26, 7:43 pm: new plan, I will set the Model2D enemy amount to the final amount of enemies faced  in wave 10 then only call a small amount of enemies in per waves
//This will use an array of the amount of enemies there will be per wave (10, 20, 25, 30, 35, 40, 45, 50, 55, 60) with their corresponding index (index 0 = wave 1, index 1 = wave 2... index 9 = wave 10
int enemy_amounts[10] = {10, 20, 25, 30, 35, 40, 45, 50, 55, 60};

//add the other variables from wave 2 to 10
int money_dropped = 5;

//total money earned 
int money_earned = 0;
//player score
int score = 0;
//tracks amount of enemy dead
int killTracker = 0;

//enemy settings
//need to make the drawing of the enemy
//if enemy needs to a const variable to make those enemy npcs
//60 is based on the amount of enemies called in wave 10
const int total_amount = 60;
Model2D enemy[total_amount];
bool enemy_valid[total_amount];


//may add the road_pos just for the player to have one spot to constantly spawn in
int road_pos_y = 700;
int road_pos_x = 935;

//upgradeable features
//this player_speed variable can be changed as it gets upgraded
//start out slow, each upgrade increases the speed by +1
float player_speed = 5;
int SpeedCost = 2;

//the variable cooldown is related to the rate of fire that the player has (starting firerate: 1000, max firerate: 50)
//enemies have a seperate cooldown
int player_cooldown = 1000; // allow to fire every x ms
int CooldownCost = 2;

//the variable player_health will start at 3 but will have a max of 15
int player_health = 3;
int HealthCost = 2;

int hit = -1; //this will serve in place of the die var from tank game
//since the bullet with hit the player three times when hit with a bullet under player_health
//using hit like how die was used can allowed for the player to get hit one time

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
const int enemy_cooldown = 200;
int enemy_cooling[total_amount];
const int enemy_speed = 10;
float enemy_movement_speed = enemy_speed / 2;
const int ENEMY_BULLET_NUMBER = 100;
Model2D Enemybullet[ENEMY_BULLET_NUMBER];
bool enemy_bullet_valid[ENEMY_BULLET_NUMBER];
int enemy_last_bullet = -1;
float enemy_bullet_speed = 2 * enemy_speed;

Model2D enemyexplosion[ENEMY_BULLET_NUMBER];
bool enemyexplosion_valid[ENEMY_BULLET_NUMBER];
int enemy_last_explosion = -1;

//methods to update the elements that are everchanging
void UpgradeSpeed()
{

    if (money_earned >= SpeedCost)
    {
        player_speed++;
        money_earned = money_earned - SpeedCost;
        SpeedCost = SpeedCost + 2;
        MessageBox(NULL, L"Upgrade brought", L"Upgrade has been gotten", MB_OK | MB_ICONERROR);
    }
    else
    {
        MessageBox(NULL, L"Unable to buy upgrade", L"Not enough funds", MB_OK | MB_ICONERROR);
    }

}

void UpgradeCooldown()
{
    if (money_earned >= CooldownCost)
    {
        if (player_cooldown == 50)
        {
            MessageBox(NULL, L"Max Upgrade level reached", L"Max cooldown reached", MB_OK | MB_ICONERROR);
        }
        else
        {
            player_cooldown = player_cooldown - 50;
            money_earned = money_earned - CooldownCost;
            CooldownCost = CooldownCost + 2;
            MessageBox(NULL, L"Upgrade brought", L"Upgrade has been gotten", MB_OK | MB_ICONERROR);
        }
    }
    else
    {
        MessageBox(NULL, L"Unable to buy upgrade", L"Not enough funds", MB_OK | MB_ICONERROR);
    }
}

void UpgradeHealth()
{
    if (money_earned >= HealthCost)
    {
        if (player_health == 15)
        {
            MessageBox(NULL, L"Max Upgrade level reached", L"Max health reached", MB_OK | MB_ICONERROR);
        }
        else
        {
            player_health++;
            money_earned = money_earned - HealthCost;
            HealthCost = HealthCost + 2;
            MessageBox(NULL, L"Upgrade brought", L"Upgrade has been gotten", MB_OK | MB_ICONERROR);
        }
    }
}

void NextWave()
{
    //resetting killTracker and incrementing the wave
    killTracker = 0;
    waves++;

    //code for upgrade path
    //starting with the message stating they won the round and if they want to go to the upgrades
    //it will be a yes or no (yes will start them on the upgrades path, no will start the next round)
    int message1 = MessageBox(NULL, L"Wave Completed!", L"Do you want to go to the upgrade section?", MB_YESNO | MB_ICONQUESTION);

    if (message1 == IDYES)
    {
        //this is where the code for the upgrades will be in 
        //introduction box that tells the player they are in the upgrade section and that they move the dialog boxes to see the other upgrades
        int introMessage = MessageBox(NULL, L"Welcome to the upgrade session", L"Move the message boxes to see the upgrade options", MB_OK | MB_ICONEXCLAMATION);

        //Click on the messagebox does work but I need to put in a loop that keeps going until the the user decides to stop upgrading then calls the WaveReset function
        //given how all of them must be active as long as the user is still on the upgrade section, they must have seperate loops and when the variables that controls the loops are all false thats when WaveReset is called
        //One loop for speed upgrade
        //One loop for cooldown upgrade
        //One loop for Health upgrade

        //if all of these go false then WaveReset is called
        bool SpeedUpgrading = true;
        bool CooldownUpgrading = true;
        bool HealthUpgrading = true;

        //intializing all of these variables related to the messagebox
        int SpeedMessage;
        int CooldownMessage;
        int HealthMessage;
        

        if (introMessage == IDOK)
        {

            while (SpeedUpgrading == true)
            {
                SpeedMessage = MessageBox(NULL, L"Do you want to upgrade your speed value?", L"Click yes to upgrade, click no to get rid of this box", MB_YESNO | MB_ICONQUESTION);

                if (SpeedMessage == IDYES)
                {
                    UpgradeSpeed();
                }
                else if (SpeedMessage == IDNO)
                {
                    SpeedUpgrading = false;
                }
            }

            while (CooldownUpgrading == true)
            {
                CooldownMessage = MessageBox(NULL, L"Do you want to upgrade your cooldown?", L"Click yes to upgrade, click no to get rid of this box", MB_YESNO | MB_ICONQUESTION);

                if (CooldownMessage == IDYES)
                {
                    UpgradeCooldown();
                }
                else if (CooldownMessage == IDNO)
                {
                    CooldownUpgrading = false;

                }
            }

            while (HealthUpgrading == true)
            {
                HealthMessage = MessageBox(NULL, L"Do you want to upgrade your health points?", L"Click yes to upgrade, click no to get rid of this box", MB_YESNO | MB_ICONQUESTION);

                if (HealthMessage == IDYES)
                {
                    UpgradeHealth();
                }
                else if (HealthMessage == IDNO)
                {
                    HealthUpgrading = false;
                }
            }

            if (SpeedUpgrading == false && CooldownUpgrading == false && HealthUpgrading == false)
            {
                //WaveReset();

                //placeholder until WaveReset is made
                gameover = true;
            }
            /*SpeedMessage = MessageBox(NULL, L"Do you want to upgrade your speed value?", L"Click yes to upgrade, click no to get rid of this box", MB_YESNO | MB_ICONQUESTION);

            CooldownMessage = MessageBox(NULL, L"Do you want to upgrade your cooldown?", L"Click yes to upgrade, click no to get rid of this box", MB_YESNO | MB_ICONQUESTION);

            HealthMessage = MessageBox(NULL, L"Do you want to upgrade your health points?", L"Click yes to upgrade, click no to get rid of this box", MB_YESNO | MB_ICONQUESTION);

            if (SpeedMessage == IDYES)
            {
                UpgradeSpeed();
            }
            else if (CooldownMessage == IDYES)
            {
                UpgradeCooldown();
            }
            else if (HealthMessage == IDYES)
            {
                UpgradeHealth();
            }*/
        }
    }
    else if (message1 == IDNO)
    {
        //WaveReset();

        //placeholder until WaveReset is made
        gameover = true;
    }
}

//void WaveReset(){}


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

void LoadEnemyBullets()
{
    for (int i = 0; i < ENEMY_BULLET_NUMBER; i++)
    {
        Enemybullet[i] = CreateModel2D(L"bullet.png");
        Enemybullet[i].move_x = 0;
        Enemybullet[i].move_y = 0;
        enemy_bullet_valid[i] = false;

        //MessageBox(NULL, L"Loading enemy bullet", L"message", MB_OK);
    }
}

void LoadEnemyExplosions()
{
    for (int i = 0; i < ENEMY_BULLET_NUMBER; i++)
    {
        enemyexplosion[i] = CreateModel2D(L"explosion.png", 8, 8);
        enemyexplosion_valid[i] = false;
    }
}

void LoadEnemies()
{
    for (int i = 0; i < enemy_amounts[waves]; i++)
    {
        enemy[i] = CreateModel2D(L"enemy_model.png", 4, 1);
        enemy_valid[i] = true;

        enemy[i].x = rand() % (Width - Player.frame_width);
        enemy[i].y = rand() % (Height - Player.frame_height);

        //random direction
        switch (rand() % 4)
        {
        case 0:
            enemy[i].frame = 0;
            enemy[i].move_x = 0;
            enemy[i].move_y = -enemy_speed;
            break;
        case 1:
            enemy[i].frame = 1;
            enemy[i].move_x = -enemy_speed;
            enemy[i].move_y = 0;
            break;
        case 2:
            enemy[i].frame = 2;
            enemy[i].move_x = 0;
            enemy[i].move_y = enemy_speed;
        case 3:
            enemy[i].frame = 3;
            enemy[i].move_x = enemy_speed;
            enemy[i].move_y = 0;
            break;
        }

        for (int j = 0; j < STONES_NUMBER; j++)
        {
            if (stone_valid[j] && CheckModel2DCollided(enemy[i], stone[j]))
            {
                ZeroMemory(&stone[j], sizeof(Model2D));
                stone_valid[j] = false;
            }
        }

        //make the loops for boxes and sticks
        for (int j = 0; j < BOXES_NUMBER; j++)
        {
            if (box_valid[j] && CheckModel2DCollided(enemy[i], box[j]))
            {
                ZeroMemory(&box[j], sizeof(Model2D));
                box_valid[j] = false;
            }
        }
        
        for (int j = 0; j < STICKS_NUMBER; j++)
        {
            if (stick_valid[j] && CheckModel2DCollided(enemy[i], stick[j]))
            {
                ZeroMemory(&stick[j], sizeof(Model2D));
                stick_valid[j] = false;
            }
        }

        //MessageBox(NULL, L"loads enemy", L"Game Over", MB_OK);
        //this shows that the 10 enemies are getting loaded in
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

    for (int i = 0; i < ENEMY_BULLET_NUMBER; i++)
    {
       if (enemy_bullet_valid[i] && CheckModel2DCollided(Player, Enemybullet[i]) && hit < 0)
       {
           enemy_last_explosion++;

           if (enemy_last_explosion >= ENEMY_BULLET_NUMBER)
           {
               enemy_last_explosion = 0;
           }

           enemyexplosion[enemy_last_explosion].x = Player.x;
           enemyexplosion[enemy_last_explosion].y = Player.y;
           enemyexplosion[enemy_last_explosion].frame = 0;
           enemyexplosion_valid[enemy_last_explosion] = true;
           //player_health -= 1;
           hit = enemy_last_explosion;
           //MessageBox(NULL, L"Player health", L"hit", MB_OK);
           //if one bullet hits the player then the box will be played three times

           explode->Play();
           
           if (player_health < 0)
           {
               player_health = 0;
           }
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

    /*if (player_health <= 0 && enemyexplosion[player_health].frame >= enemyexplosion[player_health].frame_total)
    {
        MessageBox(NULL, L"Defeated", L"Game Over", MB_OK);
        gameover = true;
    }*/

    if (hit >= 0)
    {
        //MessageBox(NULL, L"testing", L"test", MB_OK);
        player_health -= 1;
        hit = -1;
    }

    if (player_health <= 0)
    {
        MessageBox(NULL, L"Defeated", L"Game Over", MB_OK);
        gameover = true;
    }

    /*if (score == enemy_amounts[waves])
    {
        //this could be where the wave function is set
        MessageBox(NULL, L"Victory", L"Game Over", MB_OK);
        //replace this later
        gameover = true;
    }*/
}

void UpdateEnemies()
{
    for (int i = 0; i < enemy_amounts[waves]; i++)
    {
        if (enemy_valid[i])
        {
            //if reaches the edge then it turns around
            enemy[i].x += enemy[i].move_x;
            enemy[i].y += enemy[i].move_y;

            if (enemy[i].x < 0)
            {
                enemy[i].x = 0;
                enemy[i].move_x *= -1;
            }

            if (enemy[i].x > Width - enemy[i].frame_width)
            {
                enemy[i].x = Width - enemy[i].frame_width;
                enemy[i].move_x *= -1;
            }

            if (enemy[i].y < 0)
            {
                enemy[i].y = 0;
                enemy[i].move_y *= -1;
            }

            if (enemy[i].y > Height - enemy[i].frame_height)
            {
                enemy[i].y = Height - enemy[i].frame_height;
                enemy[i].move_y *= -1;
            }

            //if collided with stone, box, stick then reverse the direction
            for (int j = 0; j < STONES_NUMBER; j++)
            {
                if (stone_valid[j] && CheckModel2DCollided(enemy[i], stone[j]))
                {
                    enemy[i].move_x *= -1;
                    enemy[i].move_y *= -1;
                }
            }

            for (int j = 0; j < BOXES_NUMBER; j++)
            {
                if (box_valid[j] && CheckModel2DCollided(enemy[i], box[j]))
                {
                    enemy[i].move_x *= -1;
                    enemy[i].move_y *= -1;
                }
            }

            for (int j = 0; j < STICKS_NUMBER; j++)
            {
                if (stick_valid[j] && CheckModel2DCollided(enemy[i], stick[j]))
                {
                    enemy[i].move_x *= -1;
                    enemy[i].move_y *= -1;
                }
            }

            //putting in the proper frame when on the direction they are facing
            if (enemy[i].move_x < 0)
            {
                enemy[i].frame = 3;
            }

            if (enemy[i].move_x > 0)
            {
                enemy[i].frame = 1;
            }

            if (enemy[i].move_y < 0)
            {
                enemy[i].frame = 0;
            }

            if (enemy[i].move_y > 0)
            {
                enemy[i].frame = 2;
            }

            //if enemy is hit by player bullet then it explodes
            for (int j = 0; j < PLAYER_BULLETS_NUMBER; j++)
            {
                if (bullet_valid[j] && CheckModel2DCollided(enemy[i], bullet[j]))
                {
                    bullet[j].move_x = 0;
                    bullet[j].move_y = 0;
                    bullet_valid[j] = false;
                    enemy_valid[i] = false;
                    score++;
                    killTracker++;
                    money_earned += money_dropped;
                    last_explosion++;

                    if (last_explosion >= PLAYER_BULLETS_NUMBER)
                    {
                        last_explosion = 0;
                    }

                    explosion[last_explosion].x = enemy[i].x;
                    explosion[last_explosion].y = enemy[i].y;
                    explosion[last_explosion].frame = 0;
                    explosion_valid[last_explosion] = true;
                    explode->Play();
                }
            }

            bool firebullet = false;
            //need to use UpdateEnemyBullets() and UpdateEnemyExplosion() (may not)
            //if enemy and player in the same cell columns
            //slide 37
            if (Player.x - Player.frame_width < enemy[i].x && enemy[i].x < Player.x + Player.frame_width)
            {
                //if the enemy is point to the same direction to the player
                if (enemy[i].move_y < 0 && enemy[i].y > Player.y)
                {
                    firebullet = true;
                }

                if (enemy[i].move_y > 0 && enemy[i].y < Player.y)
                {
                    firebullet = true;
                }
            }

            //if enemy and the player are in the same cell row
            if (Player.y - Player.frame_height < enemy[i].y && enemy[i].y < Player.y + Player.frame_height)
            {
                //if enemy's direction point to Player
                if (enemy[i].move_x < 0 && enemy[i].x > Player.x)
                {
                    firebullet = true;
                }

                if (enemy[i].move_x > 0 && enemy[i].x < Player.x)
                {
                    firebullet = true;
                }
            }

            //shooting at the player
            if (firebullet && GetTickCount() - enemy_cooling[i] > enemy_cooldown)
            {
                gunfire->Play();
                enemy_cooling[i] = GetTickCount();
                enemy_last_bullet++; //leads to a new bullet being fired

                //if the end of the bullet array is reached then it will go back to the beginning the array and restart
                if (enemy_last_bullet >= ENEMY_BULLET_NUMBER)
                {
                    enemy_last_bullet = 0;
                }

                //the bullet will be fired from the center
                Enemybullet[enemy_last_bullet].x = enemy[i].x + enemy[i].frame_width / 2 - Enemybullet[enemy_last_bullet].frame_width / 2;
                Enemybullet[enemy_last_bullet].y = enemy[i].y + enemy[i].frame_height / 2 - Enemybullet[enemy_last_bullet].frame_height / 2;
                enemy_bullet_valid[enemy_last_bullet] = true;

                //the bullet direction will be the same as the player
                switch (enemy[i].frame)
                {
                case 0:
                    Enemybullet[enemy_last_bullet].move_x = 0;
                    Enemybullet[enemy_last_bullet].move_y = -enemy_bullet_speed;
                    break;
                case 1:
                    Enemybullet[enemy_last_bullet].move_x = enemy_bullet_speed;
                    Enemybullet[enemy_last_bullet].move_y = 0;
                    break;
                case 2:
                    Enemybullet[enemy_last_bullet].move_x = 0;
                    Enemybullet[enemy_last_bullet].move_y = enemy_bullet_speed;
                    break;
                case 3:
                    Enemybullet[enemy_last_bullet].move_x = -enemy_bullet_speed;
                    Enemybullet[enemy_last_bullet].move_y = 0;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < ENEMY_BULLET_NUMBER; i++)
    {
        if (enemy_bullet_valid[i])
        {
            Enemybullet[i].x += Enemybullet[i].move_x;
            Enemybullet[i].y += Enemybullet[i].move_y;

            //if enemy bullet is out of screen
            if (Enemybullet[i].x < 0 || Enemybullet[i].x > Width || Enemybullet[i].y < 0 || Enemybullet[i].y > Height)
            {
                Enemybullet[i].move_x = 0;
                Enemybullet[i].move_y = 0;
                enemy_bullet_valid[i] = false;
            }
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
    LoadEnemyBullets();
    LoadEnemyExplosions();
    LoadEnemies();

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
        UpdateEnemies();

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

        for (int i = 0; i < ENEMY_BULLET_NUMBER; i++)
        {
            if (enemy_bullet_valid[i])
            {
                DrawModel2D(Enemybullet[i]);
            }
        }

        for (int i = 0; i < ENEMY_BULLET_NUMBER; i++)
        {
            if (enemyexplosion_valid[i])
            {
                DrawModel2D(enemyexplosion[i]);
            }
        }

        for (int i = 0; i < enemy_amounts[waves]; i++)
        {
            if (enemy_valid[i])
            {
                DrawModel2D(enemy[i]);
            }
        }

        //need to add the spritefont and music
        //For spritefont, add the health of the player, score, and cash

        //this can be triggered if it is in the Game_Run method
        //a different variable other then score needs to be used so that way it can track the enemies defeated per round
        if (killTracker == enemy_amounts[waves])
        {
            //in order to end the game, the score has to be the sum of all enemies faced since score will not restart per round
            if (score == 370)
            {
                MessageBox(NULL, L"Victory, you saved the sector from the enemies", L"Game Over", MB_OK);
                gameover = true;
            }
            else
            {
                //when else is called that is the sign that the wave is over and a new wave needs to be set up
                // killTracker needs to be reset to 0
                // wave variable needs to be increment by 1
                // this should have a method called where it will show a text box containing different options 
                // these options will about health, cooldown, and speed
                // there should also be a button to go to the next wave as well
                // that button that leads to the next wave need to have a different method where it reset every
                // as of 4/20/26: work on get the next wave and resetting everything to work
                //the method name for the method to change the wave and do the upgrades will be called, NextWave()
                //killTracker restart and wave increment will be in NextWave()
                //the method name for the method to reset everything for the next wave will be call WaveReset()
                
                NextWave();

                //MessageBox(NULL, L"Victory", L"Game Over", MB_OK);
                //gameover = true;
            }



            //this could be where the wave function is set
            //MessageBox(NULL, L"Victory", L"Game Over", MB_OK);
            //MessageBox(NULL, L"Proceed?", L"Confirm", MB_YESNO | MB_ICONQUESTION);
            //replace this later
            //gameover = true;
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

    for (int i = 0; i < ENEMY_BULLET_NUMBER; i++)
    {
        Enemybullet[i].texture->Release();
    }

    for (int i = 0; i < enemy_amounts[waves]; i++)
    {
        enemy[i].texture->Release();
    }

    CleanD3D();
}

