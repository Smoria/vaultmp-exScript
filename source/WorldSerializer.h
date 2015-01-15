#ifndef WORLDSERIALIZER_H
#define WORLDSERIALIZER_H

#include "default/vaultscript.h"
using namespace vaultmp;

#include <fstream>
#include "SimpleLogger.h"

class WorldSerializer
{
private:

	struct ItemSaveStruct
	{
		char fileVersion = 1;
		Base base = Base(0);
		CELL cell = CELL(0);
		NPC_ owner = NPC_(0);
		UCount count = 0;
		Value condition = 0;
		Value X = 0;
		Value Y = 0;
		Value Z = 0;
		Value aX = 0;
		Value aY = 0;
		Value aZ = 0;
	};
	struct ContainerItemSaveStruct
	{
		char fileVersion = 1;
		Base base = Base(0);
		NPC_ owner = NPC_(0);
		UCount count = 0;
		Value condition = 0;
		State silent = True;
	};
	struct PlayerItemSaveStruct :ContainerItemSaveStruct
	{
		State equip = False;
		State stick = False;
	};
	struct ContainerSaveStruct
	{
		char fileVersion = 1;
		Base base = Base(0);
		CELL cell = CELL(0);
		NPC_ owner = NPC_(0);
		UCount count = 0;
		Lock lock = Lock::Unlocked;
		Value X = 0;
		Value Y = 0;
		Value Z = 0;
		Value aX = 0;
		Value aY = 0;
		Value aZ = 0;
	};
	struct PlayerSaveStruct
	{
		char fileVersion = 1;
		CELL cell = CELL(0);
		RACE race = RACE::Caucasian;
		Sex sex = Sex::Male;
		Value X = 0;
		Value Y = 0;
		Value Z = 0;
		Value aX = 0;
		Value aY = 0;
		Value aZ = 0;
	};


	ID lastStdContainer;
	bool placeDefaultItems;
	int serializationType;

	void init();

public:
	const char* WorldStatePath = "./saves/world.dat";
	const char* ContainersStatePath = "./saves/containers.dat";
	const char* CustomContainersStatePath = "./saves/custom_containers.dat";
	const char* ItemsStatePath = "./saves/items.dat";
	const char* PlayerStatePath = "./saves/players/";	
	
	WorldSerializer() noexcept;
	WorldSerializer(bool placeDefaultItems, int serializationType) noexcept;

	bool IsPlacingDefaultItems() noexcept;
	int GetSerializationType() noexcept;

	Result SaveWorldState() noexcept;
	Result LoadWorldState() noexcept;
	Result SavePlayerData(Player player) noexcept;
	Result LoadPlayerData(Player player, bool loadItems) noexcept;
	Result SaveAllPlayersData() noexcept;
};

void WorldSerializer::init()
{
	auto containers = Container::GetList();
	if (this->placeDefaultItems)
	{
		MainLog.i("WORLD: Placing default items");
		for (const auto& container : containers)
		{
			AddItemList(container, static_cast<ID>(0));
		}
	}

	lastStdContainer = *(std::max_element(containers.begin(), containers.end())); //Linux suck.
}

WorldSerializer::WorldSerializer() noexcept
{
	placeDefaultItems = false;
	serializationType = 0;
	init();
}

WorldSerializer::WorldSerializer(bool placeDefaultItems, int serializationType) noexcept
{
	this->placeDefaultItems = placeDefaultItems;
	this->serializationType = serializationType;
	init();
}

bool WorldSerializer::IsPlacingDefaultItems() noexcept
{
	return placeDefaultItems;
}

int WorldSerializer::GetSerializationType() noexcept
{
	return serializationType;
}

Result WorldSerializer::SaveWorldState() noexcept
{
	unsigned int savedContainerCount = 0, savedContainerItemsCount = 0, savedItemsCount = 0;
	std::fstream outFile;

	//Save items
	outFile.open(ItemsStatePath, std::fstream::out | std::fstream::binary);
	if (outFile.fail())
	{
		MainLog.e("WORLD: Can't save items");
	}
	else
	{
		IDVector items = Item::GetList();
		for (const auto& id : items)
		{
			if (GetItemContainer(id) == 0) //Save only items without any container(npc, player, etc.)
			{
				Item item(id);
				ItemSaveStruct itemSave;
				itemSave.base = item.GetBase();
				itemSave.cell = item.GetCell();
				itemSave.owner = item.GetOwner();
				itemSave.count = item.GetItemCount();
				itemSave.condition = item.GetItemCondition();
				item.GetPos(itemSave.X, itemSave.Y, itemSave.Z);
				item.GetAngle(itemSave.aX, itemSave.aY, itemSave.aZ);

				outFile.write(reinterpret_cast<char*>(&itemSave), sizeof(itemSave));
				savedItemsCount++;
			}
		}

		outFile.close();
		(MainLog << LOG_INFO << "WORLD: " << savedItemsCount << " items saved").end();
	}

	//Save standart containers
	outFile.open(ContainersStatePath, std::fstream::out | std::fstream::binary);
	if (outFile.fail())
	{
		MainLog.e("WORLD: Can't save containers");
	}
	else
	{
		IDVector containers = Container::GetList();
		for (const auto& id : containers)
		{
			if (!IsActor(id) && id <= lastStdContainer)
			{
				unsigned int containerItemCount = GetContainerItemCount(id);
				if (containerItemCount > 0)	//Save standart containers only with items
				{
					Container container(id);
					ContainerSaveStruct containerSave;

					containerSave.base = container.GetBase();
					containerSave.cell = container.GetCell();
					containerSave.owner = container.GetOwner();
					containerSave.count = containerItemCount;
					containerSave.lock = container.GetLock();
					container.GetPos(containerSave.X, containerSave.Y, containerSave.Z);
					container.GetAngle(containerSave.aX, containerSave.aY, containerSave.aZ);

					outFile.write(reinterpret_cast<char*>(&containerSave), sizeof(containerSave));

					//Save cntainer items
					IDVector items = container.GetContainerItemList();
					for (const auto& itemid : items)
					{
						Item item(itemid);
						ContainerItemSaveStruct itemSave;

						itemSave.base = item.GetBase();
						itemSave.owner = item.GetOwner();
						itemSave.count = item.GetItemCount();
						itemSave.condition = item.GetItemCondition();

						outFile.write(reinterpret_cast<char*>(&itemSave), sizeof(itemSave));

						savedContainerItemsCount++;
					}

					//Separate container from others by empty item
					ContainerItemSaveStruct itemSaveSep;
					outFile.write(reinterpret_cast<char*>(&itemSaveSep), sizeof(itemSaveSep));

					savedContainerCount++;
				}
			}
		}

		outFile.close();
		(MainLog << LOG_INFO << "WORLD: " << savedContainerCount << " containers saved with " << savedContainerItemsCount << " items").end();
	}

	//Save custom placed containers
	outFile.open(CustomContainersStatePath, std::fstream::out | std::fstream::binary);
	if (outFile.fail())
	{
		MainLog.e("WORLD: Can't save custom containers");
	}
	else
	{
		savedContainerCount = 0;
		savedContainerItemsCount = 0;
		IDVector containers = Container::GetList();
		for (const auto& id : containers)
		{
			if (!IsActor(id) && id > lastStdContainer)
			{
				unsigned int containerItemCount = GetContainerItemCount(id);

				Container container(id);
				ContainerSaveStruct containerSave;
				containerSave.base = container.GetBase();
				containerSave.cell = container.GetCell();
				containerSave.owner = container.GetOwner();
				containerSave.count = containerItemCount;
				containerSave.lock = container.GetLock();
				container.GetPos(containerSave.X, containerSave.Y, containerSave.Z);
				container.GetAngle(containerSave.aX, containerSave.aY, containerSave.aZ);
				outFile.write(reinterpret_cast<char*>(&containerSave), sizeof(containerSave));

				IDVector items = container.GetContainerItemList();
				for (const auto& itemid : items)
				{
					Item item(itemid);
					ContainerItemSaveStruct itemSave;

					itemSave.base = item.GetBase();
					itemSave.owner = item.GetOwner();
					itemSave.count = item.GetItemCount();
					itemSave.condition = item.GetItemCondition();

					outFile.write(reinterpret_cast<char*>(&itemSave), sizeof(itemSave));

					savedContainerItemsCount++;
				}

				//Separate container from others by empty item
				ContainerItemSaveStruct itemSaveSep;
				outFile.write(reinterpret_cast<char*>(&itemSaveSep), sizeof(itemSaveSep));

				savedContainerCount++;
			}
		}

		outFile.close();
		(MainLog << LOG_INFO << "WORLD: " << savedContainerCount << " custom containers saved with " << savedContainerItemsCount << " items").end();
	}

	return Result(1);
}

Result WorldSerializer::LoadWorldState() noexcept
{
	unsigned int loadedContainerCount = 0, loadedContainerItemsCount = 0, loadedItemsCount = 0;

	//Load items
	std::ifstream inFile(ItemsStatePath);
	if (inFile.good())
	{
		while (inFile.good())
		{
			ItemSaveStruct itemSave;
			inFile.read(reinterpret_cast<char*>(&itemSave), sizeof(itemSave));
			if (itemSave.base != 0)
			{
				Item item = Item::Create(itemSave.base, itemSave.cell, itemSave.X, itemSave.Y, itemSave.Z);
				item.SetAngle(itemSave.aX, itemSave.aY, itemSave.aZ);
				item.SetOwner(itemSave.owner);
				item.SetItemCount(itemSave.count);
				item.SetItemCondition(itemSave.condition);

				loadedItemsCount++;
			}
		}

		inFile.close();
		(MainLog << LOG_INFO << "WORLD: " << loadedItemsCount << " items loaded").end();
	}
	else
	{
		MainLog.e("WORLD: Can't load items");
	}
	
	//Load standart containers
	inFile.open(ContainersStatePath);
	if (inFile.good())
	{
		double stdContainerPositionCheckError = 1;
		while (inFile.good())
		{
			ContainerSaveStruct containerSave;
			inFile.read(reinterpret_cast<char*>(&containerSave), sizeof(containerSave));

			if (containerSave.base != 0)
			{
				//For each standart container in world
				IDVector containers = Container::GetList();
				for (const auto& id : containers)
				{
					//If container has same base and cell
					if (!IsActor(id) && containerSave.base == GetBase(id) && containerSave.cell == GetCell(id))
					{
						Value cX, cY, cZ;
						GetPos(id, cX, cY, cZ);
						//If container has same position
						if (abs(containerSave.X - cX) < stdContainerPositionCheckError && abs(containerSave.Y - cY) < stdContainerPositionCheckError && abs(containerSave.Z - cZ) < stdContainerPositionCheckError)
						{
							Container container(id);
							container.SetLock(id, containerSave.lock);
							container.SetOwner(containerSave.owner);
							while (inFile.good())
							{
								ContainerItemSaveStruct itemSave;
								inFile.read(reinterpret_cast<char*>(&itemSave), sizeof(itemSave));
								if (itemSave.base != 0)
								{
									container.AddItem(itemSave.base, itemSave.count, itemSave.condition, itemSave.silent);
									loadedContainerItemsCount++;
								}
								else
								{
									break;	//We reached separator item
								}
							}

							loadedContainerCount++;
							break;	// We found our container, stop searching
						}
					}
				}
			}
		}

		inFile.close();
		(MainLog << LOG_INFO << "WORLD: " << loadedContainerCount << " containers loaded with " << loadedContainerItemsCount << " items").end();
	}
	else
	{
		MainLog.e("WORLD: Can't load containers");
	}

	//Load custom placed containers
	inFile.open(CustomContainersStatePath);
	if (inFile.good())
	{
		loadedContainerCount = 0;
		loadedContainerItemsCount = 0;

		while (inFile.good())
		{
			ContainerSaveStruct containerSave;
			inFile.read(reinterpret_cast<char*>(&containerSave), sizeof(containerSave));

			if (containerSave.base != 0)
			{
				Container container = CreateContainer(static_cast<CONT>(containerSave.base), containerSave.cell, containerSave.X, containerSave.Y, containerSave.Z);

				container.SetAngle(containerSave.aX, containerSave.aY, containerSave.aZ);
				container.SetLock(ID(0), containerSave.lock);
				container.SetOwner(containerSave.owner);
				while (inFile.good())
				{
					ContainerItemSaveStruct itemSave;
					inFile.read(reinterpret_cast<char*>(&itemSave), sizeof(itemSave));
					if (itemSave.base != 0)
					{
						container.AddItem(itemSave.base, itemSave.count, itemSave.condition, itemSave.silent);
						loadedContainerItemsCount++;
					}
					else
					{
						break;	//We reached separator item
					}
				}

				loadedContainerCount++;
			}
		}

		inFile.close();
		(MainLog << LOG_INFO << "WORLD: " << loadedContainerCount << " custom containers loaded with " << loadedContainerItemsCount << " items").end();
	}
	else
	{
		MainLog.e("WORLD: Can't load custom containers");
	}

	return Result(1);
}

Result WorldSerializer::SavePlayerData(Player player) noexcept
{
	String filename(PlayerStatePath);
	String playerName = player.GetBaseName();
	filename += playerName + ".dat";

	std::fstream outFile(filename, std::fstream::out | std::fstream::binary);

	if (outFile.fail())
	{
		(MainLog << LOG_ERROR << "WORLD: Can't save player data for " << playerName).end();
		return Result(0);
	}
	else
	{
		PlayerSaveStruct playerSave;

		playerSave.cell = player.GetCell();
		playerSave.race = player.GetActorBaseRace();
		playerSave.sex = player.GetActorBaseSex();
		player.GetPos(playerSave.X, playerSave.Y, playerSave.Z);
		player.GetAngle(playerSave.aX, playerSave.aY, playerSave.aZ);

		outFile.write(reinterpret_cast<char*>(&playerSave), sizeof(playerSave));

		IDVector items = player.GetContainerItemList();
		for (const auto& itemid : items)
		{
			Item item(itemid);
			Base itemBase = item.GetBase();
			static const Base PipBoy = static_cast<Base>(ARMO::PipBoy);
			static const Base PipBoyGlove = static_cast<Base>(ARMO::PipBoyGlove);
			if (itemBase != PipBoy && itemBase != PipBoyGlove)
			{
				PlayerItemSaveStruct itemSave;

				itemSave.base = itemBase;
				itemSave.owner = item.GetOwner();
				itemSave.count = item.GetItemCount();
				itemSave.condition = item.GetItemCondition();
				itemSave.equip = item.GetItemEquipped();
				itemSave.silent = item.GetItemSilent();
				itemSave.stick = item.GetItemStick();

				outFile.write(reinterpret_cast<char*>(&itemSave), sizeof(itemSave));
			}
		}
		outFile.close();
	}

	return Result(1);
}

Result WorldSerializer::LoadPlayerData(Player player, bool loadItems) noexcept
{
	String filename(PlayerStatePath);
	String playerName = player.GetBaseName();
	filename += playerName + ".dat";

	std::ifstream inFile(filename);
	if (inFile.good())
	{
		while (inFile.good())
		{
			PlayerSaveStruct playerSave;
			inFile.read(reinterpret_cast<char*>(&playerSave), sizeof(playerSave));

			player.SetCell(playerSave.cell, playerSave.X, playerSave.Y, playerSave.Z);
			player.SetActorBaseRace(playerSave.race);
			player.SetActorBaseSex(playerSave.sex);
			player.SetAngle(playerSave.aX, playerSave.aY, playerSave.aZ);

			while (loadItems && inFile.good())
			{
				PlayerItemSaveStruct itemSave;
				inFile.read(reinterpret_cast<char*>(&itemSave), sizeof(itemSave));
				if (itemSave.base != 0)
				{
					if (itemSave.equip == True)
					{
						player.EquipItem(itemSave.base, itemSave.silent, itemSave.stick);
					}
					else
					{
						player.AddItem(itemSave.base, itemSave.count, itemSave.condition, itemSave.silent);
					}
				}
			}
		}
		inFile.close();
		return Result(1);
	}
	else
	{
		player.SetActorBaseRace(RACE::Caucasian);
		player.SetActorBaseSex(Sex::Male);
		return Result(0);
	}
}

Result WorldSerializer::SaveAllPlayersData() noexcept
{
	IDVector players = GetList(Type::ID_PLAYER);
	int n = 0;
	for (const auto& id : players)
		if (SavePlayerData(id) == 1)
			n++;

	(MainLog << LOG_INFO << "WORLD: Players data has been saved into " << n << " files").end();

	return Result(n);
}

#endif