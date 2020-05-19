#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "Object.h"
#include "Player.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "Unit.h"
#include "UnitAI.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Spell.h"
#include "timeless_isle.h"


enum TimeLostShrine
{
    // Says
    SAY_BLESSING_NIUZAO                 = 0,
    SAY_BLESSING_YULON                  = 1,
    SAY_BLESSING_CHI_JI                 = 2,
    SAY_BLESSING_XUEN                   = 3,
    SAY_CORRUPTION_ORDOS                = 4,

    // Spells
    SPELL_FORTITUDE_OF_NIUZAO           = 147281,
    SPELL_WISDOM_OF_YULON               = 147282,
    SPELL_CHI_JIS_HOPE                  = 147283,
    SPELL_XUENS_STRENGTH                = 147284,
    SPELL_ORDOS_BURNING_SACRIFICE       = 147285,
};

class go_time_lost_shrine_ti : public GameObjectScript
{
public:
    go_time_lost_shrine_ti() : GameObjectScript("go_time_lost_shrine_ti") { }

    struct go_time_lost_shrine_tiAI : public GameObjectAI
    {
        go_time_lost_shrine_tiAI(GameObject* go) : GameObjectAI(go) { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
        	/*if (used == false)
        	{
                go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
            	used = true;
				printf("Utilizo el scrip de la piedra");
        		Choice = urand(1, 5);
        	}*/

        	if (player->HasAura(SPELL_FORTITUDE_OF_NIUZAO))
        		player->RemoveAurasDueToSpell(SPELL_FORTITUDE_OF_NIUZAO);

        	if (player->HasAura(SPELL_WISDOM_OF_YULON))
            	player->RemoveAurasDueToSpell(SPELL_WISDOM_OF_YULON);

        	if (player->HasAura(SPELL_CHI_JIS_HOPE))
            	player->RemoveAurasDueToSpell(SPELL_CHI_JIS_HOPE);

        	if (player->HasAura(SPELL_XUENS_STRENGTH))
            	player->RemoveAurasDueToSpell(SPELL_XUENS_STRENGTH);

        	Creature* trigger = go->FindNearestCreature(NPC_TIME_LOST_SHRINE_TRIGGER, 5.0f); // An npc needs to be spawned in the same coordinates for the shrine
			printf("Utilizo el scrip de la piedra");

            return true;
        }

		void UpdateAI(uint32 diff)
        {
            if (used == false)
                return;

            if (BackToUse <= diff)
            {
                go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                used = false; // this will stop/prevent it from updating it
            }

            else
            	BackToUse -= diff;
        }

    private:
    	uint32 BackToUse;

    	uint8 Choice;

    	bool used;
    };

	GameObjectAI* GetAI(GameObject* go) const 
    {
        return new go_time_lost_shrine_tiAI(go);
    }
};



void AddSC_timeless_spell()
{
    new go_time_lost_shrine_ti();
}