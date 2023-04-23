#pragma once

struct CollisionInfo;
struct ItemInfo;

enum ReusableType
{
	None = 0,
	Done = 1,
	Hole = 2
};

// Puzzles
void PuzzleHoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void PuzzleDoneCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void PuzzleDone(ItemInfo* item, short itemNumber);
void PuzzleHole(ItemInfo* item, short itemNumber);
void DoPuzzle();
void InitializePuzzleDone(short itemNumber);
void InitializePuzzleHole(short itemNumber);

// Keys
void KeyHoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
