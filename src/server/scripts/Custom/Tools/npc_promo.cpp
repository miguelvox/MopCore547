/*********************
*  Made by: KuSaNaGy *
*********************/
#include "ScriptPCH.h"
 
class npc_promo : public CreatureScript
 
{
public:
    npc_promo() : CreatureScript("npc_promo") { }
 
    bool OnGossipHello(Player* player, Creature* creature)
    {
      player->ADD_GOSSIP_ITEM(7, "Seleccione tu Promocion: ", GOSSIP_SENDER_MAIN, 999999);
        switch (player->getClass())
        {
		        case CLASS_DRUID: player->ADD_GOSSIP_ITEM(10, "Druida - Feral", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+30); 
			                      player->ADD_GOSSIP_ITEM(10, "Druida - Equlibrio", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3030); break;
                case CLASS_SHAMAN: player->ADD_GOSSIP_ITEM(10, "Chaman - Elemental", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+31);
				                  player->ADD_GOSSIP_ITEM(10, "Chaman - Mejora", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3131); break;
                case CLASS_PALADIN: player->ADD_GOSSIP_ITEM(10, "Promoción - Paladin", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+32); break;
                case CLASS_WARRIOR: player->ADD_GOSSIP_ITEM(10, "Promoción - Guerrero", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+33); break;
                case CLASS_PRIEST: player->ADD_GOSSIP_ITEM(10, "Promoción - Sacerdote", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+34); break;
                case CLASS_DEATH_KNIGHT: player->ADD_GOSSIP_ITEM(10, "Promoción - Caballero de la Muerte", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+35); break;
                case CLASS_ROGUE: player->ADD_GOSSIP_ITEM(10, "Promoción - Picaro", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+36); break;
                case CLASS_HUNTER: player->ADD_GOSSIP_ITEM(10, "Promoción - Cazador", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+37); break;
                case CLASS_MAGE: player->ADD_GOSSIP_ITEM(10, "Promoción - Mago", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+38); break;
                case CLASS_WARLOCK: player->ADD_GOSSIP_ITEM(10, "Promoción - Brujo", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+39); break;
				case CLASS_MONK: player->ADD_GOSSIP_ITEM(10, "Promoción - Monje", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+40); break;
        }
      player->SEND_GOSSIP_MENU(1, creature->GetGUID());
      return true;
    }
 
    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 actions)
    {
        if (player->getLevel() >= 1 && player->getLevel() <= 10)
        {
            uint32 accountID = player->GetSession()->GetAccountId();
            QueryResult result = CharacterDatabase.PQuery("SELECT COUNT(`guid`) FROM `characters` WHERE `account`=%u", accountID);
            Field *fields = result->Fetch();
            uint32 personajes = fields[0].GetUInt32();
 
            if(personajes == 1)    
               {
                        if (actions == 999999)
                        {
                            player->CLOSE_GOSSIP_MENU();
                            return false;
                        }
						creature->MonsterSay("¡Enhorabuena! has obtenido tu promoción ", LANG_UNIVERSAL, 0);
                        player->GiveLevel(90);
                        player->SetMoney(100000000);
						player->AddItem(38082, 1); //bolsa
                        switch(actions)
                          {                              
                                  case GOSSIP_ACTION_INFO_DEF+30: // Druida feral temporada 12
                                  player->AddItem(98790, 1);
                                  player->AddItem(98793, 1);
                                  player->AddItem(98792, 1);
                                  player->AddItem(98789, 1);
                                  player->AddItem(98791, 1);
								  player->AddItem(84890, 1);
								  player->AddItem(84805, 1);
								  player->AddItem(84972, 1);
								  player->AddItem(84947, 1);
								  player->AddItem(84809, 1);
								  player->AddItem(84826, 1);
								  player->AddItem(84827, 1);
								  player->AddItem(85108, 1);  //baston
								  player->AddItem(84935, 1); //abalorio
                                  break;
								  
								  case GOSSIP_ACTION_INFO_DEF+3030: // Druida equilibro temporada 12
                                  player->AddItem(84393, 1);
                                  player->AddItem(84394, 1);
                                  player->AddItem(84395, 1);
                                  player->AddItem(84396, 1);
                                  player->AddItem(84397, 1);
								  player->AddItem(84362, 1);
								  player->AddItem(84382, 1);
								  player->AddItem(84391, 1);
								  player->AddItem(84392, 1);
								  player->AddItem(84365, 1);
								  player->AddItem(84370, 1);
								  player->AddItem(84371, 1);
								  player->AddItem(84787, 1); //baston
                                  break;
 
                                  case GOSSIP_ACTION_INFO_DEF+31: // chaman temporada 12
                                  player->AddItem(84483, 1);
                                  player->AddItem(84484, 1);
                                  player->AddItem(84485, 1);
                                  player->AddItem(84486, 1);
                                  player->AddItem(84487, 1);
								  player->AddItem(84362, 1);
								  player->AddItem(84482, 1);
								  player->AddItem(84468, 1);
								  player->AddItem(84470, 1);
								  player->AddItem(84365, 1);
								  player->AddItem(84371, 1);
								  player->AddItem(84370, 1);
								  player->AddItem(84971, 1); // arma intelecto
								  player->AddItem(84912, 1);//escudo intelecto
                                  break;

								  case GOSSIP_ACTION_INFO_DEF + 3131: // chaman temporada 12
									  player->AddItem(98901, 1);
									  player->AddItem(98903, 1);
									  player->AddItem(98899, 1);
									  player->AddItem(98900, 1);
									  player->AddItem(98902, 1);
									  player->AddItem(85131, 2);
									  player->AddItem(84890, 1);
									  player->AddItem(84804, 1);
									  player->AddItem(84981, 1);
									  player->AddItem(84958, 1);
									  player->AddItem(84817, 1);
									  player->AddItem(84827, 1);
									  player->AddItem(84971, 1); // arma intelecto
									  player->AddItem(108483, 1);//escudo intelecto
									  break;
 
                                  case GOSSIP_ACTION_INFO_DEF+32: // Paladin temporada 12
                                  player->AddItem(84418, 1);
                                  player->AddItem(84419, 1);
                                  player->AddItem(84420, 1);
                                  player->AddItem(84421, 1);
                                  player->AddItem(84422, 1);
								  player->AddItem(84492, 1);
								  player->AddItem(84434, 1);
								  player->AddItem(84437, 1);
								  player->AddItem(84438, 1);
								  player->AddItem(84493, 1);
								  player->AddItem(84497, 1);
								  player->AddItem(84498, 1);
								  player->AddItem(84790, 1); // espada de dos manos
                                  break;
 
                                  case GOSSIP_ACTION_INFO_DEF+33: // guerrero temporada 12
                                  player->AddItem(84504, 1);
                                  player->AddItem(84505, 1);
                                  player->AddItem(84506, 1);
                                  player->AddItem(84507, 1);
                                  player->AddItem(84508, 1);
								  player->AddItem(84492, 1);
								  player->AddItem(84435, 1);
								  player->AddItem(84437, 1);
								  player->AddItem(84439, 1);
								  player->AddItem(84497, 1);
								  player->AddItem(84493, 1);
								  player->AddItem(84498, 1);
								  player->AddItem(84790, 1); // espada de dos manos
                                  break;
 
                                  case GOSSIP_ACTION_INFO_DEF+34: // sacerdote temporada 12
                                  player->AddItem(84445, 1);
                                  player->AddItem(84446, 1);
                                  player->AddItem(84447, 1);
                                  player->AddItem(84448, 1);
                                  player->AddItem(84449, 1);
								  player->AddItem(84363, 1);
								  player->AddItem(84353, 1);
								  player->AddItem(84357, 1);
								  player->AddItem(84360, 1);
								  player->AddItem(84365, 1);
								  player->AddItem(84370, 1);
								  player->AddItem(84371, 1);
								  player->AddItem(84787, 1); //baston
                                  break;
 
                                  case GOSSIP_ACTION_INFO_DEF+35: // DK temporada 12
                                  player->AddItem(84372, 1);
                                  player->AddItem(84373, 1);
                                  player->AddItem(84374, 1);
                                  player->AddItem(84375, 1);
                                  player->AddItem(84376, 1);
								  player->AddItem(84492, 1);
								  player->AddItem(84434, 1);
								  player->AddItem(84436, 1);
								  player->AddItem(84438, 1);
								  player->AddItem(84493, 1);
								  player->AddItem(84497, 1);
								  player->AddItem(84498, 1);
								  player->AddItem(84790, 1); // espada de dos manos
                                  break;
 
                                  case GOSSIP_ACTION_INFO_DEF+36: // Picaro temporada 12
                                  player->AddItem(84462, 1);
                                  player->AddItem(84463, 1);
                                  player->AddItem(84464, 1);
                                  player->AddItem(84465, 1);
                                  player->AddItem(84466, 1);
								  player->AddItem(84345, 1);
								  player->AddItem(84456, 1);
								  player->AddItem(84458, 1);
								  player->AddItem(84460, 1);
								  player->AddItem(84347, 1);
								  player->AddItem(84352, 1);
								  player->AddItem(84351, 1);
								  player->AddItem(84968, 2); //espadas picaros
                                  break;
 
                                  case GOSSIP_ACTION_INFO_DEF+37: // Cazador temporada 12
                                  player->AddItem(84408, 1);
                                  player->AddItem(84409, 1);
                                  player->AddItem(84410, 1);
                                  player->AddItem(84411, 1);
                                  player->AddItem(84412, 1);
								  player->AddItem(84345, 1);
								  player->AddItem(84403, 1);
								  player->AddItem(84405, 1);
								  player->AddItem(84407, 1);
								  player->AddItem(84347, 1);
								  player->AddItem(84351, 1);
								  player->AddItem(84352, 1);
								  player->AddItem(84789, 1); //baston
                                  break;
 
                                  case GOSSIP_ACTION_INFO_DEF+38: // Mago temporada 12
                                  player->AddItem(84413, 1);
                                  player->AddItem(84414, 1);
                                  player->AddItem(84415, 1);
                                  player->AddItem(84416, 1);
                                  player->AddItem(84417, 1);
								  player->AddItem(84363, 1);
								  player->AddItem(84353, 1);
								  player->AddItem(84358, 1);
								  player->AddItem(84360, 1);
								  player->AddItem(84365, 1);
								  player->AddItem(84370, 1);
								  player->AddItem(84371, 1);
								  player->AddItem(84787, 1); //baston intelecto
                                  break;
 
                                  case GOSSIP_ACTION_INFO_DEF+39: // Brujo temporada 12
                                  player->AddItem(84499, 1);
                                  player->AddItem(84500, 1);
                                  player->AddItem(84501, 1);
                                  player->AddItem(84502, 1);
                                  player->AddItem(84503, 1);
								  player->AddItem(84362, 1);
								  player->AddItem(84354, 1);
								  player->AddItem(84357, 1);
								  player->AddItem(84359, 1);
								  player->AddItem(84365, 1);
								  player->AddItem(84370, 1);
								  player->AddItem(84371, 1);
								  player->AddItem(84787, 1); //baston
                                  break;
								  
								  case GOSSIP_ACTION_INFO_DEF+40: // Monje temporada 12
                                  player->AddItem(84543, 1);
                                  player->AddItem(84544, 1);
                                  player->AddItem(84545, 1);
                                  player->AddItem(84546, 1);
                                  player->AddItem(84547, 1);
								  player->AddItem(84345, 1);
								  player->AddItem(84457, 1);
								  player->AddItem(84458, 1);
								  player->AddItem(84461, 1);
								  player->AddItem(84347, 1);
								  player->AddItem(84351, 1);
								  player->AddItem(84352, 1);
								  player->AddItem(84789, 1); //baston
                                  break;
                           }
               }
                   
			else 
                    {
					creature->MonsterSay("La promoción solo es para 1 Personaje por cuenta. ", LANG_UNIVERSAL, 0);
					player->CLOSE_GOSSIP_MENU();
                    return false;
                    }
           
            player->CLOSE_GOSSIP_MENU();
        }
		else 
			{
					creature->MonsterSay("La promoción solo es para 1 Personaje por cuenta. ", LANG_UNIVERSAL, 0);
					player->CLOSE_GOSSIP_MENU();
                    return false;
            }
        return true;
    }
};
 
void AddSC_npc_promo()
{
    new npc_promo();
}