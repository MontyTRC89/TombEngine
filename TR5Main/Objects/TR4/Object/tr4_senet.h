#pragma once

struct ITEM_INFO;
struct CollisionInfo;

void InitialiseGameSticks(short itemNumber);
void GameSticksControl(short itemNumber);
void _0x0040FAE0(ITEM_INFO* item);
void SenetPieceExplosionEffect(ITEM_INFO* item, int color, int speed);
void TriggerItemInRoom(short roomNumber, int object);
bool CheckSenetWinner(short number);
void MakeMove(int piece, int displacement);
void GameSticksCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void ControlGodHead(short itemNumber);
void InitialiseGamePiece(short itemNumber);
void SenetControl(short itemNumber);
