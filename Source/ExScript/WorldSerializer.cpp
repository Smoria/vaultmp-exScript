#include "WorldSerializer.h"


WorldSerializer::WorldSerializer()
{
	LastStdContainer = ID(0);
}

Result WorldSerializer::SaveWorldState()
{
	unsigned int cntr = 0, cntr2 = 0;
	FILE * pFile;
	char foutput[128];
	pFile = fopen("./files/World/Items.dat", "w");
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
		timestamp(); printf("WORLD: %u items saved\n", cntr);
	}
	else
	{
		timestamp(); printf("WORLD: Can't save items\n");
	}

	pFile = fopen("./files/World/Containers.dat", "w");
	if (pFile)
	{
		cntr = 0;
		IDVector containers = Container::GetList();
		for (const auto& id : containers)
		{
			if (!IsActor(id) && id <= LastStdContainer)
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
		timestamp(); printf("WORLD: %u items saved in %u containers\n", cntr2, cntr);
	}
	else
	{
		timestamp(); printf("WORLD: Can't save containers\n");
	}

	pFile = fopen("./files/World/CustomContainers.dat", "w");
	if (pFile)
	{
		cntr = 0;
		cntr2 = 0;
		IDVector containers = Container::GetList();
		for (const auto& id : containers)
		{
			if (!IsActor(id) && id > LastStdContainer)
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
		timestamp(); printf("WORLD: %u custom containers saved with %u items\n", cntr, cntr2);
	}
	else
	{
		timestamp(); printf("WORLD: Can't save custom containers\n");
	}
	return Result(1);
}

Result WorldSerializer::LoadWorldState()
{

	return Result(1);
}