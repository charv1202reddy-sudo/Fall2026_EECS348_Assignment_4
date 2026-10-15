/*
 * Program Name : EECS 348 Assignment 4 - Recursive Object-Oriented Sudoku Solver
 *
 * Description  : Solves Sudoku puzzles with a depth-first search that backtracks.
 *                The search is written as a recursive member function of a
 *                SudokuSolver class. It finds EVERY solution of a puzzle, not
 *                just the first one, and reports "No solution found" when none exist.
 *
 * Inputs       : Five text files in the current folder: puzzle1.txt ... puzzle5.txt.
 *                Each holds 9 rows of 9 space-separated symbols. A digit 1-9 is a
 *                given clue and an underscore (_) is a blank cell.
 *
 * Output       : For every file: the file name, the puzzle as it was read, then
 *                each solution found (or "No solution found") and a solution total.
 *
 * Collaborators: None. No other students helped with this program.
 *
 * Other sources: - ChatGPT (chatgpt.com): first draft #1 of the solver. I used its
 *                  class layout, the input checking, and the solution counter.
 *                - Gemini (gemini.google.com): first draft #2 of the solver. I used
 *                  its cell-by-cell recursion and its single-loop block check.
 *                - Claude (claude.ai): helped me merge the two drafts, then helped write
 *                  and comment the final version. Code marked [AI+ME] below is from
 *                  these sources; I reviewed and tested all of it.
 *
 * Author       : Charvi Reddy Konudula
 * Creation date: October 15, 2026
 *
 * Block-authorship key used in the comments below:
 *   [CHATGPT] = idea/structure taken from ChatGPT's draft
 *   [GEMINI]  = idea/structure taken from Gemini's draft
 *   [AI+ME]   = combined/rewritten by me with Claude's help
 */

#include <iostream>   // lets us print to the screen with std::cout / std::cerr
#include <fstream>    // lets us read the puzzle files with std::ifstream
#include <string>     // gives us std::string for file names and tokens

// ---------------------------------------------------------------------------
// Named constants replace "magic numbers" so the code is easier to maintain.
// [AI+ME] (ChatGPT's draft hard-coded 9 and 3 everywhere; I named them.)
// ---------------------------------------------------------------------------
const int SIZE = 9;           // the board is SIZE x SIZE cells (9 x 9)
const int BOX = 3;            // each block (sub-grid) is BOX x BOX cells (3 x 3)
const char EMPTY = '_';       // the symbol that marks a blank cell
const int NUM_PUZZLES = 5;    // how many puzzle files we must solve

// ---------------------------------------------------------------------------
// SudokuSolver: one object holds one puzzle and knows how to solve it.
// ---------------------------------------------------------------------------
class SudokuSolver {
private:
    char board[SIZE][SIZE];      // the grid we change while searching
    char original[SIZE][SIZE];   // an untouched copy so we can print the puzzle as read
    int solutionCount;           // how many complete solutions we have found so far

    // Returns true if placing 'digit' at (row, col) breaks no Sudoku rule.
    // [GEMINI] single loop checks row, column, and block together.
    bool isSafe(int row, int col, char digit) const {
        int boxTop = BOX * (row / BOX);     // first row of the block containing this cell
        int boxLeft = BOX * (col / BOX);    // first column of the block containing this cell
        for (int i = 0; i < SIZE; ++i) {    // i walks through 9 positions
            if (board[row][i] == digit) {   // same digit already in this row?
                return false;               // then this placement is illegal
            }
            if (board[i][col] == digit) {   // same digit already in this column?
                return false;               // then this placement is illegal
            }
            // i / BOX and i % BOX turn the single index i into a row/column offset
            // inside the 3x3 block, so one loop visits all 9 block cells.
            if (board[boxTop + i / BOX][boxLeft + i % BOX] == digit) {
                return false;               // same digit already in this block
            }
        }
        return true;                        // no conflict was found, so the digit is allowed
    }

    // Prints the current contents of 'board' (used to show each solution).
    void printBoard(const char grid[SIZE][SIZE]) const {
        for (int r = 0; r < SIZE; ++r) {            // go down the rows
            for (int c = 0; c < SIZE; ++c) {        // go across the columns
                std::cout << grid[r][c];            // print the cell symbol
                if (c < SIZE - 1) {                 // put a space between cells,
                    std::cout << ' ';               // but not after the last one
                }
            }
            std::cout << '\n';                      // end the row
        }
    }

    // The recursive search. 'cell' is a number 0..80 meaning "the cell we look at now".
    // [GEMINI] cell-by-cell traversal (no rescanning for blanks). [AI+ME] flattened to one index.
    void search(int cell) {
        if (cell == SIZE * SIZE) {                  // base case: we passed the last cell, so the board is full
            ++solutionCount;                        // we found one more solution
            std::cout << "\nSolution " << solutionCount << ":\n";   // label it
            printBoard(board);                      // show the finished grid
            return;                                 // go back to look for other solutions
        }
        int row = cell / SIZE;                      // convert the cell number to its row
        int col = cell % SIZE;                      // convert the cell number to its column
        if (board[row][col] != EMPTY) {             // this cell is a given clue or already filled
            search(cell + 1);                       // just move on to the next cell
            return;                                 // nothing else to try here
        }
        for (char digit = '1'; digit <= '9'; ++digit) {   // try every candidate digit 1..9
            if (isSafe(row, col, digit)) {          // only continue if the digit is legal here
                board[row][col] = digit;            // place the digit (try it)
                search(cell + 1);                   // recursive call: solve the rest of the board
                board[row][col] = EMPTY;            // backtrack: erase it so other digits can be tried
            }
        }
        // If no digit worked we simply return, which sends us back to the previous cell.
    }

public:
    // Constructor: start with no solutions found.
    SudokuSolver() : solutionCount(0) {}

    // Reads a puzzle from a file. Returns false (and says why) if the file is bad.
    // [CHATGPT] kept its input validation; Gemini's draft had none.
    bool loadPuzzle(const std::string& filename) {
        std::ifstream file(filename);               // open the file for reading
        if (!file.is_open()) {                      // could the file not be opened?
            std::cerr << "Error: cannot open " << filename << '\n';   // tell the user
            return false;                           // report failure to the caller
        }
        std::string token;                          // holds one symbol read from the file
        for (int r = 0; r < SIZE; ++r) {            // for each of the 9 rows
            for (int c = 0; c < SIZE; ++c) {        // for each of the 9 columns
                if (!(file >> token)) {             // ran out of data before 81 symbols?
                    std::cerr << "Error: " << filename << " has fewer than 81 cells\n";
                    return false;                   // the file is too short
                }
                bool isBlank = (token == "_");                                      // is it an underscore?
                bool isDigit = (token.length() == 1 && token[0] >= '1' && token[0] <= '9');   // is it one digit 1-9?
                if (!isBlank && !isDigit) {         // anything else is not allowed
                    std::cerr << "Error: bad symbol '" << token << "' in " << filename << '\n';
                    return false;                   // reject the file
                }
                board[r][c] = token[0];             // store the symbol on the working board
                original[r][c] = token[0];          // and on the untouched copy
            }
        }
        return true;                                // all 81 cells were read correctly
    }

    // Prints the puzzle exactly as it was read from the file.
    void printOriginal() const {
        printBoard(original);                       // reuse the shared printing routine
    }

    // Starts the search and reports the totals.
    void solve() {
        solutionCount = 0;                          // reset the counter for this puzzle
        search(0);                                  // begin at cell 0 (top-left corner)
        if (solutionCount == 0) {                   // the search ended with nothing found
            std::cout << "No solution found\n";     // required wording from the assignment
        } else {                                    // at least one solution exists
            std::cout << "\nTotal solutions found: " << solutionCount << '\n';   // print the count
        }
    }
};

// ---------------------------------------------------------------------------
// main: solves puzzle1.txt through puzzle5.txt, one SudokuSolver object each.
// ---------------------------------------------------------------------------
int main() {
    for (int i = 1; i <= NUM_PUZZLES; ++i) {                        // loop over the five puzzles
        std::string filename = "puzzle" + std::to_string(i) + ".txt";   // build the file name
        std::cout << "========================================\n";  // separator line
        std::cout << "File: " << filename << '\n';                  // show which file we are on
        std::cout << "========================================\n";  // separator line
        SudokuSolver solver;                                        // a fresh object for this puzzle
        if (solver.loadPuzzle(filename)) {                          // only continue if the file loaded
            std::cout << "\nPuzzle:\n";                             // heading for the original grid
            solver.printOriginal();                                 // print the puzzle as read
            solver.solve();                                         // find and print all solutions
        }
        std::cout << '\n';                                          // blank line between puzzles
    }
    return 0;                                                       // 0 tells the OS the program succeeded
}
