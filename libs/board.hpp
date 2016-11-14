#ifndef BOARD_H_
#define BOARD_H_ 

#include <string>

using namespace std;

class Board {

    int score;

public:

    Board() { 

        score = 0;

        // Print static information on board
        mvprintw(0, 0, "SCORE: ");
        mvprintw(0, 13, "SIZE: ");
        mvprintw(0, 25, "H.POS: (");
        mvprintw(0, 35, ",");
        mvprintw(0, 38, ")  F.POS: (");
        mvprintw(0, 51, ",");
        mvprintw(0, 54, ")");
        mvprintw(0, 60, "HIGHSCORE: ");

        // Print static board limits
        for(int i = 1; i < LINES - 1; i++){
            mvprintw(i, 0, "|"); mvprintw(i, COLS - 1, "|");    
        }

        for(int j = 1;  j < COLS - 1; j++){
            mvprintw(1, j, "-"); mvprintw(LINES - 1, j, "-");
        }

        mvprintw(1,0,"+");
        mvprintw(1,COLS - 1,"+");
        mvprintw(LINES - 1,0,"+");
        mvprintw(LINES - 1, COLS - 1,"+");
    }

    int getScore() const { return score; }
    //void increaseScore(int level) { score += level; }

    void update() {
        refresh(); // update board with ncurses help
    }

    char getChar(const Point &p) const {
        return mvinch(p.getX(), p.getY()) & A_CHARTEXT;
    }

    void setPrintFood(const Point &f) {
        
        // Fill with empty space
        mvprintw(0, 49, "  ");
        mvprintw(0, 52, "  ");

        // Now print Food position
        mvprintw(0, 49, to_string(f.getX()).c_str());
        mvprintw(0, 52, to_string(f.getY()).c_str());

        // Now print Food on board
        mvprintw(f.getX(), f.getY(), "f");
    }

    void setPrintSnake(const Body &b) { 
        
        // Fill with empty space
        mvprintw(0, 33, "  ");
        mvprintw(0, 36, "  ");

        // Now print Head position
        mvprintw(0, 33, to_string(b.getHead().getX()).c_str());
        mvprintw(0, 36, to_string(b.getHead().getY()).c_str());

        mvprintw(b.getHead().getX(), b.getHead().getY(), "@");
        // trick code to make a ilusion movement
        mvprintw(b.getTail().getX(), b.getTail().getY(), " ");
    }

    void setPrintScore(int level) {
        score = score + level;
        mvprintw(0, 7, to_string(score - level).c_str());
    }

    void setPrintSize(const Body &b) { mvprintw(0, 19, to_string(b.getSize()).c_str()); }

    void printGameOver(){
        string strGameOver[7];

        strGameOver[0] = " #####                                                               ###";
        strGameOver[1] = "#     #    ##    #    #  ######       ####   #    #  ######  #####   ###";
        strGameOver[2] = "#         #  #   ##  ##  #           #    #  #    #  #       #    #  ###";
        strGameOver[3] = "#  ####  #    #  # ## #  #####       #    #  #    #  #####   #    #   # ";
        strGameOver[4] = "#     #  ######  #    #  #           #    #  #    #  #       #####      ";
        strGameOver[5] = "#     #  #    #  #    #  #           #    #   #  #   #       #   #   ###";
        strGameOver[6] = " #####   #    #  #    #  ######       ####     ##    ######  #    #  ###";

        mvprintw(9, 4, strGameOver[0].c_str());
        mvprintw(10, 4, strGameOver[1].c_str());
        mvprintw(11, 4, strGameOver[2].c_str());
        mvprintw(12, 4, strGameOver[3].c_str());
        mvprintw(13, 4, strGameOver[4].c_str());
        mvprintw(14, 4, strGameOver[5].c_str());
        mvprintw(15, 4, strGameOver[6].c_str());
    }

};

#endif