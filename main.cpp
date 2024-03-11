#include "include/raylib.h"
#include "include/raymath.h"
#include <climits>
#include <vector>
#include <queue>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <stack>
#include <set>
#include <fstream>
#include <sstream>


using namespace std;


const int screenWidth = 800;
const int screenHeight = 800;
const int mapWidth = 15;
const int mapHeight = 15;
const int gameDifficulty = 32;
const int STUNCOUNT = 5;
const int STEPCOUNTMAX = 26;
int highScore = 0;
int gameChallenge = 1;
int lolvar = 0;
int dx[] = {-1, 1, 0, 0};
int dy[] = {0, 0, -1, 1};
int coinCount = 0;
bool playerTurn = true;
bool gameOver = true;
bool increasing = true;
bool loadGame = false;
bool inGame = false;
bool hasPopulated = false;
bool songNotPlayed = false;
Sound GameTheme;
Sound lvlMusic;
float rgbval = 50.0f;
bool homeScreen = true;
Camera2D camera = { 0 };
Color textColor = PURPLE;
Vector2 mousePoint = GetMousePosition();
Font font;
Texture2D numBoard1, numBoard2, numBoard3;



int direct[][2] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};

class MazeCreation
{
    public:
        int countVisitedNeighbor(char (&gameMap)[mapWidth][mapHeight], int i, int j){
            
            int count = 0;
            for (int k = 0; k < 4; ++k)
            {
                int ni = i + direct[k][0];
                int nj = j + direct[k][1];
                //out of boundary
                if(ni < 0 || nj < 0 || ni >= mapWidth || nj >= mapHeight) continue;
                if(gameMap[ni][nj] == 1) count++;//visited
            }
            return count;
        }

        void shuffle(int a[], int n){
            for (int i = 0; i < n; ++i)
            {
                swap(a[i], a[rand() % n]);
            }
        }

        void swap(int & a, int &b){
            int c = a;
            a = b;
            b = c;
        }


        void MazeGeneration(char (&gameMap)[mapWidth][mapHeight], int i, int j)
        {


            int visitOrder[] = {0, 1, 2, 3};

            if (i < 0 || j < 0 || i >= mapWidth || j >= mapHeight) return;

            // visited
            if (gameMap[i][j] == 1) return;

            if (countVisitedNeighbor(gameMap, i, j) > 1) return;

            // visited
            gameMap[i][j] = 1;

            //shuffle the visitOrder
            shuffle(visitOrder, 4);

            for (int k = 0; k < 4; ++k)
            {
                int ni = i + direct[visitOrder[k]][0];
                int nj = j + direct[visitOrder[k]][1];
                MazeGeneration(gameMap, ni, nj);
            }
            

        }

};




struct Point {

    int x;

    int y;

    std::queue<Point> parent;

    bool operator<(const Point& p) const noexcept {

    return x == p.x ? y < p.y : x < p.x;

    }  
};


struct Coin
{
    Texture2D sprite;
    Rectangle representCoin;
    Vector2 position;
    bool isCollected;
    Sound collectFruit;
    Sound lvlChange;
};


typedef struct {
    Texture2D sprite;
    Texture2D attackRightSprite;
    Texture2D attackLeftSprite;
    Texture2D attackUpSprite;
    Texture2D attackDownSprite;
    Vector2 position;
    Rectangle representation;
    Point playerPoint;
    int stepCount = 0;
    int fruitCollected = 0;
    int canAttackCount = 0;
    int portalUse = 0;
    bool attackUp = false;
    bool attackDown = false;
    bool attackLeft = false;
    bool attackRight = false;
    bool stationary = true;
    bool canAttack = true;
    bool justTeleported = false;
    string canAttackString = "Yes";
} Player;

typedef struct {
    Texture2D sprite;
    Vector2 position;
    Sound portalSound;
    Rectangle portalRepresentation;
    bool isActivated = false;
    string isActivatedString = "No";
} Portal;

typedef struct
{
    Rectangle wepReresentation;
    Sound attackSound;
    bool isUsed = false;
    bool isActive = true;
}Weapon;

typedef struct 
{
    Texture2D sprite;
    Texture2D homeScreenSprite;
    Rectangle enemyRepresentation;
    Vector2 position;
    Point enemyPoint;
    bool stunned = false;
    bool enemyTurn = false;
    int stunCount = 0;
    bool desinationReached = false;
    bool isActive = true;
    queue<Point> pathStack; 
}Enemy;



void DrawMap(char (&gameMap)[mapWidth][mapHeight], Texture2D& wall_mabe,Texture2D& tree,Texture2D& grass, int gameDifficulty);
void PlayerMovement(Player &player, char (&gameMap)[mapWidth][mapHeight], int gameDifficulty,vector<Enemy> &enemies);
vector<Point> get_neighbors(Point &p, char (&gameMap)[mapWidth][mapHeight]);
void printPath(const Point &current);
std::queue<Point> getPath(const Point &current);
std::queue<Point> bfs(Point &start, Point &destination, char (&gameMap)[mapWidth][mapHeight], Enemy &enemy);
void PopulateCoins(vector<Coin> &coins, char (&gameMap)[mapWidth][mapHeight], int gameDifficulty);
void PopulatePortals(vector<Portal> &portals, char (&gameMap)[mapWidth][mapHeight], int gameDifficulty);
void PopulateEnemies(vector<Enemy> *enemies);
void GenerateMaze(MazeCreation &maze,char (&gameMap)[mapWidth][mapHeight], Point &playerPoint, vector<Enemy> *enemies, int i, int j);
void CheckCoinCollision(Player &player, vector<Coin> &coins, MazeCreation &maze, char (&gameMap)[mapWidth][mapHeight], vector <Enemy> *enemies, vector<Portal> &portals);
void CheckPortalCollision(Player &player, vector<Portal> &portals, char (&gameMap)[mapWidth][mapHeight]);
void DrawPortals(vector<Portal> &portals);
void WeaponAttack(Weapon &weapon, Enemy &enemy);
void EnemyMovement(vector<Enemy> &enemies, Player& player);
void DrawPlayer(Player &player);
void PlayerAttack(Player &player, vector<Enemy> *enemies, Weapon &weapon);
void LoadGame(Player &player, vector<Enemy>* enemies, Weapon &weapon, MazeCreation& maze, char (&gameMap)[mapWidth][mapHeight], vector<Coin> &coins, vector<Portal> &portals);
void HomeScreen(Player &player, Enemy &enemy, Coin &coin, Texture2D &boarder, Font &font);
void GameScreen(Player &player, vector<Enemy> &enemies, Weapon &weapon, MazeCreation &maze, char (&gameMap)[mapWidth][mapHeight], vector<Coin> &coins, vector<Portal> &portals, RenderTexture& screenCamera1, Camera2D& camera, Texture2D& wall_mabe, Texture2D& mazeTree, Texture2D& grass, bool &playerTurn, bool &gameOver, Font *font);
void HighScore(Player &player);
void InformationScreen();
void GetHighScore();


int main() {

    // MAIN VARIABLES -------------------------------------------------------
    float initialTime = GetTime();
    double waitTime = 0.65f;
    char gameMap[mapWidth][mapHeight];
    vector<Coin> coins;
    Player player;
    Weapon weapon;
    Enemy enemy;
    vector <Enemy> enemies;
    
    MazeCreation maze;
    

    vector<Portal> portals;
    

    // MAIN VARIABLES TEXTURES ----------------------------------------------------------------------------------------------------
    Texture2D wall_mabe;

    // MAIN VARIABLES POINTS ------------------------------------------------------------------------------------------------------
    
    
    
    srand((unsigned)time(0));
    //Starting positions -----------------------------------------------------------------------------------------------------------

    //second enemy -----------------------------------------------------------------------------------------------------------

 
     
    weapon.wepReresentation = {player.position.x,player.position.y,gameDifficulty,gameDifficulty};

    //initialize the window ----------------------------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "HARVEST");
    SetTargetFPS(60);


    InitAudioDevice();

    Coin coin1,coin2,coin3,coin4;


    enemy.sprite = LoadTexture("Resource/enemy.png");
    enemy.homeScreenSprite = LoadTexture("Resource/enemy_homescreen.png");

    coins.push_back(coin1);
    coins.push_back(coin2);
    coins.push_back(coin3);
    coins.push_back(coin4);   
    
    for (int i = 0; i < coins.size(); i++)
    {
        coins[i].sprite = LoadTexture("Resource/treefruit.png");
        coins[i].collectFruit = LoadSound("Resource/collect.wav");
        coins[i].lvlChange = LoadSound("Resource/lvlChange.wav");
    }
    float timeCounter = 0.f;
    wall_mabe = LoadTexture("Resource/wall_maybe.png");
    Texture2D mazeTree = LoadTexture("Resource/Tree.png");
    Texture2D grass = LoadTexture("Resource/grass.png");
    player.sprite = LoadTexture("Resource/player.png");
    player.attackLeftSprite = LoadTexture("Resource/playerSpriteLeft.png");
    player.attackRightSprite = LoadTexture("Resource/playerSpriteRight.png");
    player.attackUpSprite = LoadTexture("Resource/playerSpriteUp.png");
    player.attackDownSprite = LoadTexture("Resource/playerSpriteDown.png");
    GameTheme = LoadSound("Resource/Take_The_Baby2.mp3");
    lvlMusic = LoadSound("Resource/Jordan Hebert - Water-Tea.wav");
    
    font = LoadFont("Resource/setback.png");
    Texture2D boarder = LoadTexture("Resource/boarder.png");
    numBoard1 = LoadTexture("Resource/NumBoard1.png");
    numBoard2 = LoadTexture("Resource/NumBoard2.png");
    numBoard3 = LoadTexture("Resource/NumBoard3.png");
    weapon.attackSound = LoadSound("Resource/bash.wav");
    
    GetHighScore();
    


    // camera stuff ---------------------------------------------------------------------------------------------------------------
    
    camera.target = {90, 100 };
    camera.offset = { 200.0f, 200.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.30f;


    // initilize Game ---------------------------------------------------------------------------------------------------------------

    RenderTexture screenCamera1 = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture screenCamera2 = LoadRenderTexture(screenWidth/2, screenHeight);
    Rectangle splitScreenRect = { 0.0f, 0.0f, (float)screenCamera1.texture.width, (float)-screenCamera1.texture.height };
    SetExitKey(KEY_END);

    while (!WindowShouldClose()) {

    
                // Move the enemy along the path if there is a path
        
        BeginDrawing();

            if (gameOver)
            {
                if (homeScreen)
                {
                    HomeScreen(player,enemy,coins[0],boarder,font);
                    inGame = false;
                }
                else 
                {
                    InformationScreen();
                }

            }
            else
            {
                if (loadGame)
                {
                    LoadGame(player,&enemies,weapon,maze, gameMap,coins,portals);
                    loadGame = false;
                }
                inGame = true;
                
                GameScreen(player,enemies,weapon,maze,gameMap,coins,portals,screenCamera1,camera,wall_mabe,mazeTree,grass,playerTurn,gameOver, &font);
                DrawTextureRec(screenCamera1.texture,splitScreenRect, {0,0},WHITE);
            }
            ClearBackground(BLACK);

            
            
            // DrawRectangleLinesEx(splitScreenRect,5,BLUE);
        EndDrawing();
    }

    CloseWindow();
    UnloadRenderTexture(screenCamera1);
    UnloadRenderTexture(screenCamera2);
    UnloadTexture(wall_mabe);
    UnloadTexture(mazeTree);
    UnloadTexture(grass);
    UnloadTexture(player.sprite);
    UnloadTexture(enemy.sprite);

    UnloadTexture(player.attackLeftSprite);
    UnloadTexture(player.attackRightSprite);
    UnloadTexture(coin1.sprite);
    UnloadTexture(coin2.sprite);
    UnloadTexture(coin3.sprite);
    UnloadTexture(coin4.sprite);

    CloseAudioDevice();
    

    return 0;
}

void GetHighScore()
{
    // Open the file
    std::ifstream inFile("Resource/HighScore.txt");

    if (!inFile)
    {
        std::cerr << "Error opening file: high_score.txt" << std::endl;
        return;
    }

    std::string fileContents;
    getline(inFile, fileContents);

    // Find and extract the current high score from the file
    size_t startPos = fileContents.find("collected") + 9; // Length of "collected"
    size_t endPos = fileContents.find("fruit", startPos);

    highScore = std::stoi(fileContents.substr(startPos, endPos - startPos));

        // Close the file
    inFile.close();

}

void HighScore(Player &player)
{
    // Open the file
    std::ifstream inFile("Resource/HighScore.txt");

    if (!inFile)
    {
        std::cerr << "Error opening file: high_score.txt" << std::endl;
        return;
    }

    std::string fileContents;
    getline(inFile, fileContents);

    // Find and extract the current high score from the file
    size_t startPos = fileContents.find("collected") + 9; // Length of "collected"
    size_t endPos = fileContents.find("fruit", startPos);

    int highScore = std::stoi(fileContents.substr(startPos, endPos - startPos));

    // Check if player's fruitCollected is greater than the high score
    if (player.fruitCollected > highScore)
    {
        // Replace the old high score with the new one in the file contents
        std::ostringstream newHighScoreText;
        newHighScoreText << "Here likes an honorary harvester who collected " << player.fruitCollected << " fruit.";

        // Open the file in write mode to update the high score
        std::ofstream outFile("Resource/HighScore.txt");

        if (!outFile)
        {
            std::cerr << "Error opening file for writing: high_score.txt" << std::endl;
            return;
        }

        // Write the new high score text to the file
        outFile << newHighScoreText.str();
        std::cout << "New high score: " << player.fruitCollected << std::endl;
    }

    // Close the file
    inFile.close();
}



void PopulateEnemies(vector<Enemy> *enemies)
{
    if (!inGame)
    {
        for (int i = gameChallenge; 0 < i; i--)
        {
            Enemy enemy;
            switch (i)
            {
                case 1:
                    
                    enemy.position = {(mapWidth-1)*gameDifficulty,(mapHeight-1)*gameDifficulty};
                    enemy.enemyPoint = {static_cast<int>(enemy.position.x/gameDifficulty), static_cast<int>(enemy.position.y/gameDifficulty)};
                    enemy.enemyRepresentation = {enemy.position.x, enemy.position.y, gameDifficulty, gameDifficulty};
                    enemy.sprite = LoadTexture("Resource/enemy.png");
                    enemy.homeScreenSprite = LoadTexture("Resource/enemy.png");
                    break;
                case 2:
                    enemy.position = {0,(mapHeight-1)*gameDifficulty};
                    enemy.enemyPoint = {static_cast<int>(enemy.position.x/gameDifficulty), static_cast<int>(enemy.position.y/gameDifficulty)};
                    enemy.enemyRepresentation = {enemy.position.x, enemy.position.y, gameDifficulty, gameDifficulty};
                    enemy.sprite = LoadTexture("Resource/enemy.png");

                    break;
                case 3:
                    enemy.position = {(mapWidth-1)*gameDifficulty,0};
                    enemy.enemyPoint = {static_cast<int>(enemy.position.x/gameDifficulty), static_cast<int>(enemy.position.y/gameDifficulty)};
                    enemy.enemyRepresentation = {enemy.position.x, enemy.position.y, gameDifficulty, gameDifficulty};
                    enemy.sprite = LoadTexture("Resource/enemy.png");
                    break;
                default:    
                    break;
            }
            enemies->push_back(enemy);

        }
    }
    else if (inGame && !hasPopulated)
    {
        //add one more enemy to the game
        Enemy enemy;
        enemy.position = {(mapWidth-1)*gameDifficulty,0};
        enemy.enemyPoint = {static_cast<int>(enemy.position.x/gameDifficulty), static_cast<int>(enemy.position.y/gameDifficulty)};
        enemy.enemyRepresentation = {enemy.position.x, enemy.position.y, gameDifficulty, gameDifficulty};
        enemy.sprite = LoadTexture("Resource/enemy.png");
        enemies->push_back(enemy);
        hasPopulated = true;
    }
}

void GameScreen(Player &player, vector<Enemy> &enemies, Weapon &weapon, MazeCreation &maze, char (&gameMap)[mapWidth][mapHeight], vector<Coin> &coins, vector<Portal> &portals, RenderTexture& screenCamera1, Camera2D& camera, Texture2D& wall_mabe, Texture2D& mazeTree, Texture2D& grass, bool &playerTurn, bool &gameOver, Font  *font)
{
        // enemy.enemyPoint = {static_cast<int>(enemy.position.x/gameDifficulty), static_cast<int>(enemy.position.y/gameDifficulty)};
        if (!IsSoundPlaying(lvlMusic))
        {
            PlaySound(lvlMusic);
        }
        if (IsKeyPressed(KEY_ESCAPE))
        {
            gameOver = true;
        }
        if (playerTurn)
        {
            PlayerMovement(player,gameMap,gameDifficulty,enemies);
            CheckPortalCollision(player,portals,gameMap);
            if (player.stepCount % STEPCOUNTMAX == 0)
            {
                PopulateEnemies(&enemies);
                
                
            }
        }
 
        for (Enemy &enemy : enemies)
        {
            if (enemy.desinationReached == false )
            {
                enemy.pathStack = bfs(player.playerPoint,enemy.enemyPoint,gameMap,enemy);  
            } 
        }



        EnemyMovement(enemies,player);


        BeginTextureMode(screenCamera1);
            ClearBackground(BLACK);
            BeginMode2D(camera);
            DrawMap(gameMap,wall_mabe,mazeTree,grass,32);
            CheckCoinCollision(player,coins,maze,gameMap,&enemies,portals);
            DrawPortals(portals);


            DrawPlayer(player);
            for (Enemy &enemy: enemies)
            {
                DrawTextureEx(enemy.sprite, enemy.position, 0, 1, WHITE);
            }
            PlayerAttack(player,&enemies,weapon);

            
            // DrawTextEx(TextFormat("Fruit Saved: %i | Can Player Attack: %s | Portals Activated: %s", player.fruitCollected, player.canAttackString.c_str(), portals[0].isActivatedString.c_str()), -28, 515, 18, PURPLE);
            
            DrawTextEx(*font, TextFormat("Fruit Saved: %i | Remaining Attacks: %i \n \t \t \t \t Remaining Portal Jumps: %i", player.fruitCollected, (gameChallenge-player.canAttackCount), (gameChallenge+1 - player.portalUse)), Vector2{-16,515}, 16, 5, PURPLE);
            

            EndMode2D();
        EndTextureMode();
}

void InformationScreen()
{
    DrawTextEx(font, "-\t You are a noble harvester and \n \n your goal is to  collect the fruit \n \n before it is eaten by insects" , Vector2{20, 20}, 32, 5, PURPLE);

    DrawTextEx(font, "-\t You can move using the \n \n Q,W,E,A,D,Z,X,C keys and attack using \n \n the arrow keys, or mouse" , Vector2{20, 120}, 32, 5, PURPLE);

    DrawTextEx(font, "-\t You can attack the insects as \n \n many times as the difficulty level, \n \n per round, it renews every field." , Vector2{20, 220}, 32, 5, PURPLE);

    DrawTextEx(font, "-\t You can use the portals to \n \n teleport to another location, \n \n but you can only use them as \n \n many times as the \n \n difficulty level+1, per round,\n \n it renews every field\n" , Vector2{20, 320}, 32, 5, PURPLE);

    DrawTextEx(font, "-\t Thank you to George Wisegarver \n \n and Hannah Pugh for Home Screen track" , Vector2{20, 520}, 28, 5, PURPLE);

    DrawTextEx(font, "-\t Thank you to Oliver Mahan \n \n for discussing algorithms with me." , Vector2{20, 620}, 28, 5, PURPLE);

    DrawTextEx(font, "-\t Press Esc to go to the main menu" , Vector2{20, 720}, 28, 5, PURPLE);

    if (IsKeyPressed(KEY_ESCAPE))
    {
        homeScreen = true;
    }
}

void HomeScreen(Player &player, Enemy &enemy, Coin &coin, Texture2D &boarder, Font &font)
{

        // UnloadTexture(player.sprite);
        if (IsSoundPlaying(lvlMusic))
        {
            StopSound(lvlMusic);
        }
        // Update
        if (!IsSoundPlaying(GameTheme))
        {
            PlaySound(GameTheme);
            
        }
        
        mousePoint = GetMousePosition();  // Update the mouse position
       
        if (increasing)
        {
            rgbval += 2.0f;
            if (rgbval >= 225.0f)
                increasing = false;
        }
        else
        {
            rgbval -= 2.0f;
            if (rgbval <= 50.0f)
                increasing = true;
        }


        
        // DrawTextureEx(background, Vector2{0, 0}, 0.0f, 25, WHITE);
        // DrawTextureEx(coin.sprite, Vector2{0,0}, 0.0f, 25, WHITE);
        DrawTextEx(font, "HARVEST", Vector2{20, 25}, 160, 20, Color{0, 255, 0, (unsigned char)rgbval});
        DrawTextEx(font, "HARVEST", Vector2{20, 20}, 160, 20, textColor);

        if ((int)GetTime() %5 == 0)
        {

        
        int numFrames = 6;
        int frameWidth = enemy.homeScreenSprite.width / numFrames;
        
        // Calculate the current frame based on time
        float frameTime = 0.15f;  // Adjust this value to control the animation speed
        int currentFrame = static_cast<int>(GetTime() / frameTime) % numFrames;

        Rectangle source{
            float(currentFrame * frameWidth),
            0,
            float(frameWidth),
            float(enemy.homeScreenSprite.height)
        };

        Rectangle dest{
            screenWidth-175,
            screenHeight-130,
            gameDifficulty*4,
            gameDifficulty*4
        };

        DrawTexturePro(enemy.homeScreenSprite, source, dest, Vector2{0, 0}, 0, WHITE);
        }
        else 
        {
        DrawTextureEx(enemy.sprite, Vector2{screenWidth-175, screenHeight-130}, 0.0f, 4, WHITE);
        }
        DrawTextureEx(player.sprite, Vector2{28, screenHeight-130}, 0.0f, 4, WHITE);
        DrawTextureEx(coin.sprite, Vector2{screenWidth/2-75, screenHeight-130}, 0.0f, 4, WHITE);
        
        DrawTextureEx(coin.sprite, Vector2{screenWidth/2-225, screenHeight-130}, 0.0f, 4, WHITE);
        DrawTextureEx(coin.sprite, Vector2{screenWidth/2+75, screenHeight-130}, 0.0f, 4, WHITE);
        if (mousePoint.x > 200 && mousePoint.x < 568 && mousePoint.y > 200 && mousePoint.y < 328)
        {
            DrawRectangle(200, 200, 92*4, 32*4, Color{56, 93, 56, (unsigned char)rgbval});
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                
                StopSound(GameTheme);
                gameOver = false;
                loadGame = true;
            }

        }
        if (mousePoint.x > 200 && mousePoint.x < 568 && mousePoint.y > 320 && mousePoint.y < 428)
        {
            DrawRectangle(200, 300, 92*4, 32*4, Color{56, 93, 56, (unsigned char)rgbval});
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                
                
                homeScreen = false;
            }

        }
        if (mousePoint.x > screenWidth/2-58 && mousePoint.x < screenWidth/2+6 && mousePoint.y > 530 && mousePoint.y < 594)
        {
            DrawRectangle(screenWidth/2-58, 530, 64, 64, Color{255,0,0,(unsigned char)rgbval});
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                gameChallenge = 2;
            }

        }
        if (mousePoint.x > screenWidth/2-160 && mousePoint.x < screenWidth/2-96 && mousePoint.y > 530 && mousePoint.y < 594)
        {
            DrawRectangle(screenWidth/2-160, 530, 64, 64, Color{255,0,0,(unsigned char)rgbval});
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                gameChallenge = 1;
            }

        }
        if (mousePoint.x > screenWidth/2+45 && mousePoint.x < screenWidth/2+109 && mousePoint.y > 530 && mousePoint.y < 594)
        {
            DrawRectangle(screenWidth/2+45, 530, 64, 64, Color{255,0,0,(unsigned char)rgbval});
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                gameChallenge = 3;
            }

        }
        switch (gameChallenge)
        {
        case 1:
            DrawRectangle(screenWidth/2-160, 530, 64, 64, Color{56, 93, 56, (unsigned char)rgbval});
            break;
        case 2:
            DrawRectangle(screenWidth/2-58, 530, 64, 64, Color{56, 93, 56, (unsigned char)rgbval});
            break;
        case 3:
            DrawRectangle(screenWidth/2+45, 530, 64, 64, Color{56, 93, 56, (unsigned char)rgbval});
            break;
        default:
            break;
        }
    // minus y by 70
        DrawTextureEx(boarder, Vector2{200, 200}, 0.0f, 4, WHITE);
        DrawTextEx(font, "PLAY", Vector2{315, 233}, 50, 10, Color{0, 255, 0, (unsigned char)rgbval});
        DrawTextEx(font, "PLAY", Vector2{315, 230}, 50, 10, textColor);
        DrawTextureEx(boarder, Vector2{200, 300}, 0.0f, 4, WHITE);
        DrawTextEx(font, "INFORMATION", Vector2{213, 353}, 42, 8, Color{0, 255, 0, (unsigned char)rgbval});
        DrawTextEx(font, "INFORMATION", Vector2{213, 350}, 42, 8, textColor);
        DrawTextureEx(boarder, Vector2{200, 400}, 0.0f, 4, WHITE);
        DrawTextEx(font, "DIFFICULTY", Vector2{232, 453}, 42, 8, Color{0, 255, 0, (unsigned char)rgbval});
        DrawTextEx(font, "DIFFICULTY", Vector2{232, 450}, 42, 8, textColor);
        DrawTextureEx(numBoard1, Vector2{screenWidth/2-58, 530}, 0.0f, 2, WHITE);
        DrawTextureEx(numBoard2, Vector2{screenWidth/2-160, 530}, 0.0f, 2, WHITE);
        DrawTextureEx(numBoard3, Vector2{screenWidth/2+45, 530}, 0.0f, 2, WHITE);
        
        DrawTextEx(font, "1", Vector2{screenWidth/2-132, 542}, 42, 10, textColor);
        DrawTextEx(font, "2", Vector2{screenWidth/2-38, 542}, 42, 10, textColor);
        DrawTextEx(font, "3", Vector2{screenWidth/2+65, 542}, 42, 10, textColor);

        DrawTextEx(font, TextFormat("High Score: %i", highScore), Vector2{screenWidth/2-132, 615}, 26, 4, PURPLE);


}

void LoadGame(Player &player, vector<Enemy> *enemies, Weapon &weapon, MazeCreation &maze,  char (&gameMap)[mapWidth][mapHeight], vector<Coin> &coins, vector<Portal> &portals)
{
    enemies->clear();
    portals.clear();
    
    player.position = {0,0};
    player.playerPoint = {static_cast<int>((player.position.x)/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
    player.representation = {player.position.x, player.position.y, gameDifficulty, gameDifficulty};
    player.stepCount = 1;
    player.fruitCollected = 0;
    player.canAttack = true;
    player.canAttackCount = 0;
    player.canAttackString = "Yes";
    player.portalUse = 0;
    coinCount = 0;
    
    //main enemy -----------------------------------------------------------------------------------------------------------
    PopulateEnemies(enemies);
    hasPopulated = false;
    inGame = true;

    GenerateMaze(maze,gameMap,player.playerPoint,enemies,0,0);
    PopulateCoins(coins,gameMap,gameDifficulty);
    PopulatePortals(portals,gameMap,gameDifficulty);
}

void PlayerAttack(Player &player, vector<Enemy> *enemies, Weapon &weapon)
{
    mousePoint = GetMousePosition();
    mousePoint =  GetScreenToWorld2D(mousePoint, camera);
    mousePoint.x = static_cast<int>(mousePoint.x/gameDifficulty)*gameDifficulty;
    mousePoint.y = static_cast<int>(mousePoint.y/gameDifficulty)*gameDifficulty;
        


    if (mousePoint.x == player.position.x+32 &&  mousePoint.y >= player.position.y && mousePoint.y < player.position.y+32)
    {
        DrawRectangle(player.position.x+32, player.position.y, 32, 32, Color{255,0,0,100});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            weapon.wepReresentation = {player.position.x+gameDifficulty,player.position.y,gameDifficulty,gameDifficulty};
            player.attackRight = true;
            player.stationary = false;
            for (Enemy &enemy: *enemies)
            {
                if ((CheckCollisionRecs(weapon.wepReresentation,enemy.enemyRepresentation)) && player.canAttack)
                {
                    cout << "collision" << endl;
                    PlaySound(weapon.attackSound);
                    enemy.stunned = true;
                    player.canAttackCount += 1;
                    if (player.canAttackCount == gameChallenge)
                    {
                        player.canAttack = false;
                        player.canAttackString = "No";
                    }
                    enemy.stunCount = 0;
                }
            }
        }
    }
    if (mousePoint.x == player.position.x-32 &&  mousePoint.y >= player.position.y && mousePoint.y < player.position.y+32)
    {
        DrawRectangle(player.position.x-32, player.position.y, 32, 32, Color{255,0,0,100});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            weapon.wepReresentation = {player.position.x-gameDifficulty,player.position.y,gameDifficulty,gameDifficulty};
            player.attackLeft = true;
            player.stationary = false;
            for (Enemy &enemy: *enemies)
            {
                if ((CheckCollisionRecs(weapon.wepReresentation,enemy.enemyRepresentation)) && player.canAttack)
                {
                    cout << "collision" << endl;
                    PlaySound(weapon.attackSound);
                    enemy.stunned = true;
                    player.canAttackCount += 1;
                    if (player.canAttackCount == gameChallenge)
                    {
                        player.canAttack = false;
                        player.canAttackString = "No";
                    }
                        enemy.stunCount = 0;
                }
            }
        }    }
    if (mousePoint.y == player.position.y+32 &&  mousePoint.x >= player.position.x && mousePoint.x < player.position.x+32)
    {
        DrawRectangle(player.position.x, player.position.y+32, 32, 32, Color{255,0,0,100});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            weapon.wepReresentation = {player.position.x,player.position.y+gameDifficulty,gameDifficulty,gameDifficulty};
            player.attackDown = true;
            player.stationary = false;
            for (Enemy &enemy: *enemies)
            {
                if ((CheckCollisionRecs(weapon.wepReresentation,enemy.enemyRepresentation)) && player.canAttack)
                {
                    cout << "collision" << endl;
                    PlaySound(weapon.attackSound);
                    enemy.stunned = true;
                    player.canAttackCount += 1;
                    if (player.canAttackCount == gameChallenge)
                    {
                        player.canAttack = false;
                        player.canAttackString = "No";
                    }
                        enemy.stunCount = 0;
                }
            }
        }    }
    if (mousePoint.y == player.position.y-32 &&  mousePoint.x >= player.position.x && mousePoint.x < player.position.x+32)
    {
        DrawRectangle(player.position.x, player.position.y-32, 32, 32, Color{255,0,0,100});
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            weapon.wepReresentation = {player.position.x,player.position.y-gameDifficulty,gameDifficulty,gameDifficulty};
            player.attackUp = true;
            player.stationary = false;
            for (Enemy &enemy: *enemies)
            {
                if ((CheckCollisionRecs(weapon.wepReresentation,enemy.enemyRepresentation)) && player.canAttack)
                {
                    cout << "collision" << endl;
                    PlaySound(weapon.attackSound);
                    enemy.stunned = true;
                    player.canAttackCount += 1;
                    if (player.canAttackCount == gameChallenge)
                    {
                        player.canAttack = false;
                        player.canAttackString = "No";
                    }
                        enemy.stunCount = 0;
                }
            }
        }    }


    if (IsKeyPressed(KEY_RIGHT))
    {
        weapon.wepReresentation = {player.position.x+gameDifficulty,player.position.y,gameDifficulty,gameDifficulty};
        player.attackRight = true;
        player.stationary = false;
        for (Enemy &enemy: *enemies)
        {
            if ((CheckCollisionRecs(weapon.wepReresentation,enemy.enemyRepresentation)) && player.canAttack)
            {
                cout << "collision" << endl;
                PlaySound(weapon.attackSound);
                enemy.stunned = true;
                player.canAttackCount += 1;
                if (player.canAttackCount == gameChallenge)
                {
                    player.canAttack = false;
                    player.canAttackString = "No";
                }
                enemy.stunCount = 0;
            }
        }

    }
    else if (IsKeyPressed(KEY_LEFT))
    {
        
        weapon.wepReresentation = {player.position.x-gameDifficulty,player.position.y,gameDifficulty,gameDifficulty};
        player.attackLeft = true;
        player.stationary = false;
        for(Enemy &enemy: *enemies)
        {
            if ((CheckCollisionRecs(weapon.wepReresentation,enemy.enemyRepresentation)) && player.canAttack)
            {
                cout << "collision" << endl;
                PlaySound(weapon.attackSound);
                enemy.stunned = true;
                player.canAttackCount += 1;
                if (player.canAttackCount == gameChallenge)
                {
                    player.canAttack = false;
                    player.canAttackString = "No";
                }
                enemy.stunCount = 0;
            }
        }

    }
    else if (IsKeyPressed(KEY_UP))
    {
        weapon.wepReresentation = {player.position.x,player.position.y-gameDifficulty,gameDifficulty,gameDifficulty};
        player.attackUp = true;
        player.stationary = false;
        for (Enemy &enemy: *enemies)
        {
            if ((CheckCollisionRecs(weapon.wepReresentation,enemy.enemyRepresentation)) && player.canAttack)
            {
                cout << "collision" << endl;
                PlaySound(weapon.attackSound);
                enemy.stunned = true;
                player.canAttackCount += 1;
                if (player.canAttackCount == gameChallenge)
                {
                    player.canAttack = false;
                    player.canAttackString = "No";
                }


                cout << "is enemy stunned " << enemy.stunned << endl;
                enemy.stunCount = 0;
            }
        }

    }
    else if (IsKeyPressed(KEY_DOWN))
    {
        weapon.wepReresentation = {player.position.x,player.position.y+gameDifficulty,gameDifficulty,gameDifficulty};
        player.attackDown = true;
        player.stationary = false;
        for (Enemy &enemy: *enemies)
        {
            if ((CheckCollisionRecs(weapon.wepReresentation,enemy.enemyRepresentation)) && player.canAttack)
            {
                cout << "collision" << endl;
                PlaySound(weapon.attackSound);
                enemy.stunned = true;
                player.canAttackCount += 1;
                if (player.canAttackCount == gameChallenge)
                {
                    player.canAttack = false;
                    player.canAttackString = "No";
                }
                enemy.stunCount = 0;
            }
        }

    }

}

void DrawPlayer(Player &player)
{
    if (player.stationary)
    {
        player.attackUp = false;
        player.attackDown = false;
        player.attackLeft = false;
        player.attackRight = false;
        DrawTextureEx(player.sprite, player.position, 0, 1, WHITE);
    }
    else if (player.attackRight)
    {
        DrawTextureEx(player.attackRightSprite, player.position, 0, 1, WHITE);
    }
    else if (player.attackLeft)
    {
        DrawTextureEx(player.attackLeftSprite, Vector2{player.position.x-32,player.position.y}, 0, 1, WHITE);
    }
    else if (player.attackUp)
    {
        DrawTextureEx(player.attackUpSprite, Vector2{player.position.x,player.position.y-32}, 0, 1, WHITE);
    }
    else if (player.attackDown)
    {
        DrawTextureEx(player.attackDownSprite, player.position, 0, 1, WHITE);
    }
    else
    {
        DrawTextureEx(player.sprite, player.position, 0, 1, WHITE);
    }

}

void CheckPortalCollision(Player &player, vector<Portal> &portals, char (&gameMap)[mapWidth][mapHeight])
{
    for (int i = 0; i < portals.size(); i++)
    {
        if (CheckCollisionRecs(player.representation, portals[i].portalRepresentation) && portals[i].isActivated == false && !player.justTeleported)
        {
            cout << "collision" << endl;
            player.stepCount += 2;
            player.portalUse += 1;
            // cout << "player portal use is " << player.portalUse << endl;
            if (player.portalUse == gameChallenge +1)
            {
                for (Portal &portal: portals)
                {
                    portal.isActivated = true;
                    portal.isActivatedString = "Yes";
                }
            }


            PlaySound(portals[i].portalSound);
            //move player to random portal that is not the current portal
            int randomPortal = GetRandomValue(0, portals.size()-1);
            while (randomPortal == i)
            {
                randomPortal = GetRandomValue(0, portals.size()-1);
            }
            player.position = {portals[randomPortal].portalRepresentation.x, portals[randomPortal].portalRepresentation.y};
            player.playerPoint = {static_cast<int>(player.position.x/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
            player.representation = {player.position.x, player.position.y, gameDifficulty, gameDifficulty};
            player.justTeleported = true;
            
        }
    }
}

void DrawPortals(vector<Portal> &portals)
{
    for (int i = 0; i < portals.size(); i++)
    {
        // Assuming the portal sprite has 4 frames for animation
        int numFrames = 4;
        int frameWidth = portals[i].sprite.width / numFrames;

        // Calculate the current frame based on time
        float frameTime = 0.15f;  // Adjust this value to control the animation speed
        int currentFrame = static_cast<int>(GetTime() / frameTime) % numFrames;

        Rectangle source{
            float(currentFrame * frameWidth),
            0,
            float(frameWidth),
            float(portals[i].sprite.height)
        };

        Rectangle dest{
            portals[i].portalRepresentation.x,
            portals[i].portalRepresentation.y,
            gameDifficulty,
            gameDifficulty
        };

        DrawTexturePro(portals[i].sprite, source, dest, Vector2{0, 0}, 0, WHITE);
    }
}

vector<Point> get_neighbors(Point &p, char (&gameMap)[mapWidth][mapHeight]) {
    vector<Point> ret;

    if (p.y + 1 < mapHeight && gameMap[p.x][p.y+1] == 1) ret.push_back(Point{p.x, p.y + 1});
    if (p.y - 1 >= 0 && gameMap[p.x][p.y - 1] == 1) ret.push_back(Point{p.x, p.y - 1});
    if (p.x + 1 < mapWidth && gameMap[p.x+1][p.y] == 1) ret.push_back(Point{p.x + 1, p.y});
    if (p.x - 1 >= 0 && gameMap[p.x - 1][p.y] == 1) ret.push_back(Point{p.x - 1, p.y});
    //add diagonal checks
    if (p.x + 1 < mapWidth && p.y + 1 < mapHeight && gameMap[p.x+1][p.y+1] == 1) ret.push_back(Point{p.x + 1, p.y + 1});
    if (p.x - 1 >= 0 && p.y - 1 >= 0 && gameMap[p.x-1][p.y-1] == 1) ret.push_back(Point{p.x - 1, p.y - 1});
    if (p.x + 1 < mapWidth && p.y - 1 >= 0 && gameMap[p.x+1][p.y-1] == 1) ret.push_back(Point{p.x + 1, p.y - 1});
    if (p.x - 1 >= 0 && p.y + 1 < mapHeight && gameMap[p.x-1][p.y+1] == 1) ret.push_back(Point{p.x - 1, p.y + 1});

    return ret;
}



void CheckCoinCollision(Player &player, vector<Coin> &coins, MazeCreation &maze, char (&gameMap)[mapWidth][mapHeight], vector<Enemy> *enemies, vector<Portal> &portals)
{
 for (int i = 0; i < coins.size(); i++)
            {
                if (CheckCollisionRecs(player.representation,coins[i].representCoin) && coins[i].isCollected == false)
                {
                    cout << "collision" << endl;
                    
                    
                    coins[i].isCollected = true;
                    coinCount+=1;
                    player.fruitCollected = (coinCount*gameChallenge);
                    if (coinCount % 4 != 0)
                    {
                        PlaySound(coins[i].collectFruit);
                    }
                    if (coinCount % 4 == 0)
                    {
                        // cout << "player point is " << player.playerPoint.x << " " << player.playerPoint.y << endl;
                        // cout << "enemy point is " << enemy.enemyPoint.x << " " << enemy.enemyPoint.y << endl;
                        GenerateMaze(maze,gameMap,player.playerPoint,enemies,0,0);
                        PopulateCoins(coins,gameMap,gameDifficulty);
                        PopulatePortals(portals,gameMap,gameDifficulty);
                        player.canAttackCount = 0;
                        player.canAttack = true;
                        player.canAttackString = "Yes";
                        player.stepCount = 1;
                        player.portalUse = 0;
                        if (enemies->size() > gameChallenge)
                        {
                            for (int i = enemies->size(); i > gameChallenge ; i--)
                            {
                            enemies->pop_back();
                            }
                        }
                        hasPopulated = false;
                        
                    }
                    
                }
                if (coins[i].isCollected == false)
                {
                   DrawTextureEx(coins[i].sprite, Vector2{coins[i].representCoin.x,coins[i].representCoin.y}, 0, 1, WHITE);
                }
            }
}

void GenerateMaze(MazeCreation &maze,char (&gameMap)[mapWidth][mapHeight], Point &playerPoint, vector<Enemy> *enemies, int i, int j)
{
        for (int i = 0; i < mapHeight; i++) 
        {
            for (int j = 0; j < mapWidth; j++)
             {
              gameMap[i][j] = 0;        
        }
    }

    maze.MazeGeneration(gameMap, 0, 0);
    gameMap[playerPoint.x][playerPoint.y] = 1;
    for (int i = 0; i < enemies->size(); i++)
    {
        gameMap[enemies->at(i).enemyPoint.x][enemies->at(i).enemyPoint.y] = 1;
    }
    gameMap[0][mapHeight-1] = 1;
    gameMap[mapWidth-1][0] = 1;
    
    cout << "maze generated" << endl;
    for (int i = 0; i < mapHeight; i++) 
    {
        for (int j = 0; j < mapWidth; j++)
        {
            cout << static_cast<int>(gameMap[j][i]) << " ";
        }
        cout << endl;
    }
}


void printPath(const Point &current) {
    if (!current.parent.empty()) {
        Point parent = current.parent.front();
        printPath(parent);
       
    }
}

void PopulatePortals(vector<Portal> &portals, char (&gameMap)[mapWidth][mapHeight], int gameDifficulty)
{
    portals.clear();
    Portal portal;
    for (int i = 0; i < gameChallenge+1; i++)
    {
        int ranX, ranY;
        ranX = rand() % mapWidth;
        ranY = rand() % mapHeight;
        if (gameMap[ranX][ranY] == 0)
        {
            i--;
            continue;
        }
        portal.portalRepresentation = {static_cast<float>(ranX*gameDifficulty), static_cast<float>(ranY*gameDifficulty), (float) gameDifficulty, (float)gameDifficulty};
        portal.isActivated = false;
        portal.sprite = LoadTexture("Resource/portal.png");
        portal.portalSound = LoadSound("Resource/Portal.wav");
        portals.push_back(portal);
    }

}

void PopulateCoins(vector<Coin> &coins, char (&gameMap)[mapWidth][mapHeight], int gameDifficulty)
{

    for (int i = 0; i < coins.size(); i++)
    {
        int ranX, ranY;
        ranX = rand() % mapWidth;
        ranY = rand() % mapHeight;
        if (gameMap[ranX][ranY] == 0)
        {
            i--;
            continue;
        }
        coins[i].representCoin = {static_cast<float>(ranX*gameDifficulty), static_cast<float>(ranY*gameDifficulty), (float) gameDifficulty, (float)gameDifficulty};
        coins[i].isCollected = false;
    }
    PlaySound(coins[0].lvlChange);
}


std::queue<Point> getPath(const Point &current) {
    std::queue<Point> pathQueue;

    // Base case: If the current node has a parent
    if (!current.parent.empty())
    {
        Point parent = current.parent.front();
        pathQueue = getPath(parent); // Recurrrrrrsioooon
    }

    // Add the current node to the path queue
    pathQueue.push(current);
    printPath(current);
    return pathQueue;
}



std::queue<Point> bfs(Point &start, Point &destination, char (&gameMap)[mapWidth][mapHeight], Enemy &enemy) {
    std::queue<Point> emptyQueue; // Return an empty queue if destination is not reachable
    std::queue<Point> to_visit;
    std::set<Point> visited;
    

    to_visit.push(start);

    while (!to_visit.empty()) {
        Point current = to_visit.front();
        to_visit.pop();

        if (current.x == destination.x && current.y == destination.y) {
            enemy.desinationReached = true;
            cout << " enemy destination is " << enemy.position.x << " " << enemy.position.y << endl;

            // Get the path by traversing parent pointers
            std::queue<Point> pathQueue = getPath(current);

            // Reverse the order of the path
            std::stack<Point> tempStack;
            while (!pathQueue.empty()) {
                tempStack.push(pathQueue.front());
                pathQueue.pop();
            }

            while (!tempStack.empty()) {
                pathQueue.push(tempStack.top());
                tempStack.pop();
            }

            return pathQueue; // Return the path queue
        }

        visited.insert(current);
        std::vector<Point> neighbors = get_neighbors(current, gameMap);

        // Loop through neighbors
        for (Point &neighbor : neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                neighbor.parent.push(current);
                to_visit.push(neighbor);
                visited.insert(neighbor);
            }
        }
    }

    // Return an empty queue if destination is not reachable
    return emptyQueue;
}




void DrawMap(char (&gameMap)[mapWidth][mapHeight], Texture2D &wall_mabe,Texture2D& tree, Texture2D& grass, int gameDifficulty)
{
    for (int row = -1; row <= mapHeight; row++)
    {
        for (int col = -1; col <= mapWidth; col++)
        {
            if (row == -1 || row == mapHeight  || col == -1 || col == mapWidth )
            {
                DrawTextureEx(wall_mabe, Vector2{(float)col * gameDifficulty, (float)row * gameDifficulty}, 0, 2, WHITE);
            }
            else
            {
                if (gameMap[col][row] == 0)
                {
                    // std::string text = std::to_string(row) + "" + std::to_string(col);
                    // DrawRectangle(col * gameDifficulty, row * gameDifficulty, gameDifficulty, gameDifficulty, PURPLE);
                    DrawTextureEx(tree, Vector2{(float)col * gameDifficulty, (float)row * gameDifficulty}, 0, 1, WHITE);
                    // DrawText(text.c_str(), col * gameDifficulty, row * gameDifficulty, 20, WHITE);
                }
                else
                {
                    // std::string text = std::to_string(row) + "" + std::to_string(col);
                    DrawTextureEx(grass, Vector2{(float)col * gameDifficulty, (float)row * gameDifficulty}, 0, 1, WHITE);
                    // DrawText(text.c_str(), col * gameDifficulty, row * gameDifficulty, 20, WHITE);
                }
            }
        }
    }
}


void EnemyMovement(vector<Enemy> &enemies, Player &player)
{
    for (Enemy &enemy : enemies)
    {
        if (enemy.enemyTurn && !enemy.pathStack.empty() && !enemy.stunned)
        {
            Point nextPosition = enemy.pathStack.front();
            enemy.pathStack.pop();

            // Check if the next position is occupied by another enemy
            bool isOccupied = false;
            for (const Enemy &otherEnemy : enemies)
            {
                if (&otherEnemy != &enemy && otherEnemy.enemyPoint.x == nextPosition.x && otherEnemy.enemyPoint.y == nextPosition.y && !otherEnemy.stunned)

                {
                    isOccupied = true;
                    break;
                }
            }

            // Update the enemy's position if the next position is not occupied
            if (!isOccupied)
            {
                enemy.position.x = nextPosition.x * gameDifficulty;
                enemy.position.y = nextPosition.y * gameDifficulty;
                enemy.enemyRepresentation = {enemy.position.x, enemy.position.y, gameDifficulty, gameDifficulty};
                enemy.enemyPoint = {static_cast<int>(enemy.position.x / gameDifficulty), static_cast<int>(enemy.position.y / gameDifficulty)};

                // Check if there's still a second movement available
                if (!enemy.pathStack.empty())
                {
                    Point secondPosition = enemy.pathStack.front();
                    enemy.pathStack.pop();

                    // Check if the second position is occupied by another enemy
                    bool isSecondOccupied = false;
                    for (const Enemy &otherEnemy : enemies)
                    {
                        if (&otherEnemy != &enemy && otherEnemy.enemyPoint.x == secondPosition.x && otherEnemy.enemyPoint.y == secondPosition.y && !otherEnemy.stunned)
                        {
                            isSecondOccupied = true;
                            break;
                        }
                    }

                    // Update the enemy's position for the second movement if not occupied
                    if (!isSecondOccupied)
                    {
                        enemy.position.x = secondPosition.x * gameDifficulty;
                        enemy.position.y = secondPosition.y * gameDifficulty;
                        enemy.enemyRepresentation = {enemy.position.x, enemy.position.y, gameDifficulty, gameDifficulty};
                        enemy.enemyPoint = {static_cast<int>(enemy.position.x / gameDifficulty), static_cast<int>(enemy.position.y / gameDifficulty)};
                        
                        if (enemy.position.x == player.position.x && enemy.position.y == player.position.y)
                        {
                            HighScore(player);
                            GetHighScore();
                            gameOver = true;
                            
                        }
                    }
                }

                // Set turns for player and enemy
                playerTurn = true;
                enemy.enemyTurn = false;
            }
        }
        else if (enemy.enemyTurn && !enemy.pathStack.empty() && enemy.stunned)
        {
            playerTurn = true;
            enemy.enemyTurn = false;
        }
    }
}


void PlayerMovement(Player &player, char (&gameMap)[mapWidth][mapHeight], int gameDifficulty, vector<Enemy> &enemies)
{
    if (IsKeyPressed(KEY_KP_2) || IsKeyPressed(KEY_X))
    {
        hasPopulated = false;
        player.stationary = true;
        player.stepCount++;
        for (Enemy &enemy : enemies)
        {
            if (enemy.stunned)
            {
                enemy.stunCount++;
                if (enemy.stunCount == STUNCOUNT)
                {
                    enemy.stunned = false;
                    enemy.stunCount = 0;
                }
            }
        }

        int targetCol = static_cast<int>((player.position.x) / gameDifficulty);
        int targetRow = static_cast<int>((player.position.y + gameDifficulty) / gameDifficulty);
        

        if (targetRow >= 0 && targetRow < mapHeight && targetCol >= 0 && targetCol < mapWidth && gameMap[targetCol][targetRow] != 0)
        {
            player.position.y += gameDifficulty;
            player.playerPoint = {static_cast<int>((player.position.x)/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
            player.representation = {player.position.x, player.position.y,(float)gameDifficulty,(float)gameDifficulty};
            player.justTeleported = false;
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            
                if ((player.position.x ==  enemy.position.x && player.position.y == enemy.position.y) && enemy.stunned == false)
                {
                    HighScore(player);
                    GetHighScore();
                    gameOver = true;
                }
            }
            playerTurn = false;

        }
        else
        {
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            }

            playerTurn = false;
        }
    }
    else if (IsKeyPressed(KEY_KP_4) || IsKeyPressed(KEY_A))
    {
        player.stationary = true;
        player.stepCount++;
        hasPopulated = false;
        for (Enemy &enemy : enemies)
        {
            if (enemy.stunned)
            {
                enemy.stunCount++;
                if (enemy.stunCount == STUNCOUNT)
                {
                    enemy.stunned = false;
                    enemy.stunCount = 0;
                }
            }
        }
        int targetCol = static_cast<int>((player.position.x - gameDifficulty) / gameDifficulty);
        int targetRow = static_cast<int>((player.position.y) / gameDifficulty);

        if (targetRow >= 0 && targetRow < mapHeight && targetCol >= 0 && targetCol < mapWidth && gameMap[targetCol][targetRow] != 0)
        {
            player.position.x -= gameDifficulty;
            player.playerPoint = {static_cast<int>((player.position.x)/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
            player.representation = {player.position.x, player.position.y,(float)gameDifficulty,(float)gameDifficulty};
            player.justTeleported = false;
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            
                if ((player.position.x ==  enemy.position.x && player.position.y == enemy.position.y) && enemy.stunned == false)
                {
                    HighScore(player);
                    GetHighScore();
                    gameOver = true;
                }
            }
            playerTurn = false;
        }
        else
        {
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            }

            playerTurn = false;
        }
    }
    else if (IsKeyPressed(KEY_KP_6) || IsKeyPressed(KEY_D))
    {
        player.stationary = true;
        player.stepCount++;
        hasPopulated = false;
        for (Enemy &enemy : enemies)
        {
            if (enemy.stunned)
            {
                enemy.stunCount++;
                if (enemy.stunCount == STUNCOUNT)
                {
                    enemy.stunned = false;
                    enemy.stunCount = 0;
                }
            }
        }
        int targetCol = static_cast<int>((player.position.x + gameDifficulty) / gameDifficulty);
        int targetRow = static_cast<int>((player.position.y) / gameDifficulty);

        if (targetRow >= 0 && targetRow < mapHeight && targetCol >= 0 && targetCol < mapWidth && gameMap[targetCol][targetRow] != 0)
        {
            player.position.x += gameDifficulty;
            player.playerPoint = {static_cast<int>((player.position.x)/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
            player.representation = {player.position.x, player.position.y,(float)gameDifficulty,(float)gameDifficulty};
            player.justTeleported = false;
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            
                if (((player.position.x ==  enemy.position.x && player.position.y == enemy.position.y) && enemy.stunned == false))
                {
                    HighScore(player);
                    GetHighScore();
                    gameOver = true;
                }
            }
            playerTurn = false;
        }
        else
        {
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            }

            playerTurn = false;
        }
    }
    else if (IsKeyPressed(KEY_KP_8) || IsKeyPressed(KEY_W))
    {
        player.stationary = true;
        player.stepCount++;
        hasPopulated = false;
        for (Enemy &enemy : enemies)
        {
            if (enemy.stunned)
            {
                enemy.stunCount++;
                if (enemy.stunCount == STUNCOUNT)
                {
                    enemy.stunned = false;
                    enemy.stunCount = 0;
                }
            }
        }
        int targetRow = static_cast<int>((player.position.y - gameDifficulty) / gameDifficulty);
        int targetCol = static_cast<int>((player.position.x) / gameDifficulty);
        player.stepCount++;

        if (targetRow >= 0 && targetRow < mapHeight && targetCol >= 0 && targetCol < mapWidth && gameMap[targetCol][targetRow] != 0)
        {
            player.position.y -= gameDifficulty;
            player.playerPoint = {static_cast<int>((player.position.x)/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
            player.representation = {player.position.x, player.position.y,(float)gameDifficulty,(float)gameDifficulty};
            player.justTeleported = false;
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            
                if ((player.position.x ==  enemy.position.x && player.position.y == enemy.position.y) && enemy.stunned == false)
                {
                    HighScore(player);
                    void GetHighScore();
                    gameOver = true;
                }
            }
            playerTurn = false;
        }
        else
        {
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            }

            playerTurn = false;
        }
    }
    else if (IsKeyPressed(KEY_KP_1) || IsKeyPressed(KEY_Z))
    {
        // Move down-left
        player.stationary = true;
        player.stepCount++;
        hasPopulated = false;
        for (Enemy &enemy : enemies)
        {
            if (enemy.stunned)
            {
                enemy.stunCount++;
                if (enemy.stunCount == STUNCOUNT)
                {
                    enemy.stunned = false;
                    enemy.stunCount = 0;
                }
            }
        }
        int targetCol = static_cast<int>((player.position.x - gameDifficulty) / gameDifficulty);
        int targetRow = static_cast<int>((player.position.y + gameDifficulty) / gameDifficulty);

        if (targetRow >= 0 && targetRow < mapHeight && targetCol >= 0 && targetCol < mapWidth && gameMap[targetCol][targetRow] != 0)
        {
            player.position.x -= gameDifficulty;
            player.position.y += gameDifficulty;
            player.playerPoint = {static_cast<int>((player.position.x)/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
            player.representation = {player.position.x, player.position.y,(float)gameDifficulty,(float)gameDifficulty};
            player.justTeleported = false;
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            
                if ((player.position.x ==  enemy.position.x && player.position.y == enemy.position.y) && enemy.stunned == false)
                {
                    HighScore(player);
                    void GetHighScore();
                    gameOver = true;
                }
            }
            playerTurn = false;
        }
        else
        {
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            }

            playerTurn = false;
        }
    }
    else if (IsKeyPressed(KEY_KP_3) || IsKeyPressed(KEY_C))
    {
        player.stationary = true;
        player.stepCount++;
        hasPopulated = false;
        for (Enemy &enemy : enemies)
        {
            if (enemy.stunned)
            {
                enemy.stunCount++;
                if (enemy.stunCount == STUNCOUNT)
                {
                    enemy.stunned = false;
                    enemy.stunCount = 0;
                }
            }
        }
        // Move down-right
        int targetCol = static_cast<int>((player.position.x + gameDifficulty) / gameDifficulty);
        int targetRow = static_cast<int>((player.position.y + gameDifficulty) / gameDifficulty);

        if (targetRow >= 0 && targetRow < mapHeight && targetCol >= 0 && targetCol < mapWidth && gameMap[targetCol][targetRow] != 0)
        {
            player.position.x += gameDifficulty;
            player.position.y += gameDifficulty;
            player.playerPoint = {static_cast<int>((player.position.x)/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
            player.representation = {player.position.x, player.position.y,(float)gameDifficulty,(float)gameDifficulty};
            player.justTeleported = false;
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            
                if ((player.position.x ==  enemy.position.x && player.position.y == enemy.position.y) && enemy.stunned == false)
                {
                    HighScore(player);
                    void GetHighScore();
                    gameOver = true;
                }
            }
            playerTurn = false;
        }
        else
        {
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            }

            playerTurn = false;
        }
    }
    else if (IsKeyPressed(KEY_KP_7) || IsKeyPressed(KEY_Q))
    {
        player.stationary = true;
        player.stepCount++;
        hasPopulated = false;
        for (Enemy &enemy : enemies)
        {
            if (enemy.stunned)
            {
                enemy.stunCount++;
                if (enemy.stunCount == STUNCOUNT)
                {
                    enemy.stunned = false;
                    enemy.stunCount = 0;
                }
            }
        }
        // Move up-left
        int targetCol = static_cast<int>((player.position.x - gameDifficulty) / gameDifficulty);
        int targetRow = static_cast<int>((player.position.y - gameDifficulty) / gameDifficulty);

        if (targetRow >= 0 && targetRow < mapHeight && targetCol >= 0 && targetCol < mapWidth && gameMap[targetCol][targetRow] != 0)
        {
            player.position.x -= gameDifficulty;
            player.position.y -= gameDifficulty;
            player.playerPoint = {static_cast<int>((player.position.x)/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
            player.representation = {player.position.x, player.position.y,(float)gameDifficulty,(float)gameDifficulty};
            player.justTeleported = false;
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            
                if ((player.position.x ==  enemy.position.x && player.position.y == enemy.position.y) && enemy.stunned == false)
                {
                    HighScore(player);
                    void GetHighScore();
                    gameOver = true;
                }
            }
            playerTurn = false;
        }
        else
        {
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            }

            playerTurn = false;
        }
    }
    else if (IsKeyPressed(KEY_KP_9) || IsKeyPressed(KEY_E))
    {
        player.stationary = true;
        player.stepCount++;
        hasPopulated = false;
        for (Enemy &enemy : enemies)
        {
            if (enemy.stunned)
            {
                enemy.stunCount++;
                if (enemy.stunCount == STUNCOUNT)
                {
                    enemy.stunned = false;
                    enemy.stunCount = 0;
                }
            }
        }    
        // Move up-right
        int targetCol = static_cast<int>((player.position.x + gameDifficulty) / gameDifficulty);
        int targetRow = static_cast<int>((player.position.y - gameDifficulty) / gameDifficulty);

        if (targetRow >= 0 && targetRow < mapHeight && targetCol >= 0 && targetCol < mapWidth && gameMap[targetCol][targetRow] != 0)
        {
            player.position.x += gameDifficulty;
            player.position.y -= gameDifficulty;
            player.playerPoint = {static_cast<int>((player.position.x)/gameDifficulty), static_cast<int>(player.position.y/gameDifficulty)};
            player.representation = {player.position.x, player.position.y,(float)gameDifficulty,(float)gameDifficulty};
            player.justTeleported = false;
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            
                if ((player.position.x ==  enemy.position.x && player.position.y == enemy.position.y) && enemy.stunned == false)
                {
                    HighScore(player);
                    void GetHighScore();
                    gameOver = true;
                }
            }
            playerTurn = false;
        }
        else
        {
            for (Enemy &enemy : enemies)
            {
                enemy.desinationReached = false;
                enemy.enemyTurn = true;
            }

            playerTurn = false;
        }
    }

    // Common code for all movement directions
    


}


