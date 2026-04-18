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
    Rectangle btnRect;
    int posX;
    int posY;
    bool isPressed;
} Dude;

typedef struct BoardDifficulty{
    int width;
    int hight;
    int num_of_bombs;
    int board_scale;
} BoardDifficulty;

// declare functions
Cell **GenerateNewBoard(int, int, int);
void DiscoverEmptyCells(Cell**, VisualCell*, int, int, int, int);
Cell **ResetBoard(Cell** last_board, VisualCell* visual_cell_arr, int* cell_index , int rows, int columns, int num_of_bombs, int board_scale, Vector2 board_offset, int margin);
Cell** ResetBoardViaDifficulty(Cell** last_board, VisualCell* visual_cell_arr, int* cell_index, int difficulty, Vector2 board_offset, int margin, int* board_width_var, int* board_hight_var);

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



    const int BOARD_SCALE = 2;

    int margin = 0;

    VisualCell *visualCells = malloc(sizeof(VisualCell) * 2500); // set to 2500 because it is the size of the hareders difficulty
    //Rectangle *cellsRects = malloc(sizeof(Rectangle) * BOARD_HIGHT * BOARD_WIDTH);


    int cellIndex = 0;
    board = ResetBoard(board, visualCells, &cellIndex, BOARD_HIGHT, BOARD_WIDTH, NUM_OF_BOMBS, BOARD_SCALE, (Vector2){BOARD_OFFSET_X, BOARD_OFFSET_Y}, margin);

    // set size to match game
    SetWindowSize((BOARD_WIDTH * CELL_WIDTH * BOARD_SCALE)  + BOARD_OFFSET_X, (BOARD_HIGHT * CELL_HIGHT * BOARD_SCALE)  + BOARD_OFFSET_Y);


    // scared guy button

    Dude scaredGuy;

    scaredGuy.state = FACE_NATURAL;
    scaredGuy.posX = GetScreenWidth() / 2 - 32;
    scaredGuy.posY = 20;
    scaredGuy.isPressed = false;
    scaredGuy.btnRect = (Rectangle){scaredGuy.posX, scaredGuy.posY, 64, 64};


    // --- START GAME LOOP ---

    SetTargetFPS(60);



    // in-game vars
    bool is_mouse_0_down = false; // left click
    bool is_mouse_1_down = false; // right click

    // ui vars
    int current_selected_difficulty = 0;      // Index of selected item
    bool isDifficultySelectionActive = false;

    // Main Loop
    while (!WindowShouldClose())
    {
        // --- START OF UPDATE ---
        if (is_mouse_0_down){

            if (GetMousePosition().y > BOARD_OFFSET_Y){ // the player is pressing on the board
                scaredGuy.state = FACE_SCARED;
            }
            else{
                if (CheckCollisionPointRec(GetMousePosition(), scaredGuy.btnRect) && !isDifficultySelectionActive){
                    scaredGuy.isPressed = true;
                }
            }


            if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){ // on mouse 0 up

                // scared guy button
                if (CheckCollisionPointRec(GetMousePosition(), scaredGuy.btnRect)){
                    scaredGuy.isPressed = false;
                    //board = ResetBoard(board, visualCells, &cellIndex, BOARD_HIGHT, BOARD_WIDTH, NUM_OF_BOMBS, BOARD_SCALE, (Vector2){BOARD_OFFSET_X, BOARD_OFFSET_Y}, margin);
                    board = ResetBoardViaDifficulty(board, visualCells, &cellIndex, current_selected_difficulty, (Vector2){BOARD_OFFSET_X, BOARD_OFFSET_Y}, margin, &BOARD_WIDTH, &BOARD_HIGHT);
                    //ResetBoard(board, BOARD_WIDTH, BOARD_HIGHT, NUM_OF_BOMBS);
                }

                // board buttons

                for (int c = 0; c < cellIndex; c++){ // fore each cell check if the mouse is overlapping
                    if (CheckCollisionPointRec(GetMousePosition(), visualCells[c].btnRect)){
                        Cell* selected_cell = visualCells[c].assignedCell;
                        if (!selected_cell->isFlagged && !isDifficultySelectionActive) // uncover cell
                        {
                            if (selected_cell->value == 0)
                                DiscoverEmptyCells(board, visualCells, c / BOARD_WIDTH, c % BOARD_WIDTH, BOARD_HIGHT, BOARD_WIDTH);
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

            DrawTextureEx(cellBackgroundTextures[scaredGuy.isPressed ? 1 : 0], (Vector2){scaredGuy.posX, scaredGuy.posY}, 0,  2, WHITE);
            DrawTextureEx(sg_current_appearance, (Vector2){scaredGuy.posX, scaredGuy.posY}, 0, 2 , WHITE);

            //  difficulty changer
            //int selected_difficulty =
            // 1. Define state variables outside the main loop


            // 2. Inside your drawing loop
            GuiLabel((Rectangle){10, 10, 200, 20}, "Difficulty");
            if (GuiDropdownBox((Rectangle){ 10, 40, 200, 30 }, "EASY;MEDDIUM;HARD;WILL COVER YOUR ENTIRE SCREEN", &current_selected_difficulty, isDifficultySelectionActive))
{
            // When clicked, toggle the edit mode
                isDifficultySelectionActive = !isDifficultySelectionActive;
                printf("%i", current_selected_difficulty);
            }


        EndDrawing();
        // --- END OF DRAWING ---
    }

    return 0;
}

void DiscoverEmptyCells(Cell **board, VisualCell* visualCells, int row, int column, int rows, int columns)
{
    printf("discovering at %i, %i\n", row, column);
    board[row][column].isUncovered = true;
    //visualCells[row * columns + column].assignedCell->isUncovered = true;
    for (int f = -1; f <= 1; f++) {
            for (int g = -1; g <= 1; g++) {
                int neighbor_r = row + f;
                int neighbor_c = column + g;

                // Only check if the neighbor is actually on the board
                if (neighbor_r >= 0 && neighbor_r < rows &&
                    neighbor_c >= 0 && neighbor_c < columns) {

                    if (board[row][column].value == 0 && !board[neighbor_r][neighbor_c].isUncovered){
                        DiscoverEmptyCells(board, visualCells, neighbor_r, neighbor_c, rows, columns);
                    }
                }
            }
    }

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

void FreeBoard(Cell** board) // a function to free a board (mainly for the reset function)
{
    if (board == NULL) return;
    // free the actual data
    free(board[0]);
    // free the pointer to the data
    free(board);
}



Cell** ResetBoardViaDifficulty(Cell** last_board, VisualCell* visual_cell_arr, int* cell_index, int difficulty, Vector2 board_offset, int margin, int* board_width_var, int* board_hight_var){
    BoardDifficulty d;
    switch (difficulty){
    case 0:
        d = (BoardDifficulty){9, 9, 10, 2};
        break;
    case 1:
        d = (BoardDifficulty){16, 16, 40, 2};
        break;
    case 2:
        d = (BoardDifficulty){30, 16, 99, 2};
        break;
    case 3:
        d = (BoardDifficulty){50, 50, 625, 1}; // will cover your entire screen
        break;
    };

    *board_hight_var = d.hight;
    *board_width_var = d.width;

    SetWindowSize((d.width * 32 * d.board_scale)  + board_offset.x, (d.hight * 32 * d.board_scale)  + board_offset.y);
    return ResetBoard(last_board, visual_cell_arr, cell_index, d.hight, d.width, d.num_of_bombs, d.board_scale, board_offset, margin);

}


Cell** ResetBoard(Cell** last_board, VisualCell* visual_cell_arr, int* cell_index , int rows, int columns, int num_of_bombs, int board_scale, Vector2 board_offset, int margin)
{
    if (last_board != NULL) {
        FreeBoard(last_board);
    }


    Cell **new_board = GenerateNewBoard(rows, columns, num_of_bombs);
    *cell_index = 0;



    // reset visual cells
    int currentCellPosX = 0;
    int currentCellPosY = 0;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < columns; c++)
        {
            VisualCell vc;

            vc.assignedCell = &new_board[r][c];
            vc.posX = currentCellPosX + board_offset.x;
            vc.posY = currentCellPosY + board_offset.y;
            vc.scale = board_scale;
            vc.btnRect = (Rectangle){
                currentCellPosX + board_offset.x,
                currentCellPosY + board_offset.y,
                32 * board_scale,
                32 * board_scale
            };

            visual_cell_arr[*cell_index] = vc;

            currentCellPosX += (32 * board_scale) + margin;
            *cell_index += 1;
        }
        currentCellPosY += 32 * board_scale + margin;
        currentCellPosX = 0;
    }

    return new_board;

}
