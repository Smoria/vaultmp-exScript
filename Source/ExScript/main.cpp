#include "vaultscript.h"

#ifdef __WIN32__
#define EXs "W"
#include "io.h"
#else
#define EXs "U"
#include "sys/io.h"
#include <unistd.h>
#endif

#define EXver "0.2.0" EXs

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "SimpleIni.h"

using namespace vaultmp;

#include "exRecords.h"
#include "logging.h"
#include "func.h"
#include "worldSerializer.h"

std::string serverPass, motd, CHCplayercname, CHCplayercmsg, CHCservermsg;
bool showPOSRule;
unsigned int serializeWorld;
WorldSerializer* worldSerializer;

bool SavePlayerData(ID player) noexcept
{
	String name = GetBaseName(player);
	char filename[64];
	snprintf(filename, 64, "./files/Players/%s.dat", name.c_str());
	FILE * pFile;
	pFile = fopen(filename, "w");
	if (pFile)
	{
		Value X, Y, Z, rX, rY, rZ;
		GetPos(player, X, Y, Z);
		GetAngle(player, rX, rY, rZ);
		char foutput[128];
		snprintf(foutput, 128, "%d %f %f %f %i %i %f %f %f\r\n", GetCell(player), X, Y, Z, GetActorBaseRace(player), GetActorBaseSex(player), rX, rY, rZ);
		fputs(foutput, pFile);
		IDVector items = Item::GetList();
		for (const auto& id : items)
		{
			Item item(id);
			Base base = item.GetBase();
			static const Base PipBoy = static_cast<Base>(0x00015038);
			static const Base PipBoyGloves = static_cast<Base>(0x00025B83);
			if (base != PipBoy && base != PipBoyGloves && item.GetItemContainer() == player)
			{
				snprintf(foutput, 128, "%u %u %f %u %u\r\n", base, item.GetItemCount(), item.GetItemCondition(), item.GetItemEquipped() ? 1 : 0, item.GetItemSilent());
				fputs(foutput, pFile);
			}
		}
		fclose(pFile);
		return true;
	}
	else
	{
		printf("PLAYER_sys: Can't save %s player data\n", name.c_str());
		return false;
	}
}

Result SaveAllPlayers() noexcept
{
	IDVector players = GetList(Type::ID_PLAYER);
	int n = 0;
	for (const auto& id : players)
		if (SavePlayerData(id) == 1) n++;
	timestamp(); printf("PLAYER_sys: Players data has been saved into %d files\n", n);
	return (Result)1;
}

void ShowPlayersOnlineSR(ID player) // Shows all online players in server rules
{
	std::string msg;
	IDVector players = GetList(Type::ID_PLAYER);
	for (const ID& id : players)
		if (id != player)
		{
		String name = GetBaseName(id);
		msg += ", " + name;
		}
	msg.erase(0, 2);
	if (msg.length() != 0) SetServerRule("players", msg);
	else SetServerRule("players", "-");
	return;
}

Result TimerSaveWorld()
{
	return worldSerializer->SaveWorldState();
}

void Cleanup() noexcept
{
	delete worldSerializer;
}

Void VAULTSCRIPT OnServerInit() noexcept
{
	printf("\nEX script %s\n----------------------------------------------------------\n", EXver);
	SetServerRule("script", "EX " EXver);
	worldSerializer = new WorldSerializer(true);

	CSimpleIniA ini;
	SI_Error rc = ini.LoadFile("Settings.ini");
	if (rc < 0) printf("WARNING: Can't load file Settings.ini, loading default settings...\n");

	std::string name = ini.GetValue("settings", "name", "Untitled");
	char fname[name.length() + 16];
	snprintf(fname, name.length() + 16, name.c_str(), EXver);
	SetServerName(fname);

	motd = string_replace(ini.GetValue("settings", "motd", "Welcome to $sname, stranger!"), "$sname", fname);

	std::string map = ini.GetValue("settings", "map", "Whole world");
	SetServerMap(map);

	std::string site = ini.GetValue("settings", "website", "vaultmp.com");
	SetServerRule("website", site);

	showPOSRule = strcmp(ini.GetValue("settings", "showposr", "true"), "true") == 0;
	if (showPOSRule) SetServerRule("players", "-");	

	serializeWorld = std::strtoul(ini.GetValue("settings", "serializeworld", "3"), nullptr, 10);

	serverPass = ini.GetValue("settings", "password", "");

	//WTHRDYNTYPE weather = static_cast<WTHRDYNTYPE>(std::strtoul(ini.GetValue("settings", "weather", "350"), nullptr, 10));
	//WeatherManager::getInstance().ChangeDynamicType(weather);

	float timescale = atof(ini.GetValue("settings", "timescale", "1.0"));
	SetTimeScale(timescale);

	CHCplayercname = ini.GetValue("Chat settings", "name", "99CC00");
	CHCplayercmsg = ini.GetValue("Chat settings", "msg", "33B5E5");
	CHCservermsg = ini.GetValue("Chat settings", "server", "FF4444");

	CHCplayercname = "[colour='FF" + CHCplayercname + "']";
	CHCplayercmsg = "[colour='FF" + CHCplayercmsg + "']";
	CHCservermsg = "[colour='FF" + CHCservermsg + "']";

	unsigned int consoleEnabled = std::strtoul(ini.GetValue("settings", "console", "1"), nullptr, 10);
	SetConsoleEnabled(consoleEnabled == 1 ? True : False);

	printf("Server settings\n\nName:\t\t\t%s\nPassword:\t\t\t%s\nMOTD:\t\t\t%s\nMap:\t\t\t%s\nWebsite:\t\t%s\nTimescale:\t\t%.2f\nWeather:\t\t%s\nWorld serialization:\t%s\nSOPSR:\t\t\t%s\nConsole enabled:\t%s\nDefault items:\t\t%s"
		"\n----------------------------------------------------------\n",
		fname, serverPass.c_str(), motd.c_str(), map.c_str(), site.c_str(), timescale,
		/*weather == WTHRDYNTYPE::Random ? "Random" : weather == WTHRDYNTYPE::Listed ? "Listed" : "Static",*/ "Disabled",
		serializeWorld == 0 ? "No" : serializeWorld == 1 ? "Load only" : serializeWorld == 2 ? "Save only" : serializeWorld == 3 ? "Full" : "Unknown",
		showPOSRule ? "Yes" : "No", consoleEnabled == 1 ? "Yes" : "No", worldSerializer->IsPlacingDefaultItems() ? "Yes" : "No");

	if (serializeWorld & 1)
		worldSerializer->LoadWorldState();
	if (serializeWorld & 2)
		CreateTimer(&TimerSaveWorld, (Interval)1200000);
	CreateTimer(&SaveAllPlayers, (Interval)300000);
}

Void VAULTSCRIPT OnServerExit() noexcept
{
	printf("EX script terminating\n");
	SaveAllPlayers();
	if (serializeWorld & 2) 
		worldSerializer->SaveWorldState();
	Cleanup();
}

State VAULTSCRIPT OnClientAuthenticate(cRawString name, cRawString pwd) noexcept
{
	if (serverPass.length() != 0)
	{
		if (serverPass.compare(pwd) != 0)
		{
			timestamp(); printf("Authentication rejected: %s\n", name);
			return False;
		}
	}
	return True;
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason) noexcept
{
	SavePlayerData(player);
	String name = GetBaseName(player);
	timestamp(); printf("PLAYER: %s disconnected (reason:%d online:%d)\n", name.c_str(), reason, GetCurrentPlayers() - 1);
	Chat << CHCplayercname + name + CHCplayercmsg + " left the game";
	if (showPOSRule) ShowPlayersOnlineSR(player);
}

NPC_ VAULTSCRIPT OnPlayerRequestGame(ID player) noexcept
{
	Player pplayer(player);
	String name = pplayer.GetBaseName();
	timestamp(); printf("PLAYER: %s connected (online:%d)\n", name.c_str(), GetCurrentPlayers());
	pplayer.AddItem(static_cast<Base>(0x0005B6CD), 10, 1);
	char tmpc[64];
	snprintf(tmpc, 64, "%s%s%s joined the game. Players online %d", CHCplayercname.c_str(), name.c_str(), CHCplayercmsg.c_str(), GetCurrentPlayers());
	Chat << tmpc;
	pplayer << string_replace(motd, "$name", name);
	snprintf(tmpc, 64, "./files/Players/%s.dat", name.c_str());
	if (FileExists(tmpc))
	{
		std::ifstream fin(tmpc);
		char buf[128];
		fin.getline(buf, 128);
		fin.getline(buf, 128);
		while (fin.good())
		{
			unsigned int base, count, equipped, silent;
			Value condition;
			sscanf(buf, "%u %u %lf %u %u", &base, &count, &condition, &equipped, &silent);
			pplayer.AddItem(static_cast<Base>(base), (UCount)count, condition, static_cast<State>(silent));
			if ((State)equipped)
			{
				pplayer.EquipItem(static_cast<Base>(base), True, False);
			}
			fin.getline(buf, 128);
		}
		fin.close();
	}
	if (showPOSRule) ShowPlayersOnlineSR((ID)(-1));
	return static_cast<NPC_>(0);
}

State VAULTSCRIPT OnPlayerChat(ID player, RawString message) noexcept
{
	String name = GetBaseName(player);
	if (*message == '/')
	{
		std::vector<String> tokens;
		RawString cmd = std::strtok(message + 1, " ");
		while (cmd)
		{
			tokens.emplace_back(cmd);
			cmd = std::strtok(nullptr, " ");
		}
		if (!tokens.empty())
		{
			Player pplayer(player);
			if (!tokens[0].compare("players"))                              // Show players online
			{
				std::string msg;
				IDVector players = GetList(Type::ID_PLAYER);
				pplayer << CHCservermsg + "Players online:";
				for (const ID& id : players)
				{
					String name = GetBaseName(id);
					if ((msg.length() + 2 + name.length()) >= static_cast<size_t>(Index::MAX_CHAT_LENGTH))
					{
						msg.erase(0, 2);
						pplayer << CHCplayercmsg + msg;
						msg.clear();
					}
					char num[4];
					std::snprintf(num, sizeof(num), "%d", GetConnection(id));
					msg += ", " + name + '[' + String(num) + ']';
				}
				msg.erase(0, 2);
				pplayer << CHCplayercmsg + msg;
			}
			else if (!tokens[0].compare("save"))                            // Save player data
			{
				SavePlayerData(player);
				pplayer << CHCservermsg + "Your player data has been saved";
			}
			else if (!tokens[0].compare("safe"))
			{
				static Base safe = static_cast<Base>(0x000533CE);
				Value X, Y, Z;
				GetPos(player, X, Y, Z);
				CreateContainer(static_cast<CONT>(safe), GetCell(player), X, Y, Z);
			}
			else if (!tokens[0].compare("ad"))
			{
				pplayer.UIMessage("Hello, " + pplayer.GetBaseName() + "!");

				auto ID = ItemList::Create({
						{ ALCH::NukaCola, 100 }, // 100x Nuka Cola
						{ WEAP::Weap32CaliberPistol, 1, 80.0, True, True }, // .32 pistol, 80% health, equipped
						{ AMMO::Ammo32Caliber, 300 }, // 300x .32 ammo
				});

				pplayer.AddItemList(ID);
				DestroyObject(ID);
			}
			else if (!tokens[0].compare("anim") || !tokens[0].compare("a")) // Play animation
			{
				std::string errmsgl1(CHCservermsg + "Use /a(anim) <id(1-103)>");
				std::string errmsgl2(CHCservermsg + "or /a(anim) <death/hi/give/take/smoke/heal/sleep/heart/fu>");
				std::string errmsgl3(CHCservermsg + "or /a(anim) <main/chat/warm/wall/weapon/work/tour> <id>");
				std::string errmsgl4(CHCservermsg + "or /a(anim) <cook/ground/sit/bar/wtf> <id>");
				if (tokens.size() > 1)
				{
					unsigned int idx = std::strtoul(tokens[1].c_str(), nullptr, 10);
					if (idx == 0)
					{
						unsigned int anim = 0;
						if (!tokens[1].compare("death")) anim = mainAnims[0];
						else if (!tokens[1].compare("hi")) anim = mainAnims[1];
						else if (!tokens[1].compare("give")) anim = mainAnims[2];
						else if (!tokens[1].compare("take")) anim = mainAnims[3];
						else if (!tokens[1].compare("smoke")) anim = mainAnims[4];
						else if (!tokens[1].compare("fu")) anim = mainAnims[5];
						else if (!tokens[1].compare("heal")) anim = mainAnims[6];
						else if (!tokens[1].compare("heart")) anim = mainAnims[7];
						else if (!tokens[1].compare("sleep")) anim = mainAnims[8];
						else
						{
							if (tokens.size() < 3) pplayer << errmsgl1 << errmsgl2 << errmsgl3 << errmsgl4;
							else
							{
								unsigned int aidx = std::strtoul(tokens[2].c_str(), nullptr, 10);
								if (!tokens[1].compare("chat"))         anim = mainAnims[9 + aidx % 20];
								else if (!tokens[1].compare("combat"))  anim = mainAnims[29 + aidx % 4];
								else if (!tokens[1].compare("warm"))    anim = mainAnims[33 + aidx % 3];
								else if (!tokens[1].compare("wall"))    anim = mainAnims[36 + aidx % 6];
								else if (!tokens[1].compare("weapon"))  anim = mainAnims[42 + aidx % 4];
								else if (!tokens[1].compare("work"))    anim = mainAnims[46 + aidx % 8];
								else if (!tokens[1].compare("tour"))    anim = mainAnims[54 + aidx % 4];
								else if (!tokens[1].compare("cook"))    anim = mainAnims[58 + aidx % 3];
								else if (!tokens[1].compare("ground"))  anim = mainAnims[61 + aidx % 10];
								else if (!tokens[1].compare("sit"))     anim = mainAnims[71 + aidx % 5];
								else if (!tokens[1].compare("bar"))     anim = mainAnims[76 + aidx % 7];
								else if (!tokens[1].compare("main"))    anim = mainAnims[83 + aidx % 17];
								else if (!tokens[1].compare("wtf"))     anim = mainAnims[100 + aidx % 3];
								else pplayer << errmsgl1 << errmsgl2 << errmsgl3 << errmsgl4;
							}
						}
						if (anim > 0) pplayer.PlayIdle(static_cast<IDLE>(anim)); //Play named animation
					}
					else if (idx <= mainAnims.size())                            //Play unnamed animation from mainAnims list
						pplayer.PlayIdle(static_cast<IDLE>(mainAnims[idx - 1]));
					else
						pplayer.PlayIdle(static_cast<IDLE>(idx));               //Play unnamed animation
				}
				else pplayer << errmsgl1 << errmsgl2 << errmsgl3 << errmsgl4;
			}
			else if (!tokens[0].compare("pos"))                                 // Show player position
			{
				Value X, Y, Z;
				GetPos(player, X, Y, Z);
				char msg[128];
				snprintf(msg, 128,
					"%sPosition X: %.2f Y: %.2f Z: %.2f Cell: %d",
					CHCservermsg.c_str(), X, Y, Z, GetCell(player));
				pplayer << msg;
				GetAngle(player, X, Y, Z);
				snprintf(msg, 128,
					"%sRotation X: %.2f Y: %.2f Z: %.2f",
					CHCservermsg.c_str(), X, Y, Z);
				pplayer << msg;
			}
			/*else if (!tokens[0].compare("weather"))                             // Change weather
			{
			char msg [16];
			if (tokens.size() > 1)
			{
			int wid = std::strtoul(tokens[1].c_str(), nullptr, 16);
			WeatherManager::getInstance().ChangeWeather(static_cast<WTHR>(wid));
			snprintf(msg, 16, "%d", wid);
			pplayer << msg;
			}
			snprintf(msg, 16, "%sWeather id: %d", CHCservermsg.c_str(), GetGameWeather());
			pplayer << msg;
			if (tokens.size() == 0) pplayer << CHCservermsg + "Use /weather <id>";
			}*/
			else if (!tokens[0].compare("time"))
			{
				if (tokens.size() > 1)
				{
					if (!tokens[1].compare("set"))                                  // Change time
					{
						if (tokens.size() > 2) SetGameHour(std::strtoul(tokens[2].c_str(), nullptr, 10));
						else pplayer << CHCservermsg + "Use /time set <hour>";
					}
					else if (!tokens[1].compare("day"))     SetGameHour(12);
					else if (!tokens[1].compare("night"))   SetGameHour(0);
					else if (!tokens[1].compare("morning")) SetGameHour(6);
					else if (!tokens[1].compare("evening")) SetGameHour(18);
				}
				else pplayer << CHCservermsg + "Use /time <set/day/night/morning/evening>";
			}
			else if (!tokens[0].compare("race"))                                    // Change player race
			{
				if (tokens.size() > 1)
				{
					RACE race = static_cast<RACE>(std::strtoul(tokens[1].c_str(), nullptr, 16));
					SetActorBaseRace(player, race);
				}
				else pplayer << CHCservermsg + "Use /race <id>";
			}
			else if (!tokens[0].compare("kill"))                                    // Kill player
			{
				KillActor(player, player);
				timestamp(); printf("PLAYER: %s died (suicide)\n", name.c_str());
				Chat << CHCplayercname + name + CHCplayercmsg + " committed suicide";
			}
			else if (!tokens[0].compare("sex"))                                     // Change player sex
				pplayer.SetActorBaseSex(static_cast<Sex>(1 - static_cast<int>(pplayer.GetActorBaseSex())));
			else if (!tokens[0].compare("age"))
				pplayer.AgeActorBaseRace(1);
			else if (!tokens[0].compare("young"))
				pplayer.AgeActorBaseRace(-1);
			else if (!tokens[0].compare("?") || !tokens[0].compare("help") || !tokens[0].compare("commands"))// Show all commands
				pplayer << CHCservermsg + "Commands:"
				<< CHCservermsg + "?(help, commands), save, weather, time, pos, players"
				<< CHCservermsg + "sex, race, age, young, kill, a(anim)";
			else
				pplayer << CHCservermsg + "Unknown command: " + tokens[0]
				<< CHCservermsg + "Commands:"
				<< CHCservermsg + "?(help, commands), save, weather, time, pos, players" << CHCservermsg + "sex, race, age, young, kill, a(anim)";
		}
		return False;
	}
	else
	{
		String msg = CHCplayercname + name + ": " + CHCplayercmsg + message;           // Build chat message
		std::strncpy(message, msg.c_str(), static_cast<size_t>(Index::MAX_CHAT_LENGTH));
	}
	return True;
}

Void VAULTSCRIPT OnSpawn(ID object) noexcept
{
	if (IsPlayer(object))// Get player data from file
	{
		Player player(object);
		String name = player.GetBaseName();
		char filename[64];
		snprintf(filename, 64, "./files/Players/%s.dat", name.c_str());
		if (FileExists(filename))
		{
			std::ifstream fin(filename);
			char buf[128];
			fin.getline(buf, 128);
			int cell, race, sex;
			Value X, Y, Z, rX, rY, rZ;
			sscanf(buf, "%d %lf %lf %lf %d %d %lf %lf %lf", &cell, &X, &Y, &Z, &race, &sex, &rX, &rY, &rZ);
			player.SetCell(static_cast<CELL>(cell), X, Y, Z);
			player.SetAngle(rX, rY, rZ);
			SetActorBaseRace(object, static_cast<RACE>(race));
			SetActorBaseSex(object, static_cast<Sex>(sex));
			fin.close();
		}
		else
		{
			SetActorBaseRace(object, static_cast<RACE>(25));
			SetActorBaseSex(object, Sex::Male);
		}
	}
}

Void VAULTSCRIPT OnActorDeath(ID actor, ID killer, Limb limbs, Death cause) noexcept
{
	if (IsPlayer(actor))
	{
		String name = GetBaseName(actor);
		timestamp(); printf("PLAYER: %s died (%d)\n", name.c_str(), cause);
		Chat << CHCplayercname + name + CHCplayercmsg + " " + GetDeathReason(cause, killer);
	}
}

/*

Void VAULTSCRIPT OnGameYearChange(UCount year) noexcept
{

}

Void VAULTSCRIPT OnGameMonthChange(UCount month) noexcept
{

}

Void VAULTSCRIPT OnGameDayChange(UCount day) noexcept
{

}

Void VAULTSCRIPT OnGameHourChange(UCount hour) noexcept
{

}

Void VAULTSCRIPT OnCellChange(ID object, VAULTCELL cell) noexcept
{

}

Void VAULTSCRIPT OnContainerItemChange(ID container, Base item, Count count, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorValueChange(ID actor, Index index, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorBaseValueChange(ID actor, Index index, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorAlert(ID actor, State alerted) noexcept
{

}

Void VAULTSCRIPT OnActorSneak(ID actor, State sneaking) noexcept
{

}

Void VAULTSCRIPT OnActorDeath(ID actor, Limb limbs, Death cause) noexcept
{

}

Void VAULTSCRIPT OnActorEquipItem(ID actor, Base item, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorUnequipItem(ID actor, Base item, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorDropItem(ID actor, Base item, UCount count, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorPickupItem(ID actor, Base item, UCount count, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorPunch(ID actor, State power) noexcept
{

}

Void VAULTSCRIPT OnActorFireWeapon(ID actor, VAULTWEAPON weapon) noexcept
{

}

*/
