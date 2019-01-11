#include "util/SimpleException.h"
#include "player/PlayerManager.h"
#include "player/Bot.h"
#include "player/PlayerManager.h"
#include "navmesh/nav_mesh.h"
#include <convar.h>
#include <eiface.h>
#include <iplayerinfo.h>
#include <utlmap.h>

/*
 extern PlayerManager *playerManager;

 Vector getListenClientPos() {
 Vector pos;
 auto& players = playerManager->getPlayers();
 FOR_EACH_MAP_FAST(players, i) {
 IPlayerInfo* player = playerinfomanager->GetPlayerInfo(
 const_cast<edict_t*>(players[i]->getEdict()));
 if (!player->IsFakeClient()) {
 pos = player->GetAbsOrigin();
 break;
 }
 }
 return pos;
 }

 void botComeCmd() {
 Vector pos = getListenClientPos();
 auto& players = playerManager->getPlayers();
 FOR_EACH_MAP_FAST(players, i) {
 IPlayerInfo* player = playerinfomanager->GetPlayerInfo(
 const_cast<edict_t*>(players[i]->getEdict()));
 if (player->IsFakeClient()) {
 dynamic_cast<Bot*>(players[i])->setDestination(pos);
 }
 }
 }

 static ConCommand mybot_come("mybot_come", &botComeCmd,
 "Have all bots come to this location");

 void botComeToAreaCmd() {
 auto& players = playerManager->getPlayers();
 Vector pos = getListenClientPos();
 extern CNavMesh* TheNavMesh;
 CNavArea* area = TheNavMesh->GetNavArea(pos);
 if (area == nullptr) {
 area = TheNavMesh->GetNearestNavArea(pos);
 if (area == nullptr) {
 Warning("Unable to get area.\n");
 return;
 }
 }
 FOR_EACH_MAP_FAST(players, i) {
 IPlayerInfo* player = playerinfomanager->GetPlayerInfo(
 const_cast<edict_t*>(players[i]->getEdict()));
 if (player->IsFakeClient()) {
 dynamic_cast<Bot*>(players[i])->setArea(area->GetID());
 }
 }
 }

 static ConCommand mybot_come_area("mybot_come_area", &botComeToAreaCmd,
 "Have all bots come to this nav area");
 **/
