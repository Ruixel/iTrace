#pragma once

#include <array>

namespace iTrace {

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
		


	};

	


	struct WeatherManager {

		std::array<WeatherData, static_cast<int>(Weather::SIZE)> Weathers; 

		float CurrentWeatherFactor = 1.0; //0.0 = clear, 4.0 = Thunderstorm. Controlled based on a noise factor 

		void PrepareWeather(); 
		void PollWeather(float t); 
		WeatherData GetWeather(); 

	protected: 
		WeatherData CurrentWeather; 

	};

	

}