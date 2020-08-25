#pragma once

struct ITEM_INFO;
struct COLL_INFO;

void InitialiseElementPuzzle(short itemNumber);
void ElementPuzzleControl(short itemNumber);
void ElementPuzzleDoCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void ElementPuzzleCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);