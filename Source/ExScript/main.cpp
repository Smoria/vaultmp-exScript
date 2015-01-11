#include "default/vaultscript.h"

#if defined(WIN32) && !defined(__WIN32__)
#define __WIN32__
#endif

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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "SimpleIni.h"

using namespace vaultmp;
using namespace std;

#include "exRecords.h"
#include "logging.h"
#include "WorldSerializer.h"
#include "WeatherManager.h"
#include "default/pickup.hpp"
#include "default/ilview.hpp"
#include "default/cview.hpp"

string ServerPassword;
string ServerName;
string ServerMapName;
string ServerSite;
string ServerMOTD;
float ServerTimeScale;
State ServerConsoleEnabled;
bool ShowPlayersOnlineInRules;

IDHash<ID> cview_map;
IDHash<ID> open_cviews;

WorldSerializer* worldSerializer;
WeatherManager* weatherManager;

Result TimerSaveWorld() noexcept
{
	return worldSerializer->SaveWorldState();
}

Result TimerSavePlayers() noexcept
{
	return worldSerializer->SaveAllPlayersData();
}

Result TimerWeatherUpdate() noexcept
{
	weatherManager->ChangeWeather();
	return Result(True);
}

Result TimerRemoveActor(ID actor) noexcept
{
	DestroyObject(actor);
	KillTimer();
	return Result(True);
}

Result TimerCloseCView(ID player, ID cview) noexcept
{
	constexpr Value distance = 150.0;

	Container container(cview_map[cview]);
	Value X, Y, Z;
	container.GetPos(X, Y, Z);

	if (!container || !IsNearPoint(player, X, Y, Z, distance))
	{
		DestroyWindow(cview);
		KillTimer();
	}

	return Result(True);
}

State ProceedCommand(Player player, int argc, vector<String> argv) noexcept;
string string_replace(string src, string const& target, string const& repl) noexcept;

// Return death message for different reasons
String GetDeathReason(Death d, ID killer) noexcept 
{
	switch (d)
	{
	case Death::None:
		return "died";
	case Death::Explosion:
		return "blew up";
	case Death::Gun:
		return "was shot by " + GetBaseName(killer);
	case Death::BluntWeapon:
		return "was slain by " + GetBaseName(killer);
	case Death::HandToHand:
		return "was torn in half by " + GetBaseName(killer);
	case Death::ObjectImpact:
		return "skull was crushed";
	case Death::Poison:
		return "died from poison";
	case Death::Radiation:
		return "was turned into pile of flesh";
	}
	return "died from unknown reason";
}

// Shows all online players in server rules
void ShowPlayersOnlineSR(ID excludedPlayer) 
{
	String ruleText;
	IDVector players = Player::GetList();
	for (const auto& id : players)
	{
		if (id != excludedPlayer)
		{
			String name = GetBaseName(id);
			ruleText += ", " + name;
		}
	}
	ruleText.erase(0, 2);
	if (ruleText.length() != 0)
		SetServerRule("players", ruleText);
	else
		SetServerRule("players", "-");
}

void Cleanup() noexcept
{
	delete worldSerializer;
	delete weatherManager;
}

Result OnItemPickup(ID item, ID actor) noexcept;
Result OnItemMove(ID cview, ID item, ID destination, ID player) noexcept;

Result FormatItemCView(ID item, char* format) noexcept
{
	Item data(item);
	Actor actor(data.GetItemContainer());
	auto type = data.BaseToType();

	if (!type.compare("WEAP") || !type.compare("ARMO") || !type.compare("ARMA"))
		snprintf(format, IlView::MAX_LENGTH_ITEM, "%dx %s (%d%%)%s", data.GetItemCount(), data.BaseToString().c_str(), static_cast<unsigned int>(data.GetItemCondition()), actor && data.GetItemEquipped() ? " (equipped)" : "");
	else
		snprintf(format, IlView::MAX_LENGTH_ITEM, "%dx %s", data.GetItemCount(), data.BaseToString().c_str());

	return Result(True);
}

State IsCViewOpen(ID player, ID itemlist) noexcept
{
	for (const auto& cview : open_cviews)
	if (cview.second == player)
		if (cview_map[cview.first] == itemlist)
			return True;

	return False;
}

Void OnServerInit() noexcept
{
	MainLog.i("exScript loading...");
	cout << endl << "exScript " EXver << endl;
	cout << "----------------------------------------------------------" << endl;
	SetServerRule("script", "exScript " EXver);

	CSimpleIniA ini;
	SI_Error rc = ini.LoadFile("Settings.ini");
	if (rc < 0)
		MainLog.w("Can't load file Settings.ini, loading default settings...");

	//Name
	ServerName = string_replace(ini.GetValue("settings", "name", "Untitled"), "$exver", EXver);
	//Password
	ServerPassword = ini.GetValue("settings", "password", "");
	//Map
	ServerMapName = ini.GetValue("settings", "map", "Whole world");
	//Site
	ServerSite = ini.GetValue("settings", "website", "vaultmp.com");
	//Message of the day
	ServerMOTD = string_replace(ini.GetValue("settings", "motd", "Welcome to $sname, stranger!"), "$sname", ServerName);
	//Timescale
	ServerTimeScale = atof(ini.GetValue("settings", "timescale", "1.0"));
	//Show players in server rules
	ShowPlayersOnlineInRules = strcmp(ini.GetValue("settings", "showposr", "true"), "true") == 0;
	//Console enabled
	ServerConsoleEnabled = (std::strtoul(ini.GetValue("settings", "console", "1"), nullptr, 10) == 1 ? True : False);

	//Place default items
	bool useDefaultItemLists = strcmp(ini.GetValue("settings", "useitemlists", "false"), "true") == 0;
	//Serialize world type
	int serializationType = std::strtoul(ini.GetValue("settings", "serializeworld", "3"), nullptr, 10);
	
	//Weather type
	WTHRDYNTYPE weatherType = static_cast<WTHRDYNTYPE>(std::strtoul(ini.GetValue("settings", "weather", "350"), nullptr, 10));

	SetServerName(ServerName);
	SetServerMap(ServerMapName);
	SetServerRule("website", ServerSite);
	SetTimeScale(ServerTimeScale);
	SetConsoleEnabled(ServerConsoleEnabled);
	if (ShowPlayersOnlineInRules)
		SetServerRule("players", "-");

	worldSerializer = new WorldSerializer(useDefaultItemLists, serializationType);
	weatherManager = new WeatherManager();

	cout << "Server settings" << endl << endl;

	cout << "Name:\t\t\t"			<< ServerName << endl;
	cout << "Password:\t\t"			<< ServerPassword << endl;
	cout << "Map:\t\t\t"			<< ServerMapName<< endl;
	cout << "Website:\t\t"			<< ServerSite << endl;
	cout << "Timescale:\t\t"		<< ServerTimeScale << endl;

	cout << "Weather:\t\t"			<< (weatherManager->GetDynamicsType() == WTHRDYNTYPE::Random ? "Random" :
										weatherManager->GetDynamicsType() == WTHRDYNTYPE::Listed ? "Dynamic" : "Static") << endl;

	cout << "World serialization:\t"<< (worldSerializer->GetSerializationType() == 0 ? "No" : 
										worldSerializer->GetSerializationType() == 1 ? "Load only" : 
										worldSerializer->GetSerializationType() == 2 ? "Save only" : 
										worldSerializer->GetSerializationType() == 3 ? "Full" : "Unknown") << endl;

	cout << "Show Online:\t\t"		<< (ShowPlayersOnlineInRules ? "Yes" : "No") << endl;
	cout << "Console enabled:\t"	<< (ServerConsoleEnabled ? "Yes" : "No") << endl;
	cout << "Default items:\t\t"	<< (worldSerializer->IsPlacingDefaultItems() ? "Yes" : "No") << endl;

	cout << "MOTD: " << endl << ServerMOTD << endl;

	cout << "----------------------------------------------------------" << endl;

	if (ServerPassword.length() > (int)Index::MAX_PASSWORD_SIZE)
	{
		(MainLog << LOG_ERROR << "Server password too long(max " << (int)Index::MAX_PASSWORD_SIZE << " characters)").end();
		ServerPassword.clear();
	}
	if (ServerMOTD.length() > (int)Index::MAX_MESSAGE_LENGTH)
	{
		(MainLog << LOG_ERROR << "Server MOTD too long(max " << (int)Index::MAX_MESSAGE_LENGTH << " characters)").end();
		ServerMOTD.erase((int)Index::MAX_MESSAGE_LENGTH);
	}

	if (worldSerializer->GetSerializationType() & 1)
		worldSerializer->LoadWorldState();

	if (worldSerializer->GetSerializationType() & 2)
		CreateTimer(&TimerSaveWorld, Interval(1200000));

	CreateTimer(&TimerSavePlayers, Interval(300000));

	weatherManager->ChangeDynamicType(weatherType);

	Pickup::Register(OnItemPickup);

	MainLog.i("exScript loaded");
}

Void OnServerExit() noexcept
{
	MainLog.i("exScript exiting...");

	worldSerializer->SaveAllPlayersData();

	if (worldSerializer->GetSerializationType() & 2)
		worldSerializer->SaveWorldState();

	Cleanup();
}

State OnClientAuthenticate(cRawString name, cRawString pwd) noexcept
{
	if (ServerPassword.length() != 0)
	{
		if (ServerPassword.compare(pwd) != 0)
		{
			(MainLog << LOG_INFO << "Authentication rejected: " << name).end();
			return False;
		}
	}
	return True;
}

Void OnPlayerDisconnect(ID playerID, Reason reason) noexcept
{
	Player player(playerID);

	//Save player data
	worldSerializer->SavePlayerData(player);

	String playerName = player.GetBaseName();
	(MainLog << LOG_INFO << "Player " << playerName << " disconnected (reason:" << reason << " online:" << GetCurrentPlayers() - 1 << ")").end();
	Chat << playerName + "$green left the game";

	// Update server rules
	if (ShowPlayersOnlineInRules) 
		ShowPlayersOnlineSR(playerID);
}

NPC_ OnPlayerRequestGame(ID playerID) noexcept
{
	Player player(playerID);

	String playerName = player.GetBaseName();
	(MainLog << LOG_INFO << "Player " << playerName << " connected (online:" << GetCurrentPlayers() << ")").end();
	Chat << playerName + "$green joined the game. Players online " + to_string(GetCurrentPlayers());

	//Show MOTD to player
	player << string_replace(ServerMOTD, "$name", playerName);

	//Load player data with items
	worldSerializer->LoadPlayerData(player, true);

	// Update server rules
	if (ShowPlayersOnlineInRules)
		ShowPlayersOnlineSR(ID(-1));

	return NPC_(0);
}

State OnPlayerChat(ID playerID, RawString message) noexcept
{
	Player player(playerID);
	if (*message == '/')
	{
		std::vector<String> argv;
		RawString cmd = std::strtok(message + 1, " ");
		while (cmd)
		{
			argv.emplace_back(cmd);
			cmd = std::strtok(nullptr, " ");
		}
		if (!argv.empty())
		{
			return ProceedCommand(player, argv.size(), argv);
		}
		else
		{ 
			return False;
		}
	}
	else
	{
		String msg = player.GetBaseName() + ":$green " + message;           // Build chat message
		std::strncpy(message, msg.c_str(), static_cast<size_t>(Index::MAX_CHAT_LENGTH));
		(ChatLog << message).end();
	}
	return True;
}

Void OnSpawn(ID object) noexcept
{
	if (IsPlayer(object))// Get player data from file
	{
		Player player(object);
		worldSerializer->LoadPlayerData(player, false);
	}
}

Void OnActorDeath(ID actor, ID killer, Limb limbs, Death cause) noexcept
{
	if (IsActor(actor))
	{
		constexpr Interval delete_after_ms = static_cast<Interval>(8000);
		CreateTimerEx(TimerRemoveActor, delete_after_ms, actor);
		if (IsPlayer(actor))
		{
			String name = GetBaseName(actor);
			(MainLog << "PLAYER: " << name << " died (" << (int)cause << ")").end();
			Chat << name + "$green " + GetDeathReason(cause, killer);
		}
	}
}

State ProceedCommand(Player player, int argc, vector<String> argv) noexcept
{
	String playerName = player.GetBaseName();

	// Suicide
	if (!argv[0].compare("kill"))
		player.KillActor();
	
	// Show online players
	else if (!argv[0].compare("players"))
	{
		IDVector players = Player::GetList();
		player << "$yellowPlayers online:";
		for (const auto& id : players)
		{
			player << GetBaseName(id);
		}
	}

	// Save player data
	else if (!argv[0].compare("save"))
	{
		worldSerializer->SavePlayerData(player);
		player << "$yellowYour player data has been saved";
	}

	// Play animation
	else if (!argv[0].compare("anim") || !argv[0].compare("a"))
	{
		string errmsgl1("$redUse /a(anim) <id(1-103)>");
		string errmsgl2("$redor /a(anim) <death/hi/give/take/smoke/heal/sleep/heart/fu>");
		string errmsgl3("$redor /a(anim) <main/chat/warm/wall/weapon/work/tour> <id>");
		string errmsgl4("$redor /a(anim) <cook/ground/sit/bar/wtf> <id>");
		if (argc > 1)
		{
			unsigned int idx = strtoul(argv[1].c_str(), nullptr, 10);
			if (idx == 0)
			{
				IDLE anim = IDLE(0);
				if (!argv[1].compare("death")) anim = SelectedAnimations[0];
				else if (!argv[1].compare("hi")) anim = SelectedAnimations[1];
				else if (!argv[1].compare("give")) anim = SelectedAnimations[2];
				else if (!argv[1].compare("take")) anim = SelectedAnimations[3];
				else if (!argv[1].compare("smoke")) anim = SelectedAnimations[4];
				else if (!argv[1].compare("fu")) anim = SelectedAnimations[5];
				else if (!argv[1].compare("heal")) anim = SelectedAnimations[6];
				else if (!argv[1].compare("heart")) anim = SelectedAnimations[7];
				else if (!argv[1].compare("sleep")) anim = SelectedAnimations[8];
				else
				{
					if (argc < 3)
					{
						player << errmsgl1 << errmsgl2 << errmsgl3 << errmsgl4;
					}
					else
					{
						unsigned int aidx = std::strtoul(argv[2].c_str(), nullptr, 10);
						if (!argv[1].compare("chat"))         anim = SelectedAnimations[9 + aidx % 20];
						else if (!argv[1].compare("combat"))  anim = SelectedAnimations[29 + aidx % 4];
						else if (!argv[1].compare("warm"))    anim = SelectedAnimations[33 + aidx % 3];
						else if (!argv[1].compare("wall"))    anim = SelectedAnimations[36 + aidx % 6];
						else if (!argv[1].compare("weapon"))  anim = SelectedAnimations[42 + aidx % 4];
						else if (!argv[1].compare("work"))    anim = SelectedAnimations[46 + aidx % 8];
						else if (!argv[1].compare("tour"))    anim = SelectedAnimations[54 + aidx % 4];
						else if (!argv[1].compare("cook"))    anim = SelectedAnimations[58 + aidx % 3];
						else if (!argv[1].compare("ground"))  anim = SelectedAnimations[61 + aidx % 10];
						else if (!argv[1].compare("sit"))     anim = SelectedAnimations[71 + aidx % 5];
						else if (!argv[1].compare("bar"))     anim = SelectedAnimations[76 + aidx % 7];
						else if (!argv[1].compare("main"))    anim = SelectedAnimations[83 + aidx % 17];
						else if (!argv[1].compare("wtf"))     anim = SelectedAnimations[100 + aidx % 3];
						else player << errmsgl1 << errmsgl2 << errmsgl3 << errmsgl4;
					}
				}
				if (anim != IDLE(0)) player.PlayIdle(anim); //Play named animation
			}
			else if (idx <= SelectedAnimations.size())                            //Play unnamed animation from SelectedAnimations list
				player.PlayIdle(static_cast<IDLE>(SelectedAnimations[idx - 1]));
			else
				player.PlayIdle(static_cast<IDLE>(idx));               //Play unnamed animation
		}
		else player << errmsgl1 << errmsgl2 << errmsgl3 << errmsgl4;
	}

	// Show player position and rotation
	else if (!argv[0].compare("pos"))
	{
		char msg[(int)Index::MAX_MESSAGE_LENGTH];
		Value X, Y, Z;
		player.GetPos(X, Y, Z);
		snprintf(msg, (int)Index::MAX_MESSAGE_LENGTH, "Position X: %.2f Y: %.2f Z: %.2f Cell: %d", X, Y, Z, player.GetCell());
		player << msg;

		player.GetAngle(X, Y, Z);
		snprintf(msg, (int)Index::MAX_MESSAGE_LENGTH, "Rotation X: %.2f Y: %.2f Z: %.2f", X, Y, Z);
		player << msg;
	}

	// Change weather
	else if (!argv[0].compare("weather"))
	{
		if (argv.size() > 1)
		{
			int wid = std::strtoul(argv[1].c_str(), nullptr, 16);
			weatherManager->ChangeWeather(WTHR(wid));
		}

		char msg[(int)Index::MAX_MESSAGE_LENGTH];
		snprintf(msg, (int)Index::MAX_MESSAGE_LENGTH, "$yellowCurrent weather is %d", GetGameWeather());
		player << msg;

		if (argv.size() == 0)
			player << "$redUse /weather <id>";
	}

	// Get/Set Time
	else if (!argv[0].compare("time"))
	{
		if (argc > 1)
		{
			if (!argv[1].compare("set"))
			{
				if (argc > 2)
				{
					int hour = std::strtoul(argv[2].c_str(), nullptr, 10) + 2;
					if (hour > 23)
						hour = hour % 24;
					SetGameHour(hour);
					player << "$yellowTime set to " + to_string(GetGameHour());
				}
				else
					player << "$redUse /time set <hour(0-23)>";
			}
			else if (!argv[1].compare("day"))
			{
				SetGameHour(12);
				player << "$yellowTime set to " + to_string(GetGameHour());
			}
			else if (!argv[1].compare("night"))
			{
				SetGameHour(0);
				player << "$yellowTime set to " + to_string(GetGameHour());
			}
			else if (!argv[1].compare("morning"))
			{
				SetGameHour(6);
				player << "$yellowTime set to " + to_string(GetGameHour());
			}
			else if (!argv[1].compare("evening"))
			{
				SetGameHour(18);
				player << "$yellowTime set to " + to_string(GetGameHour());
			}
			else if (!argv[1].compare("get"))
				player << "$yellowIt's " + to_string(GetGameHour());
			else
				player << "$redUse /time <get/set/day/night/morning/evening>";
		}
		else
			player << "$redUse /time <get/set/day/night/morning/evening>";
	}

	// Change player Race
	else if (!argv[0].compare("race"))
	{
		if (argc > 1)
		{
			RACE race = static_cast<RACE>(std::strtoul(argv[1].c_str(), nullptr, 16));
			player.SetActorBaseRace(race);
		}
		else
			player << "$redUse /race <id>" << "$yellowCurrent race is " + to_string((int)player.GetActorBaseRace());
	}

	// Change player Sex
	else if (!argv[0].compare("sex"))
		player.SetActorBaseSex(static_cast<Sex>(1 - static_cast<int>(player.GetActorBaseSex())));

	// Change player Age/Young
	else if (!argv[0].compare("age"))
		player.AgeActorBaseRace(1);
	else if (!argv[0].compare("young"))
		player.AgeActorBaseRace(-1);

	// Show all commands
	else
		player << "$redUnknown command: " + argv[0]
		<< "$redCommands:"
		<< "$redsave, weather, time, pos, players"
		<< "$redsex, race, age, young, kill, a(anim)";

	return False;
}

Void OnDestroy(ID reference) noexcept
{
	open_cviews.erase(reference);
	cview_map.erase(reference);
}

Void OnActivate(ID object, ID actor) noexcept
{
	Container container(object);
	Player player(actor);

	if (container && player && container.GetLock() == Lock::Unlocked && !IsCViewOpen(actor, object))
	{
		auto cview = CView::Create(actor, object, OnItemMove, FormatItemCView);
		open_cviews.emplace(cview, actor);
		cview_map.emplace(cview, object);
		player.AttachWindow(cview);

		constexpr Interval interval = static_cast<Interval>(500);
		CreateTimerEx(TimerCloseCView, interval, actor, cview);
	}
}

Result OnItemPickup(ID item, ID actor) noexcept
{
	return static_cast<Result>(True);
}

Result OnItemMove(ID cview, ID item, ID destination, ID player) noexcept
{
	return static_cast<Result>(True);
}

// Return a string with all occurrences of substring target replaced by repl.
string string_replace(string src, string const& target, string const& repl) noexcept
{
	// handle error situations/trivial cases
	if (target.length() == 0)
	{
		// searching for a match to the empty string will result in
		//  an infinite loop
		//  it might make sense to throw an exception for this case
		return src;
	}
	if (src.length() == 0)
	{
		return src;  // nothing to match against
	}
	size_t idx = 0;
	for (;;)
	{
		idx = src.find(target, idx);
		if (idx == string::npos)  break;
		src.replace(idx, target.length(), repl);
		idx += repl.length();
	}
	return src;
}