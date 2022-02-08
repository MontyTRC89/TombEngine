#pragma once

struct ITEM_INFO;
struct COLL_INFO;

void InitialiseGameStix(short itemNumber);
void GameStixControl(short itemNumber);
void _0x0040FAE0(ITEM_INFO* item);
void SenetPieceExplosionEffect(ITEM_INFO* item, int color, int speed);
void trigger_item_in_room(short room_number, int object);
bool CheckSenetWinner(short num);
void MakeMove(int piece, int displacement);
void GameStixCollision(short item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void ControlGodHead(short itemNumber);
void InitialiseGamePiece(short itemNumber);
void SenetControl(short itemNumber);
