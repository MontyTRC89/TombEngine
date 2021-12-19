#pragma once

struct ITEM_INFO;
struct COLL_INFO;
/*puzzles*/
void PuzzleHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PuzzleDoneCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PuzzleDone(ITEM_INFO* item, short itemNum);
void DoPuzzle();
/*keys*/
void KeyHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
