#include "raylib.h"   // raylib: window, drawing, keyboard and mouse
#include <stdio.h>    // file handling: fopen, fscanf, fprintf, fclose
#include <string.h>   // strcpy, strlen, strchr
#include <time.h>     // time() is used to make random numbers different every run

/* ================= FILE FUNCTIONS (used by Minesweeper and Flappy Bird) ================= */

int LoadScore(char filename[])
{
    int score = 0;                      // if the file does not exist the score stays 0
    FILE *f = fopen(filename, "r");     // open the file for reading
    if (f != NULL)                      // NULL means the file was not found
    {
        fscanf(f, "%d", &score);        // read the saved number from the file
        fclose(f);                      // always close the file after use
    }
    return score;                       // send the score back to the caller
}

void SaveScore(char filename[], int score)
{
    FILE *f = fopen(filename, "w");     // "w" deletes the old content and writes new content
    if (f != NULL)
    {
        fprintf(f, "%d", score);        // write the score into the file
        fclose(f);
    }
}

/* ================= MODULE 1 : TIC TAC TOE ================= */

void TicTacToe(void)
{
    char board[3][3];       // 3x3 board, every cell holds ' ', 'X' or 'O'
    char turn = 'X';        // whose turn it is
    char winner = ' ';      // ' ' = nobody yet, 'X' or 'O' = winner, 'D' = draw
    int moves = 0;          // how many cells are filled
    int r, c, i;            // loop counters / row and column
    int back = 0;           // 1 = go back to the menu
    int reset = 1;          // 1 = clear the board (also runs once at the start)

    while (!back && !WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE)) back = 1;     // Esc goes back to the menu
        if (IsKeyPressed(KEY_R)) reset = 1;         // R restarts the game

        if (reset)                                  // clear everything for a new game
        {
            for (r = 0; r < 3; r++)
                for (c = 0; c < 3; c++)
                    board[r][c] = ' ';
            turn = 'X';
            winner = ' ';
            moves = 0;
            reset = 0;
        }

        // the board starts at x=250, y=150 and every cell is 100 x 100 pixels
        if (winner == ' ' && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            GetMouseX() >= 250 && GetMouseX() < 550 &&
            GetMouseY() >= 150 && GetMouseY() < 450)
        {
            c = (GetMouseX() - 250) / 100;          // mouse position -> column number
            r = (GetMouseY() - 150) / 100;          // mouse position -> row number

            if (board[r][c] == ' ')                 // only an empty cell can be used
            {
                board[r][c] = turn;                 // put X or O in the cell
                moves++;

                for (i = 0; i < 3; i++)             // check all 3 rows and all 3 columns
                {
                    if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2])
                        winner = board[i][0];
                    if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i])
                        winner = board[0][i];
                }
                // check both diagonals (they both pass through the middle cell)
                if (board[1][1] != ' ' &&
                    ((board[0][0] == board[1][1] && board[1][1] == board[2][2]) ||
                     (board[0][2] == board[1][1] && board[1][1] == board[2][0])))
                    winner = board[1][1];

                if (winner == ' ' && moves == 9) winner = 'D';      // board full and no winner = draw
                if (winner == ' ') turn = (turn == 'X') ? 'O' : 'X'; // switch the turn
            }
        }

        BeginDrawing();                             // start drawing this frame
        ClearBackground(WHITE);                     // white background

        DrawText("TIC TAC TOE", 270, 30, 40, BLACK);
        if (winner == ' ') DrawText(TextFormat("Turn: %c", turn), 340, 90, 30, BLACK);
        else if (winner == 'D') DrawText("It's a draw!", 320, 90, 30, BLACK);
        else DrawText(TextFormat("%c wins!", winner), 340, 90, 30, BLACK);

        for (i = 1; i < 3; i++)                     // draw the 4 lines of the grid
        {
            DrawRectangle(250 + i * 100 - 2, 150, 4, 300, BLACK);   // vertical line
            DrawRectangle(250, 150 + i * 100 - 2, 300, 4, BLACK);   // horizontal line
        }

        for (r = 0; r < 3; r++)                     // draw every X and O
            for (c = 0; c < 3; c++)
                if (board[r][c] != ' ')
                    DrawText(TextFormat("%c", board[r][c]), 250 + c * 100 + 28, 150 + r * 100 + 15, 70, BLACK);

        DrawText("Click to play   |   R = restart   |   Esc = menu", 190, 520, 20, GRAY);
        EndDrawing();                               // show the frame on screen
    }
}

/* ================= MODULE 2 : MINESWEEPER ================= */

#define MS_SIZE  10     // grid is 10 x 10
#define MS_MINES 15     // total mines
#define MS_CELL  40     // size of one cell in pixels
#define MS_X     200    // left side of the grid
#define MS_Y     120    // top side of the grid

int mine[MS_SIZE][MS_SIZE];     // 1 = this cell has a mine
int around[MS_SIZE][MS_SIZE];     // how many mines are around this cell
int state[MS_SIZE][MS_SIZE];    // 0 = hidden, 1 = opened, 2 = flagged

void NewMinefield(void)         // clears the grid and places the mines randomly
{
    int r, c, dr, dc, count, placed = 0;

    for (r = 0; r < MS_SIZE; r++)
        for (c = 0; c < MS_SIZE; c++)
        {
            mine[r][c] = 0;     // no mine
            state[r][c] = 0;    // everything hidden
        }

    while (placed < MS_MINES)   // keep going until all mines are placed
    {
        r = GetRandomValue(0, MS_SIZE - 1);     // random row
        c = GetRandomValue(0, MS_SIZE - 1);     // random column
        if (mine[r][c] == 0)                    // skip cells that already have a mine
        {
            mine[r][c] = 1;
            placed++;
        }
    }

    for (r = 0; r < MS_SIZE; r++)               // count the mines around every cell
        for (c = 0; c < MS_SIZE; c++)
        {
            count = 0;
            for (dr = -1; dr <= 1; dr++)        // look at the 8 neighbours
                for (dc = -1; dc <= 1; dc++)
                    if (r + dr >= 0 && r + dr < MS_SIZE && c + dc >= 0 && c + dc < MS_SIZE)
                        count = count + mine[r + dr][c + dc];
            around[r][c] = count;
        }
}

void Reveal(int r, int c)       // opens a cell (recursion: a cell with 0 mines opens its neighbours)
{
    int dr, dc;
    if (r < 0 || r >= MS_SIZE || c < 0 || c >= MS_SIZE) return;     // outside the grid: stop
    if (state[r][c] != 0) return;                                   // already opened or flagged: stop
    state[r][c] = 1;                                                // open this cell
    if (around[r][c] == 0)                                          // no mines around: open neighbours too
        for (dr = -1; dr <= 1; dr++)
            for (dc = -1; dc <= 1; dc++)
                Reveal(r + dr, c + dc);                             // function calls itself
}

void Minesweeper(void)
{
    int r, c, x, y, t;
    int status = 0;                             // 0 = playing, 1 = won, 2 = lost
    int started = 0;                            // timer starts after the first click
    float timer = 0;                            // seconds passed
    int best = LoadScore("minesweeper.txt");    // fastest win from the file (0 = no record)
    int opened, flags;                          // counters for opened cells and flags
    int back = 0, reset = 1;

    while (!back && !WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE)) back = 1;
        if (IsKeyPressed(KEY_R)) reset = 1;

        if (reset)                              // new game
        {
            NewMinefield();
            status = 0;
            started = 0;
            timer = 0;
            reset = 0;
        }

        if (status == 0 && started) timer = timer + GetFrameTime();    // GetFrameTime = seconds since last frame

        // is the mouse inside the grid?
        if (status == 0 && GetMouseX() >= MS_X && GetMouseX() < MS_X + MS_SIZE * MS_CELL &&
                           GetMouseY() >= MS_Y && GetMouseY() < MS_Y + MS_SIZE * MS_CELL)
        {
            c = (GetMouseX() - MS_X) / MS_CELL;         // mouse position -> column
            r = (GetMouseY() - MS_Y) / MS_CELL;         // mouse position -> row

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && state[r][c] == 0)
            {
                started = 1;                            // start the timer
                if (mine[r][c] == 1) status = 2;        // clicked a mine: game lost
                else Reveal(r, c);                      // safe cell: open it
            }
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))   // right click puts or removes a flag
            {
                if (state[r][c] == 0) state[r][c] = 2;
                else if (state[r][c] == 2) state[r][c] = 0;
            }
        }

        opened = 0;
        flags = 0;
        for (r = 0; r < MS_SIZE; r++)           // count opened cells and flags
            for (c = 0; c < MS_SIZE; c++)
            {
                if (state[r][c] == 1) opened++;
                if (state[r][c] == 2) flags++;
            }

        if (status == 0 && opened == MS_SIZE * MS_SIZE - MS_MINES)     // all safe cells opened = win
        {
            status = 1;
            t = (int)timer;                     // time in whole seconds
            if (t < 1) t = 1;
            if (best == 0 || t < best)          // new record (lower time is better)
            {
                best = t;
                SaveScore("minesweeper.txt", best);     // save the record in the file
            }
        }

        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("MINESWEEPER", 250, 20, 40, BLACK);
        DrawText(TextFormat("Mines: %d", MS_MINES - flags), MS_X, 75, 24, BLACK);
        DrawText(TextFormat("Time: %d", (int)timer), 360, 75, 24, BLACK);
        if (best > 0) DrawText(TextFormat("Best: %ds", best), 500, 75, 24, BLACK);
        else DrawText("Best: --", 500, 75, 24, BLACK);

        for (r = 0; r < MS_SIZE; r++)
            for (c = 0; c < MS_SIZE; c++)
            {
                x = MS_X + c * MS_CELL;         // top-left corner of this cell
                y = MS_Y + r * MS_CELL;

                if (state[r][c] == 1)           // opened cell: outline + number
                {
                    DrawRectangleLines(x, y, MS_CELL - 2, MS_CELL - 2, GRAY);
                    if (around[r][c] > 0)
                        DrawText(TextFormat("%d", around[r][c]), x + 14, y + 8, 24, BLACK);
                }
                else                            // hidden cell: gray box
                {
                    DrawRectangle(x, y, MS_CELL - 2, MS_CELL - 2, LIGHTGRAY);
                    if (state[r][c] == 2) DrawText("F", x + 14, y + 8, 24, RED);   // flag
                    if (status == 2 && mine[r][c] == 1)                            // lost: show the mines
                        DrawRectangle(x, y, MS_CELL - 2, MS_CELL - 2, RED);
                }
            }

        if (status == 1) DrawText("You win!", 340, 540, 30, BLACK);
        if (status == 2) DrawText("Boom! You hit a mine.", 270, 540, 30, BLACK);
        DrawText("Left click: open   Right click: flag   R: restart   Esc: menu", 140, 570, 20, GRAY);
        EndDrawing();
    }
}

/* ================= MODULE 3 : HANGMAN ================= */
/* hangman.txt format: first number = best streak, then 100 words (CAPITAL letters, one per line) */

void Hangman(void)
{
    char words[100][20];        // 100 words, each word up to 19 letters
    int count = 0;              // how many words were read from the file
    int best = 0;               // best streak (stored in the file)
    int streak = 0;             // words guessed in a row
    char *word;                 // pointer to the word chosen for this round
    int guessed[26] = { 0 };    // guessed[0] is letter A, guessed[1] is B ... 1 = already guessed
    int wrong = 0;              // wrong guesses
    int status = 0;             // 0 = playing, 1 = won, 2 = lost
    int key, i, x, len, found;
    int back = 0, newWord = 1;
    FILE *f;

    f = fopen("hangman.txt", "r");              // open the word file
    if (f != NULL)
    {
        fscanf(f, "%d", &best);                 // first value in the file is the best streak
        while (count < 100 && fscanf(f, "%19s", words[count]) == 1)     // read words one by one
            count++;
        fclose(f);
    }
    if (count == 0)                             // file missing: use one word so the game still works
    {
        strcpy(words[0], "COMPUTER");
        count = 1;
    }

    while (!back && !WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE)) back = 1;

        if (newWord)                            // choose a new random word
        {
            word = words[GetRandomValue(0, count - 1)];
            for (i = 0; i < 26; i++) guessed[i] = 0;
            wrong = 0;
            status = 0;
            newWord = 0;
        }

        key = GetKeyPressed();                  // key pressed this frame (KEY_A to KEY_Z are 'A' to 'Z')
        if (status == 0 && key >= 'A' && key <= 'Z' && guessed[key - 'A'] == 0)
        {
            guessed[key - 'A'] = 1;             // mark this letter as used
            if (strchr(word, key) == NULL) wrong++;     // strchr searches the letter inside the word

            found = 1;                          // assume the whole word is guessed
            len = strlen(word);
            for (i = 0; i < len; i++)
                if (guessed[word[i] - 'A'] == 0) found = 0;     // one missing letter means not finished

            if (found)                          // word completed
            {
                status = 1;
                streak++;
                if (streak > best)              // new high score: rewrite the file (score + words)
                {
                    best = streak;
                    f = fopen("hangman.txt", "w");
                    if (f != NULL)
                    {
                        fprintf(f, "%d\n", best);
                        for (i = 0; i < count; i++) fprintf(f, "%s\n", words[i]);
                        fclose(f);
                    }
                }
            }
            else if (wrong == 6)                // 6 wrong guesses: lost
            {
                status = 2;
                streak = 0;
            }
        }
        if (status != 0 && IsKeyPressed(KEY_ENTER)) newWord = 1;    // Enter = next word

        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("HANGMAN", 300, 20, 40, BLACK);
        DrawText(TextFormat("Wrong: %d / 6", wrong), 450, 130, 30, BLACK);
        DrawText(TextFormat("Streak: %d", streak), 450, 180, 30, BLACK);
        DrawText(TextFormat("Best: %d", best), 450, 230, 30, BLACK);
        if (status == 1) DrawText("You won! Press Enter", 450, 300, 24, BLACK);
        if (status == 2) DrawText("You lost! Press Enter", 450, 300, 24, BLACK);

        DrawRectangle(100, 420, 200, 10, BLACK);        // gallows base
        DrawRectangle(150, 120, 10, 300, BLACK);        // pole
        DrawRectangle(150, 120, 120, 10, BLACK);        // top beam
        DrawLine(267, 130, 267, 170, BLACK);            // rope
        if (wrong >= 1) DrawCircleLines(267, 190, 20, BLACK);       // head
        if (wrong >= 2) DrawLine(267, 210, 267, 320, BLACK);        // body
        if (wrong >= 3) DrawLine(267, 235, 227, 285, BLACK);        // left arm
        if (wrong >= 4) DrawLine(267, 235, 307, 285, BLACK);        // right arm
        if (wrong >= 5) DrawLine(267, 320, 232, 380, BLACK);        // left leg
        if (wrong >= 6) DrawLine(267, 320, 302, 380, BLACK);        // right leg

        len = strlen(word);                             // strlen gives the number of letters
        for (i = 0; i < len; i++)                       // draw the word: one underline per letter
        {
            x = (800 - len * 40) / 2 + i * 40;          // centers the word on the screen
            if (guessed[word[i] - 'A'] == 1 || status == 2)     // show guessed letters (all if lost)
                DrawText(TextFormat("%c", word[i]), x + 8, 465, 30, BLACK);
            DrawRectangle(x + 4, 505, 28, 3, BLACK);
        }

        for (i = 0; i < 26; i++)                        // alphabet row: used letters turn light gray
            DrawText(TextFormat("%c", 'A' + i), 36 + i * 28, 545, 24, guessed[i] ? LIGHTGRAY : BLACK);

        DrawText("Type letters on the keyboard   |   Esc = menu", 200, 575, 18, GRAY);
        EndDrawing();
    }
}

/* ================= MODULE 4 : FLAPPY BIRD ================= */

void FlappyBird(void)
{
    float y = 300, speed = 0;       // bird height and its up/down speed
    int pipeX[3], gapY[3];          // x position of each pipe and where its gap starts
    int passed[3];                  // 1 = bird already passed this pipe (score given)
    int score = 0, status = 0;      // status: 0 = waiting, 1 = playing, 2 = game over
    int best = LoadScore("flappy.txt");     // highest score from the file
    int i, flap, dead;
    int back = 0, reset = 1;

    while (!back && !WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE)) back = 1;

        if (reset)                              // set up a new game
        {
            y = 300;
            speed = 0;
            score = 0;
            status = 0;
            for (i = 0; i < 3; i++)
            {
                pipeX[i] = 800 + i * 300;               // pipes start off screen, 300 pixels apart
                gapY[i] = GetRandomValue(80, 330);      // random height of the gap
                passed[i] = 0;
            }
            reset = 0;
        }

        flap = IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON);  // jump button
        dead = 0;

        if (status == 0 && flap) status = 1;            // first flap starts the game

        if (status == 1)
        {
            if (flap) speed = -7;                       // negative speed = move up
            speed = speed + 0.4;                        // gravity pulls the bird down
            y = y + speed;                              // move the bird

            for (i = 0; i < 3; i++)
            {
                pipeX[i] = pipeX[i] - 3;                // move pipe to the left
                if (pipeX[i] < -60)                     // pipe left the screen: send it to the back
                {
                    pipeX[i] = pipeX[i] + 900;
                    gapY[i] = GetRandomValue(80, 330);
                    passed[i] = 0;
                }
                if (passed[i] == 0 && pipeX[i] + 60 < 150)      // bird (x=150) crossed the pipe
                {
                    score++;
                    passed[i] = 1;
                }
                // bird is treated as a 30x30 box; check if it touches the top or bottom pipe
                if (165 > pipeX[i] && 135 < pipeX[i] + 60 && (y - 15 < gapY[i] || y + 15 > gapY[i] + 150))
                    dead = 1;
            }
            if (y + 15 > 550 || y - 15 < 0) dead = 1;   // touched ground or the top of the screen

            if (dead)
            {
                status = 2;
                if (score > best)                       // new high score
                {
                    best = score;
                    SaveScore("flappy.txt", best);      // save it in the file
                }
            }
        }
        else if (status == 2 && flap) reset = 1;        // flap after game over = play again

        BeginDrawing();
        ClearBackground(WHITE);

        for (i = 0; i < 3; i++)                         // draw top and bottom part of every pipe
        {
            DrawRectangle(pipeX[i], 0, 60, gapY[i], DARKGRAY);
            DrawRectangle(pipeX[i], gapY[i] + 150, 60, 550 - (gapY[i] + 150), DARKGRAY);
        }
        DrawRectangle(0, 550, 800, 50, BLACK);          // ground
        DrawCircle(150, (int)y, 15, RED);               // bird

        DrawText(TextFormat("%d", score), 380, 30, 60, BLACK);
        DrawText(TextFormat("Best: %d", best), 20, 20, 24, BLACK);
        if (status == 0) DrawText("Press SPACE or click to flap", 230, 250, 30, BLACK);
        if (status == 2) DrawText("GAME OVER - SPACE to retry", 220, 250, 30, BLACK);
        DrawText("Esc = menu", 650, 565, 20, WHITE);
        EndDrawing();
    }
}

/* ================= MAIN : window + menu ================= */

int main(void)
{
    int key, quit = 0;

    InitWindow(800, 600, "Game Hub");       // open a 800 x 600 window
    SetExitKey(KEY_NULL);                   // Esc should not close the window (we use it for "back")
    SetTargetFPS(60);                       // run at 60 frames per second
    SetRandomSeed((unsigned int)time(NULL));    // different random numbers every run

    while (!quit && !WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);
        DrawText("GAME HUB", 250, 80, 70, BLACK);
        DrawText("1 - Tic Tac Toe", 270, 220, 32, BLACK);
        DrawText("2 - Minesweeper", 270, 280, 32, BLACK);
        DrawText("3 - Hangman", 270, 340, 32, BLACK);
        DrawText("4 - Flappy Bird", 270, 400, 32, BLACK);
        DrawText("Esc - Quit", 270, 480, 32, GRAY);
        EndDrawing();

        key = GetKeyPressed();              // which key was pressed?
        switch (key)                        // run the chosen game (each game is its own function)
        {
            case KEY_ONE:    TicTacToe();   break;
            case KEY_TWO:    Minesweeper(); break;
            case KEY_THREE:  Hangman();     break;
            case KEY_FOUR:   FlappyBird();  break;
            case KEY_ESCAPE: quit = 1;      break;
        }
    }

    CloseWindow();                          // close the window
    return 0;
}
