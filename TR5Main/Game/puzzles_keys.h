#pragma once

struct ITEM_INFO;
struct COLL_INFO;
/*puzzles*/
void PuzzleHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PuzzleDoneCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PuzzleDone(ITEM_INFO* item, short itemNum);
void do_puzzle();
/*keys*/
void KeyHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int KeyTrigger(short itemNum);
