#include "Weather.h"
#include "glm-master/glm/gtc/noise.hpp"
#include <iostream>

namespace iTrace {

	WeatherManager GlobalWeatherManager; 

	void WeatherManager::PrepareWeather()
	{
		Weathers[static_cast<int>(Weather::CLEAR)] = WeatherData(
			1.0,1.0, //<- still the same type of clouds
			16.0, //<- the global power is a lot higher, resulting in fewer general clouds 
			10.0, //<- the detail power is also a lot higher, resulting in generally smaller clouds
			7.0, //<- the noise power is the same
			0.333, 0.333 //<- in order to not overpower, the volumetrics intensity is reduced 
			//... everything else stays the same ... 



		);

		//Weather::CLOUDY is skipped because it uses the default params ... 

		Weathers[static_cast<int>(Weather::OVERCAST)] = WeatherData(
			1.0, 1.0, //<- still the same type of clouds 
			4.0, //<- the global power is a lot smaller, resulting in generally more clouds
			3.0, //<- the detail power is also a lot smaller, resulting in generally larger clouds 
			4.0 //<- the noise power is also a lot smaller, resulting in generally thicker clouds 
		); 

		Weathers[static_cast<int>(Weather::RAINY)] = WeatherData(
			1.0, 7.2, //<- the clouds are a LOT darker 
			// ... The cloud shape is the same as overcast weather ... 
			6.0,
			3.0,
			4.0,

			0.5, 10.0, //<- the volumetrics are also a lot darker 

			0.7, //<- the general wetness is no longer 0, as it is raining! 

			0.2, //<- the general volume is a lot more quiet 
			0.75, //<- theres a lot more rain sound 
			0.0 // still no thunder! 

		); 

		Weathers[static_cast<int>(Weather::THUNDERSTORM)] = WeatherData(
			
			0.5, 20.0, //<- the clouds are even darker than the ones seen in the regular rain 

			//They're also the largest ones yet... 
			2.0,
			2.0,
			2.0, 

			0.5,20.0, //<- the volumetrics are also darker 

			1.0,
			
			0.0, //no more general sound 
			1.0, //rain is going at full force! 
			1.0 //now theres thunder! 

		); 

	}
	void WeatherManager::PollWeather(float t)
	{
		CurrentWeatherFactor = glm::simplex(Vector2f(t * 0.005, 1238)) * 2 + 2; 
		CurrentWeatherFactor = 4.0; 
		int MixUnder = floor(CurrentWeatherFactor); 
		int MixOver = MixUnder + 1; 

		MixUnder = glm::clamp(MixUnder, 0, 4); 
		MixOver = glm::clamp(MixOver, 0, 4);

		float MixFactor = glm::fract(CurrentWeatherFactor); 

		auto& a = Weathers[MixUnder];
		auto& b = Weathers[MixOver];
		float c = MixFactor; 

		CurrentWeather.CloudScatteringMultiplier = glm::mix(a.CloudScatteringMultiplier, b.CloudScatteringMultiplier, c); 
		CurrentWeather.CloudAbsorbtionMultiplier = glm::mix(a.CloudAbsorbtionMultiplier, b.CloudAbsorbtionMultiplier, c);

		CurrentWeather.CloudGlobalPower = glm::mix(a.CloudGlobalPower, b.CloudGlobalPower, c);
		CurrentWeather.CloudDetailPower = glm::mix(a.CloudDetailPower, b.CloudDetailPower, c);
		CurrentWeather.CloudNoisePower = glm::mix(a.CloudNoisePower, b.CloudNoisePower, c);

		CurrentWeather.VolumetricsScatteringMultiplier = glm::mix(a.VolumetricsScatteringMultiplier, b.VolumetricsScatteringMultiplier, c);
		CurrentWeather.VolumetricsAbsorptionMultiplier = glm::mix(a.VolumetricsAbsorptionMultiplier, b.VolumetricsAbsorptionMultiplier, c);

		CurrentWeather.Wetness = glm::mix(a.Wetness, b.Wetness, c);

		CurrentWeather.StandardAmbienceSoundGain = glm::mix(a.StandardAmbienceSoundGain, b.StandardAmbienceSoundGain, c);
		CurrentWeather.RainyAmbienceSoundGain = glm::mix(a.RainyAmbienceSoundGain, b.RainyAmbienceSoundGain, c);
		CurrentWeather.ThunderStormGain = glm::mix(a.ThunderStormGain, b.ThunderStormGain, c);

	}
	WeatherData WeatherManager::GetWeather()
	{
		return CurrentWeather;
	}
	WeatherManager& GetGlobalWeatherManager()
	{
		return GlobalWeatherManager; 
	}

}