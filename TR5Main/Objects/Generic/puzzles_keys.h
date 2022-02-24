#pragma once

struct ITEM_INFO;
struct COLL_INFO;

// Puzzles
void PuzzleHoleCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
void PuzzleDoneCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
void PuzzleDone(ITEM_INFO* item, short itemNumber);
void DoPuzzle();

// Keys
void KeyHoleCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
