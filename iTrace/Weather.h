#pragma once

#include <array>
#include "DependenciesMath.h"
#include <vector>
#include <string>

namespace iTrace {


	std::vector<std::string> Weather(std::vector<std::string> Input); 


	const enum class Weather { CLEAR, CLOUDY, OVERCAST, RAINY, THUNDERSTORM, SIZE };

	struct WeatherData {

		float CloudScatteringMultiplier = 1.0; //changes the amount of scattering for the clouds (1.0 = standard cloudy) 
		float CloudAbsorbtionMultiplier = 1.0; //changes the amount of absorption for the clouds (1.0 = standard cloudy) 

		float CloudGlobalPower = 8.0; //Controls the general coverage for the clouds (higher power = less coverage)
		float CloudDetailPower = 5.0; //Controls the detail coverage for the clouds (higher power = less coverage) 
		float CloudNoisePower = 7.0; //Controls the noise coveerage for the clouds (higher power = less coverage)
		
		float VolumetricsScatteringMultiplier = 1.0; 
		float VolumetricsAbsorptionMultiplier = 1.0; 

		float Wetness = 0.0; //the general wetness for the current weather (useful for the block rendering, so that blocks may appear "wet")
		
		float StandardAmbienceSoundGain = 1.0; 
		float RainyAmbienceSoundGain = 0.0; 
		float ThunderStormGain = 0.0; 
		
		WeatherData(float CloudScatteringMultiplier = 1.0, float CloudAbsorbtionMultiplier = 1.0, float CloudGlobalPower = 8.0, float CloudDetailPower = 5.0, float CloudNoisePower = 7.0, 
						float VolumetricsScatteringMultiplier = 1.0, float VolumetricsAbsorptionMultiplier = 1.0, float Wetness = 0.0, 
						float StandardAmbienceSoundGain = 1.0, float RainyAmbienceSoundGain = 0.0, float ThunderStormGain = 0.0) : 
			CloudScatteringMultiplier(CloudScatteringMultiplier), CloudAbsorbtionMultiplier(CloudAbsorbtionMultiplier), CloudGlobalPower(CloudGlobalPower), CloudDetailPower(CloudDetailPower), CloudNoisePower(CloudNoisePower),
			VolumetricsScatteringMultiplier(VolumetricsScatteringMultiplier), VolumetricsAbsorptionMultiplier(VolumetricsAbsorptionMultiplier), Wetness(Wetness),
			StandardAmbienceSoundGain(StandardAmbienceSoundGain), RainyAmbienceSoundGain(RainyAmbienceSoundGain), ThunderStormGain(ThunderStormGain) {
		
		}


	};

	
	struct WeatherManager {

		std::array<WeatherData, static_cast<int>(Weather::SIZE)> Weathers; 

		float CurrentWeatherFactor = 1.0; //0.0 = clear, 4.0 = Thunderstorm. Controlled based on a noise factor 

		void PrepareWeather(); 
		void PollWeather(float t, float ft); 
		WeatherData GetWeather(); 

		WeatherManager() : 
			CurrentWeather(Weathers[4]) {}

	protected: 
		WeatherData CurrentWeather = Weathers[4]; 

	};

	WeatherManager& GetGlobalWeatherManager(); //bite me 


}