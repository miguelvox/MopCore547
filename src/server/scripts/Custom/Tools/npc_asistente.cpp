#include "ScriptPCH.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "PlayerDump.h"

class player_assistant : public CreatureScript
{
public:
    player_assistant() : CreatureScript("player_assistant") { }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        if (pPlayer->isInCombat())
        {
            pPlayer->GetSession()->SendNotification("Te encuentras en combate");
            return false;
        }
        pPlayer->GetSession()->SendNotification("Bienvenido al asistente");
        pPlayer->ADD_GOSSIP_ITEM(5, "Obtener equipo administrativo", GOSSIP_SENDER_MAIN, 1);
        pPlayer->ADD_GOSSIP_ITEM(5, "Customizar personaje", GOSSIP_SENDER_MAIN, 2);
        pPlayer->ADD_GOSSIP_ITEM(5, "Obtener Maximo nivel", GOSSIP_SENDER_MAIN, 3);
        pPlayer->ADD_GOSSIP_ITEM(5, "Resetear mis puntos de talento", GOSSIP_SENDER_MAIN, 4);
        pPlayer->ADD_GOSSIP_ITEM(5, "Resetear mis spells", GOSSIP_SENDER_MAIN, 5);
        pPlayer->ADD_GOSSIP_ITEM(5, "Resetar mis puntos de honor", GOSSIP_SENDER_MAIN, 6);
        pPlayer->ADD_GOSSIP_ITEM(5, "Resetear mi oro", GOSSIP_SENDER_MAIN, 7);
        pPlayer->ADD_GOSSIP_ITEM(5, "Curarme", GOSSIP_SENDER_MAIN, 8);
        pPlayer->ADD_GOSSIP_ITEM(7, "Cerrar", GOSSIP_SENDER_MAIN, 9);
        pPlayer->SEND_GOSSIP_MENU(907, pCreature->GetGUID());
        return true;
    }
	
    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
    {
        pPlayer->PlayerTalkClass->ClearMenus();

        switch (uiAction)
        {
          case 1:
              {                  
                  pPlayer->CastSpell(pPlayer,84594,false);
				  pPlayer->AddItem(6242, 1);
				  pPlayer->SetHealth(2000000);
                  pPlayer->GetSession()->SendNotification("Has obtenido equipo basico administrativo");
				  pPlayer->CLOSE_GOSSIP_MENU();
              }break;
          case 2:
              {
				PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
				stmt->setUInt16(0, uint16(AT_LOGIN_CUSTOMIZE));	  
				pPlayer->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
                stmt->setUInt32(1, pPlayer->GetGUIDLow());		
                pPlayer->GetSession()->SendNotification("Desconectate para customizar tu personaje");
				pPlayer->CLOSE_GOSSIP_MENU();
              }break;
          case 3:
              {
				  pPlayer->GiveLevel(90);
				  pPlayer->InitTalentForLevel();
                  pPlayer->SetUInt32Value(PLAYER_XP, 0);
                  pPlayer->GetSession()->SendNotification("Has alcanzado el nivel maximo");
				  pPlayer->CLOSE_GOSSIP_MENU();
              }break;
         case 4:
              {
                  pPlayer->CLOSE_GOSSIP_MENU();
                  pPlayer->ResetTalents(true);
                  pPlayer->SetFreeTalentPoints(0);
                  pCreature->MonsterWhisper("Tus talentos se han restaurado.", pPlayer->GetGUID());
              }break;
         case 5:
              {
                  pPlayer->CLOSE_GOSSIP_MENU();
                  pPlayer->resetSpells(false);
                  pCreature->MonsterWhisper("Tus Poderes han sido reseteados.", pPlayer->GetGUID());
              }break;
         case 6:
              {
                  pPlayer->CLOSE_GOSSIP_MENU();
                  //pPlayer->SetHonorPoints(0);
				  pPlayer->ModifyCurrency(CURRENCY_TYPE_HONOR_POINTS, 0);
                  pCreature->MonsterWhisper("Tu honor ha sido reseteado.", pPlayer->GetGUID());
              }break;
         case 7:
              {
                  pPlayer->CLOSE_GOSSIP_MENU();
                  pPlayer->SetMoney(0);
                  pCreature->MonsterWhisper("Tu oro ha sido reseteado.", pPlayer->GetGUID());
              }break;
         case 8:
              {
				  int32 hp = 2000000;    
                  int32 hpm = 2000000;   				  
                  pPlayer->SetHealth(hp);
                  pPlayer->SetMaxHealth(hpm);                
                  pCreature->MonsterWhisper("Has sido curado", pPlayer->GetGUID());
				  pPlayer->CLOSE_GOSSIP_MENU();
              }break;
         case 9:
              {
                  pPlayer->CLOSE_GOSSIP_MENU();
                return true;
              }break;
        }
        pPlayer->CLOSE_GOSSIP_MENU();
        return true;
    }

};

void AddSC_player_assistant()
{
    new player_assistant();
}