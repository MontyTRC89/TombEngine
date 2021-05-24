#pragma once

void InitialiseGameStix(short itemNumber);
void GameStixControl(short itemNumber);
void _0x0040FAE0(ITEM_INFO* item);
void SenetPieceExplosionEffect(ITEM_INFO* item, int color, int speed);
void trigger_item_in_room(short room_number, int object);
int CheckSenetWinner(short num);
