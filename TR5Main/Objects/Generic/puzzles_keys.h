#pragma once

struct ITEM_INFO;
struct CollisionInfo;

// Puzzles
void PuzzleHoleCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void PuzzleDoneCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void PuzzleDone(ITEM_INFO* item, short itemNumber);
void DoPuzzle();

// Keys
void KeyHoleCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
