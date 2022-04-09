/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Chat.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Transmogrification.h"

using namespace Acore::ChatCommands;

class transmog_commandscript : public CommandScript
{
public:
    transmog_commandscript() : CommandScript("transmog_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
		static ChatCommandTable transmogCommandTable =
        {
            { "add",        HandleAddCommand,            SEC_GAMEMASTER,    Console::No },
            { "",           HandleDisableTransMogVisual, SEC_PLAYER,        Console::No }
        };
        
        static ChatCommandTable commandTable =
        {
            { "transmog", transmogCommandTable }
        };

        return commandTable;
    }

    static bool HandleDisableTransMogVisual(ChatHandler* handler, bool hide)
    {
        Player* player = handler->GetPlayer();

        if (hide)
        {
            player->UpdatePlayerSetting("mod-transmog", SETTING_HIDE_TRANSMOG, 0);
            handler->SendSysMessage(LANG_CMD_TRANSMOG_SHOW);
        }
        else
        {
            player->UpdatePlayerSetting("mod-transmog", SETTING_HIDE_TRANSMOG, 1);
            handler->SendSysMessage(LANG_CMD_TRANSMOG_HIDE);
        }

        player->UpdateObjectVisibility();
        return true;
    }
	
	static bool HandleAddCommand(ChatHandler* handler, char const* args)
    {
        Player* me = handler->GetSession()->GetPlayer();
		
		if (!*args)
        {	
			handler->SendSysMessage(LANG_IMPROPER_VALUE);
            handler->SetSentErrorMessage(true);
			ChatHandler(me->GetSession()).PSendSysMessage("Escriba el id del item a enviar");
            return false;
        }
		
		uint32 itemId = 0;
		
		if (args[0] == '[')                                        // [name] manual form
        {
            char const* itemNameStr = strtok((char*)args, "]");

            if (itemNameStr && itemNameStr[0])
            {
                std::string itemName = itemNameStr + 1;

                WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_ITEM_TEMPLATE_BY_NAME);
                //stmt->setString(0, itemName);
                PreparedQueryResult result = WorldDatabase.Query(stmt);

                if (!result)
                {
                    handler->PSendSysMessage(LANG_COMMAND_COULDNOTFIND, itemNameStr + 1);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
                itemId = result->Fetch()->GetUInt32();
            }
            else
                return false;
        }
        else                                                    // item_id or [name] Shift-click form |color|Hitem:item_id:0:0:0|h[name]|h|r
        {
            char const* id = handler->extractKeyFromLink((char*)args, "Hitem");
            if (!id)
                return false;
            itemId = uint32(atol(id));
        }
		
		Player* player = handler->GetSession()->GetPlayer();
        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
            playerTarget = player;

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
        {
            handler->PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }
		
		QueryResult result = CharacterDatabase.Query("SELECT `account_id` = '{}' FROM `custom_unlocked_appearances` WHERE `item_template_id` = '{}'", playerTarget, itemId);
		
		if (player == playerTarget)
		{
			if (result)
			{
				ChatHandler(me->GetSession()).PSendSysMessage("Ya el jugador tiene el Item: |cff4CFF00 %s|r", itemId.c_str());
			}
			
			else
			{				
				CharacterDatabase.Query("INSERT INTO `custom_unlocked_appearances` (`account_id`,`item_template_id`) VALUES ('{},{}')", playerTarget, itemId);
				ChatHandler(me->GetSession()).PSendSysMessage("Agregada: |cff4CFF00 %s|r El Item", itemId);
			}
			
        return true;
		
		}		
		
    }
	
};

void AddSC_transmog_commandscript()
{
    new transmog_commandscript();
}
