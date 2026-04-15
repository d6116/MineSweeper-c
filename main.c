#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"



typedef struct cell
{
    int value;
    bool isFlagged;
    bool isUncovered;
} Cell;

typedef struct visualCell
{
    Cell* assignedCell;
    Rectangle btnRect; // rectangle for the button
    int posX;
    int posY;
    int scale;
} VisualCell;

typedef enum {
    FACE_NATURAL = 0,
    FACE_SCARED = 1,
    FACE_DEAD = 2
} DudeState;

typedef struct dude {
    DudeState state;
    int posX;
    int posY;
} Dude;

// declare functions
Cell **GenerateNewBoard(int, int, int);
int DiscoverEmptyCells(Cell**, int, int, int, int);


int main()
{

    srand(time(NULL));

    int BOARD_WIDTH = 16;
    int BOARD_HIGHT = 10;
    int NUM_OF_BOMBS = 10;

    int input_rows_result;
    printf("Number of rows: (0 to skip config)\n");
    scanf("%i", &input_rows_result);

    if (!input_rows_result == 0){
        BOARD_HIGHT = input_rows_result;
        printf("Number of columns:\n ");
        scanf("%i", &BOARD_WIDTH);
        printf("Number of bombs:\n ");
        scanf("%i", &NUM_OF_BOMBS);
    }



    const int SCREEN_WIDTH = 800;
    const int SCREEN_HIGHT = 450;

    InitWindow(SCREEN_WIDTH, SCREEN_HIGHT, "Mine Sweeper");


    // init things and load textures
    Cell **board = GenerateNewBoard(BOARD_HIGHT, BOARD_WIDTH, NUM_OF_BOMBS);

    for(int r = 0; r < BOARD_HIGHT; r++){
        for (int c = 0; c < BOARD_WIDTH; c++){
            printf("|  %i  |", board[r][c].value);
        };
        printf("\n");
    };


    // numbers textures
    Texture *numbersTextures = malloc(8 * sizeof(Texture));

    for (int i = 0; i < 8; i++){

        char current_file[90];

        snprintf(current_file, sizeof(current_file), "sprites\\uncovered_%i.png", i + 1);

        numbersTextures[i] = LoadTexture(current_file);
    }



    // cell backgrounds
    Texture *cellBackgroundTextures = malloc(2 * sizeof(Texture));

    cellBackgroundTextures[0] = LoadTexture("sprites\\covered_tile.png"); // the covered tile texture
    cellBackgroundTextures[1] = LoadTexture("sprites\\uncovered_tile.png"); // the uncovered tile texture

    // faces
    Texture *facesTextures = malloc(4 * sizeof(Texture));

    facesTextures[0] = LoadTexture("sprites\\face_natural.png"); // normal face
    facesTextures[1] = LoadTexture("sprites\\face_scared.png"); // scared face (when selecting cell)
    facesTextures[2] = LoadTexture("sprites\\face_dead.png"); // self explanitory

    // other symbols
    Texture *otherSymbols = malloc(2 * sizeof(Texture));

    otherSymbols[0] = LoadTexture("sprites\\covered_tile_marked.png"); // marked bomb texture
    otherSymbols[1] = LoadTexture("sprites\\uncovered_bomb.png"); //bomb symbol


   // cells pos anc cell buttons and create rects for clicking the buttons

    const int BOARD_OFFSET_X = 0;
    const int BOARD_OFFSET_Y = 100;

    const int CELL_HIGHT = 32;
    const int CELL_WIDTH = 32;

    int currentCellPosX = 0;
    int currentCellPosY = 0;

    const int BOARD_SCALE = 1;

    int margin = 0;

    VisualCell *visualCells = malloc(sizeof(VisualCell) * BOARD_HIGHT * BOARD_WIDTH);
    //Rectangle *cellsRects = malloc(sizeof(Rectangle) * BOARD_HIGHT * BOARD_WIDTH);


    int cellIndex = 0;
    for (int r = 0; r < BOARD_HIGHT; r++)
    {
        for (int c = 0; c < BOARD_WIDTH; c++)
        {
            VisualCell vc;

            vc.assignedCell = &board[r][c];
            vc.posX = currentCellPosX + BOARD_OFFSET_X;
            vc.posY = currentCellPosY + BOARD_OFFSET_Y;
            vc.scale = BOARD_SCALE;
            vc.btnRect = (Rectangle){
                currentCellPosX + BOARD_OFFSET_X,
                currentCellPosY + BOARD_OFFSET_Y,
                CELL_WIDTH * BOARD_SCALE,
                CELL_HIGHT * BOARD_SCALE
            };

            visualCells[cellIndex] = vc;

            currentCellPosX += (CELL_WIDTH * BOARD_SCALE) + margin;
            cellIndex += 1;
        }
        currentCellPosY += CELL_HIGHT * BOARD_SCALE + margin;
        currentCellPosX = 0;
    }

    // set size to match game
    SetWindowSize((BOARD_WIDTH * CELL_WIDTH * BOARD_SCALE)  + BOARD_OFFSET_X, (BOARD_HIGHT * CELL_HIGHT * BOARD_SCALE)  + BOARD_OFFSET_Y);


    Dude scaredGuy;

    scaredGuy.state = FACE_NATURAL;
    scaredGuy.posX = GetScreenWidth() / 2 - 32;
    scaredGuy.posY = 20;



    // --- START GAME LOOP ---

    SetTargetFPS(60);



    // in-game vars
    bool is_mouse_0_down = false; // left click
    bool is_mouse_1_down = false; // right click

    // Main Loop
    while (!WindowShouldClose())
    {
        // --- START OF UPDATE ---
        if (is_mouse_0_down){

            scaredGuy.state = FACE_SCARED;

            if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){ // on mouse 0 up
                for (int c = 0; c < cellIndex; c++){ // fore each cell check if the mouse is overlapping
                    if (CheckCollisionPointRec(GetMousePosition(), visualCells[c].btnRect)){
                        Cell* selected_cell = visualCells[c].assignedCell;
                        if (!selected_cell->isFlagged) // uncover cell
                        {
                            if (selected_cell->value == 0)
                                DiscoverEmptyCells(board, c / BOARD_WIDTH, c % BOARD_WIDTH, BOARD_HIGHT, BOARD_WIDTH);
                            visualCells[c].assignedCell->isUncovered = true;
                        }

                        break;
                    }
                }
            }

        }
        else{
            scaredGuy.state = FACE_NATURAL;
        }

        if (is_mouse_1_down){

            if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){ // on mouse 1 up
                for (int c = 0; c < cellIndex; c++){ // fore each cell check if the mouse is overlapping
                    if (CheckCollisionPointRec(GetMousePosition(), visualCells[c].btnRect)){
                        visualCells[c].assignedCell->isFlagged = !visualCells[c].assignedCell->isFlagged;
                        break;
                    }
                }
            }

        }





        is_mouse_0_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        is_mouse_1_down = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

        // --- END OF UPDATE ---


        // --- START OF DRAWING ---
        BeginDrawing();
            ClearBackground(RAYWHITE);





            // --- Draw Board ---
            for (int c = 0; c < cellIndex; c++)
            {
                VisualCell current_cell = visualCells[c];
                Vector2 pos = (Vector2){current_cell.posX, current_cell.posY};
                    if (current_cell.assignedCell->isUncovered){
                        DrawTextureEx(cellBackgroundTextures[1], pos, 0, current_cell.scale, WHITE);
                        if (9 > current_cell.assignedCell->value && current_cell.assignedCell->value > 0)
                        {
                            DrawTextureEx(numbersTextures[current_cell.assignedCell->value - 1], pos, 0, current_cell.scale, WHITE);
                        }
                        else if (current_cell.assignedCell->value == -1){
                            DrawTextureEx(otherSymbols[1], pos, 0, current_cell.scale, WHITE); // draw bomb
                        }
                    }
                    else{
                        DrawTextureEx(cellBackgroundTextures[0], pos, 0, current_cell.scale, WHITE);
                        if (current_cell.assignedCell->isFlagged){
                            DrawTextureEx(otherSymbols[0], pos, 0, current_cell.scale, WHITE);
                        }
                    }
            }


            // --- Draw UI ---

            // the scared guy

            Texture sg_current_appearance;

            switch (scaredGuy.state){
                case FACE_NATURAL:
                    sg_current_appearance = facesTextures[0];
                    break;
                case FACE_SCARED:
                    sg_current_appearance = facesTextures[1];
                    break;
                case FACE_DEAD:
                    sg_current_appearance = facesTextures[2];
                    break;
                default:
                    sg_current_appearance = facesTextures[0];
                    break;
            }

            DrawTextureEx(cellBackgroundTextures[0], (Vector2){scaredGuy.posX, scaredGuy.posY}, 0,  2, WHITE);
            DrawTextureEx(sg_current_appearance, (Vector2){scaredGuy.posX, scaredGuy.posY}, 0, 2 , WHITE);



        EndDrawing();
        // --- END OF DRAWING ---
    }

    return 0;
}

int DiscoverEmptyCells(Cell **board, int row, int column, int rows, int columns)
{

    board[row][column].isUncovered = true;
    for (int f = -1; f <= 1; f++) {
            for (int g = -1; g <= 1; g++) {
                int neighbor_r = row + f;
                int neighbor_c = column + g;

                // Only check if the neighbor is actually on the board
                if (neighbor_r >= 0 && neighbor_r < rows &&
                    neighbor_c >= 0 && neighbor_c < columns) {

                    if (board[row][column].value == 0 && !board[neighbor_r][neighbor_c].isUncovered){
                        DiscoverEmptyCells(board, neighbor_r, neighbor_c, rows, columns);
                    }
                }
            }
    }
    return 1;
    return 0;
}


Cell **GenerateNewBoard(int rows, int columns, int num_of_bombs)
{


    Cell *data = malloc(rows * columns * sizeof(Cell));

    Cell **result = malloc(rows * sizeof(Cell*));

    for (size_t i = 0; i < rows; i++){
        result[i] = data + i*columns;
    }


    for (int r = 0; r < rows; r++){
        for (int c = 0; c < columns; c++){
            result[r][c].value = 0;
            //printf(result[r][c].value);
        }
    }



    // chack that there arn't too many bombs

    if (num_of_bombs > rows * columns){
        num_of_bombs = rows * columns;
    }

    // place bombs
    while (num_of_bombs > 0){
        int selected_row = rand() % rows;
        int selected_column = rand() % columns;
        Cell *selected_cell = &result[selected_row][selected_column];

        if (selected_cell->value == 0){
            printf("Placing BOMB in cell (%i, %i)\n", selected_column, selected_row);
            selected_cell->value = -1;
            num_of_bombs--;
        }
    }
    // check the value for each cell
    for (int r = 0; r < rows; r++){
        for (int c = 0; c < columns; c++){ // for each cell
            Cell *selected_cell = &result[r][c];
            // reset the cell
            selected_cell->isUncovered = false;
            selected_cell->isFlagged = false;

            if (selected_cell->value == -1) // if the cell is a bomb, skip it
                continue;

            int related_bombs = 0;

            // check naighbors
            for (int f = -1; f <= 1; f++) {
                for (int g = -1; g <= 1; g++) {
                    int neighbor_r = r + f;
                    int neighbor_c = c + g;

                    // Only check if the neighbor is actually on the board
                    if (neighbor_r >= 0 && neighbor_r < rows &&
                        neighbor_c >= 0 && neighbor_c < columns) {

                        if (result[neighbor_r][neighbor_c].value == -1){
                            related_bombs++;
                        }
                    }
                }
            }

            selected_cell->value = related_bombs;

        }
    }


    return result;

}
