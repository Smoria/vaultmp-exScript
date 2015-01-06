#ifndef WORLDSERIALIZER_H
#define WORLDSERIALIZER_H

#include "vaultscript.h"
using namespace vaultmp;

#include <fstream>
#include "logging.h"
#include "func.h"

class WorldSerializer
{

private:
	ID lastStdContainer;
	bool placeDefaultItems;

	void init();

public:
	const char* WorldStatePath = "./saves/world.dat";
	const char* ContainersStatePath = "./saves/containers.dat";
	const char* CustomContainersStatePath = "./saves/custom_containers.dat";
	const char* ItemsStatePath = "./saves/items.dat";
	const char* PlayerStatePath = "./saves/players/";	
	
	WorldSerializer() noexcept;
	WorldSerializer(bool placeDefaultItems) noexcept;
	bool IsPlacingDefaultItems() noexcept;
	Result SaveWorldState() noexcept;
	Result LoadWorldState() noexcept;

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
	placeDefaultItems = true;
	init();
	//useItemLists = strcmp(ini.GetValue("settings", "useitemlists", "false"), "true") == 0;	
}

WorldSerializer::WorldSerializer(bool placeDefaultItems) noexcept
{
	this->placeDefaultItems = placeDefaultItems;
	init();
}

bool WorldSerializer::IsPlacingDefaultItems() noexcept
{
	return placeDefaultItems;
}

Result WorldSerializer::SaveWorldState() noexcept
{
	unsigned int cntr = 0, cntr2 = 0;
	FILE * pFile;
	char foutput[128];
	pFile = fopen(ItemsStatePath, "w");
	if (pFile)
	{
		IDVector items = Item::GetList();
		for (const auto& id : items)
		{
			if (GetItemContainer(id) == 0)
			{
				Item item(id);
				UCount count = item.GetItemCount();
				Value condition = item.GetItemCondition();
				Value X, Y, Z, rX, rY, rZ;
				item.GetPos(X, Y, Z);
				item.GetAngle(rX, rY, rZ);
				snprintf(foutput, 128, "%u %u %f %i %f %f %f %f %f %f %u\r\n", item.GetBase(), count, condition, item.GetCell(), X, Y, Z, rX, rY, rZ, item.GetOwner());
				fputs(foutput, pFile);
				cntr++;
			}
		}
		fclose(pFile);
		MainLog << LOG_INFO << "WORLD: " << cntr << " items saved" << std::endl;
	}
	else
	{
		MainLog.e("WORLD: Can't save items");
	}

	pFile = fopen(ContainersStatePath, "w");
	if (pFile)
	{
		cntr = 0;
		IDVector containers = Container::GetList();
		for (const auto& id : containers)
		{
			if (!IsActor(id) && id <= lastStdContainer)
			{
				unsigned int itemCount = GetContainerItemCount(id);
				if (itemCount > 0)
				{
					Value X, Y, Z;
					Container container(id);
					container.GetPos(X, Y, Z);
					snprintf(foutput, 128, "%u %i %f %f %f %u %u %u\r\n", container.GetBase(), container.GetCell(), X, Y, Z, itemCount, container.GetLock(), container.GetOwner());
					fputs(foutput, pFile);
					IDVector items = container.GetContainerItemList();
					for (const auto& itemid : items)
					{
						Item item(itemid);
						snprintf(foutput, 128, "%u %u %f %u\r\n", item.GetBase(), item.GetItemCount(), item.GetItemCondition(), item.GetItemSilent());
						fputs(foutput, pFile);
						cntr2++;
					}
					fputs("\r\n", pFile); // Separator
					cntr++;
				}
			}
		}
		fclose(pFile);
		MainLog << LOG_INFO << "WORLD: " << cntr2 << " items saved in " << cntr << " containers" << std::endl;
	}
	else
	{
		MainLog.e("WORLD: Can't save containers");
	}

	pFile = fopen(CustomContainersStatePath, "w");
	if (pFile)
	{
		cntr = 0;
		cntr2 = 0;
		IDVector containers = Container::GetList();
		for (const auto& id : containers)
		{
			if (!IsActor(id) && id > lastStdContainer)
			{
				Container container(id);
				unsigned int itemCount = GetContainerItemCount(id);
				Value X, Y, Z, rX, rY, rZ;
				container.GetPos(X, Y, Z);
				container.GetAngle(rX, rY, rZ);
				snprintf(foutput, 128, "%u %i %f %f %f %f %f %f %u %u %u\r\n", container.GetBase(), container.GetCell(), X, Y, Z, rX, rY, rZ, itemCount, container.GetLock(), container.GetOwner());
				fputs(foutput, pFile);
				if (itemCount > 0)
				{
					IDVector items = container.GetContainerItemList();
					for (const auto& itemid : items)
					{
						Item item(itemid);
						snprintf(foutput, 128, "%u %u %f %u\r\n", item.GetBase(), item.GetItemCount(), item.GetItemCondition(), item.GetItemSilent());
						fputs(foutput, pFile);
						cntr2++;
					}
					fputs("\r\n", pFile); // Separator
				}
				cntr++;
			}
		}
		fclose(pFile);
		MainLog << LOG_INFO << "WORLD: " << cntr2 << " items saved in " << cntr << " custom containers" << std::endl;
	}
	else
	{
		MainLog.e("WORLD: Can't save custom containers");
	}
	return Result(1);
}

Result WorldSerializer::LoadWorldState() noexcept
{
	unsigned int cntr = 0, cntr2 = 0;
	char buf[128];
	if (FileExists(ItemsStatePath))
	{
		std::ifstream fin(ItemsStatePath);
		fin.getline(buf, 128);
		while (fin.good() && strlen(buf) > 2)
		{
			unsigned int base, count, cell, owner;
			Value condition, X, Y, Z, rX, rY, rZ;
			sscanf(buf, "%u %u %lf %i %lf %lf %lf %lf %lf %lf %u", &base, &count, &condition, &cell, &X, &Y, &Z, &rX, &rY, &rZ, &owner);
			Item item = CreateItem(static_cast<Base>(base), static_cast<CELL>(cell), X, Y, Z);
			item.SetAngle(rX, rY, rZ);
			item.SetOwner(static_cast<NPC_>(owner));
			item.SetItemCount(count);
			item.SetItemCondition(condition);
			fin.getline(buf, 128);
			cntr++;
		}
		fin.close();
		MainLog << LOG_INFO << "WORLD: " << cntr << " items loaded" << std::endl;
	}
	else
	{
		MainLog.e("WORLD: Can't load items");
	}

	if (FileExists(ContainersStatePath))
	{
		cntr = 0;
		std::ifstream fin(ContainersStatePath);
		fin.getline(buf, 128);
		while (fin.good() && strlen(buf) > 2)
		{
			Value X, Y, Z;
			unsigned int cell, base, itemCount, owner, lock;
			sscanf(buf, "%u %i %lf %lf %lf %u %u %u", &base, &cell, &X, &Y, &Z, &itemCount, &lock, &owner);
			IDVector containers = Container::GetList();
			for (const auto& id : containers)
			{
				if (!IsActor(id) && base == GetBase(id) && static_cast<CELL>(cell) == GetCell(id))
				{
					Value cX, cY, cZ;
					GetPos(id, cX, cY, cZ);
					if (abs(X - cX) < 1 && abs(Y - cY) < 1 && abs(Z - cZ) < 1)
					{
						Container container(id);
						container.SetLock(id, static_cast<Lock>(lock));
						container.SetOwner(static_cast<NPC_>(owner));
						cntr++;
						if (itemCount > 0)
						{
							fin.getline(buf, 128);
							while (fin.good() && strlen(buf) > 2)
							{
								UCount count;
								Value condition;
								unsigned int silent;
								sscanf(buf, "%u %u %lf %u", &base, &count, &condition, &silent);
								container.AddItem(static_cast<Base>(base), count, condition, static_cast<State>(silent));
								cntr2++;
								fin.getline(buf, 128);
							}
						}
						fin.getline(buf, 128);
						break;
					}
				}
			}
			fin.getline(buf, 128);
		}
		fin.close();
		MainLog << LOG_INFO << "WORLD: " << cntr2 << " items loaded and placed in " << cntr << " containers" << std::endl;
	}
	else
	{
		MainLog.e("WORLD: Can't load containers");
	}

	if (FileExists(CustomContainersStatePath))
	{
		cntr = 0;
		cntr2 = 0;
		std::ifstream fin(CustomContainersStatePath);
		fin.getline(buf, 128);
		while (fin.good() && strlen(buf) > 2)
		{
			Value X, Y, Z, rX, rY, rZ;
			unsigned int cell, base, itemCount, owner, lock;
			sscanf(buf, "%u %i %lf %lf %lf %lf %lf %lf %u %u %u", &base, &cell, &X, &Y, &Z, &rX, &rY, &rZ, &itemCount, &lock, &owner);
			ID id = CreateContainer(static_cast<CONT>(base), static_cast<CELL>(cell), X, Y, Z);
			Container container(id);
			container.SetAngle(rX, rY, rZ);
			container.SetLock(id, static_cast<Lock>(lock));
			container.SetOwner(static_cast<NPC_>(owner));
			cntr++;
			if (itemCount > 0)
			{
				fin.getline(buf, 128);
				while (fin.good() && strlen(buf) > 2)
				{
					UCount count;
					Value condition;
					unsigned int silent;
					sscanf(buf, "%u %u %lf %u", &base, &count, &condition, &silent);
					container.AddItem(static_cast<Base>(base), count, condition, static_cast<State>(silent));
					cntr2++;
					fin.getline(buf, 128);
				}
			}
			fin.getline(buf, 128);
		}
		fin.close();
		MainLog << LOG_INFO<< "WORLD: " << cntr2 << " items saved and placed in " << cntr << " custom containers" << std::endl;
	}
	else
	{
		MainLog.e("WORLD: Can't load items");
	}
	return Result(1);
}

#endif