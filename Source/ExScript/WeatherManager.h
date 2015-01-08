#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

#include <cstdlib>
#include <cstdio>
#include "vaultscript.h"

using namespace vaultmp;

#include "exRecords.h"
#include "logging.h"

Result TimerWeatherUpdate() noexcept;

enum VAULTCPP(class) WTHRDYNTYPE VAULTCPP(: int8_t)
{
	Random = -1,
	Listed = 0,
	Static = 1,
};


class WeatherManager
{
private:
	WTHRDYNTYPE dynamicsType;
	uint16_t listedWeatherId;
	Timer updateTimer;

public:
	WeatherManager();
	~WeatherManager();

	void ChangeDynamicType(WTHRDYNTYPE dynamicsType);
	void ChangeWeather();
	void ChangeWeather(WTHR id);
	WTHRDYNTYPE GetDynamicsType();
	WTHR GetWeather();
};

WeatherManager::WeatherManager()
{
	dynamicsType = WTHRDYNTYPE::Random;
	listedWeatherId = 0;
	updateTimer = Timer(-1);
}

WeatherManager::~WeatherManager()
{
	KillTimer(updateTimer);
}

WTHRDYNTYPE WeatherManager::GetDynamicsType()
{
	return dynamicsType;
}

WTHR WeatherManager::GetWeather()
{
	return GetGameWeather();
}

void WeatherManager::ChangeDynamicType(WTHRDYNTYPE dynamicsType)
{
	if (dynamicsType < WTHRDYNTYPE::Random || dynamicsType > WTHRDYNTYPE::Static)
		dynamicsType = WTHRDYNTYPE::Random;
	else
		dynamicsType = dynamicsType;

	if (updateTimer != Timer(-1))
	{
		KillTimer(updateTimer);
		updateTimer = Timer(-1);
	}

	switch (dynamicsType)
	{
	case WTHRDYNTYPE::Random:
		updateTimer = CreateTimer(&TimerWeatherUpdate, Interval(300000));
		break;

	case WTHRDYNTYPE::Listed:
		updateTimer = CreateTimer(&TimerWeatherUpdate, Interval(600000));
		break;

	default:
		if (dynamicsType > WTHRDYNTYPE::Static)
			SetGameWeather(WTHR::DefaultWeather);
		else
			SetGameWeather(WTHR(dynamicsType));
		break;
	}

	(MainLog << LOG_INFO << "Weather dynamic type changed to " << (dynamicsType == WTHRDYNTYPE::Random ? "Random" : (dynamicsType == WTHRDYNTYPE::Listed ? "Listed" : "Static"))).end();
}

void WeatherManager::ChangeWeather()
{
	WTHR nw;
	switch (dynamicsType)
	{
	case WTHRDYNTYPE::Random:
		nw = RandomWeatherList.at(rand() % RandomWeatherList.size());
		SetGameWeather(nw);
		break;

	case WTHRDYNTYPE::Listed:
		nw = DynamicWeatherList.at(listedWeatherId++);
		SetGameWeather(nw);
		if (listedWeatherId >= DynamicWeatherList.size())
			listedWeatherId = 0;
		break;

	default:
		if (updateTimer != Timer(-1))
		{
			KillTimer(updateTimer);
			updateTimer = Timer(-1);
		}
		SetGameWeather(WTHR::DefaultWeather);
		nw = WTHR::DefaultWeather;
		break;
	}
	(MainLog << LOG_INFO << "Weather changed to " << (int)nw).end();
}

void WeatherManager::ChangeWeather(WTHR id)
{
	dynamicsType = WTHRDYNTYPE::Static;
	SetGameWeather(id);

	(MainLog << LOG_INFO << "Weather changed to " << (int)id).end();
}


#endif // WEATHERMANAGER_H