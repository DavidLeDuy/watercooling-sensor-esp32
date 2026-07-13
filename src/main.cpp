#include <Arduino.h>
#include <esp_adc_cal.h>
#include <driver/adc.h>

esp_adc_cal_characteristics_t adc_chars;

const int green = 4;
const int blue = 0;

const int buzzerPin = 32;

const int analogPin = 36; // GPIO36 on ESP32, also labeled as A0

// Thermistor parameters (example values, replace with your thermistor's coefficients)
const float A = 0.0008717193362;
const float B = 0.0002544434452;
const float C = 0.0000001776314062;

const int NUM_SAMPLES = 5;

float calculate_temp(float resistance)
{
  float steinhart = resistance / 10000; // (R/Ro)
  steinhart = log(steinhart);           // ln(R/Ro)
  steinhart /= 3435.0;                  // 1/B * ln(R/Ro)
  steinhart += 1.0 / (25 + 273.15);     // + (1/To)
  steinhart = 1.0 / steinhart;          // Invert
  steinhart -= 273.15;                  // convert absolute temp to C
  return steinhart;
}

void calibrate_ADC()
{
  esp_adc_cal_characterize(
      ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100.0, &adc_chars);
}

float convert_to_mVoltage(int adcValue)
{
  return esp_adc_cal_raw_to_voltage(adcValue, &adc_chars); // since its mV we divide by 1000 to get V
}

float calculate_resistance(float voltage)
{
  return 10000.0 * ((3.3 / voltage) - 1.0);
}

float take_reading()
{
  float average = 0.0;
  for (byte i = 0; i < NUM_SAMPLES; i++)
  {
    average += adc1_get_raw(ADC1_CHANNEL_0);
  }
  average = average / NUM_SAMPLES;
  return convert_to_mVoltage(average);
}

void setup()
{
  Serial.begin(115200);

  // Power-on delay to allow external circuits/capacitors to stabilize
  delay(1000);

  // Keep ADC power switch ON to avoid GPIO36 pull-down glitches (Errata 3.11)
  adc_power_acquire();

  // setup led pins
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  pinMode(buzzerPin, OUTPUT);

  calibrate_ADC();
  adc1_config_width(ADC_WIDTH_BIT_12);
  analogReadResolution(12);                                   // set the analog read resolution to 12 bits (4096 levels)
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); // set the attenutation for channel 0 (A0) to 0dB (the full range of 0v-3.3v
}

void loop()
{
  float mVoltage = take_reading();
  float voltage = mVoltage / 1000.0;

  // Validate sensor reading bounds to prevent division by zero or NaN
  if (voltage < 0.1)
  {
    Serial.println("Error: Sensor Disconnected (Open Circuit)");
  }
  else if (voltage > 3.2)
  {
    Serial.println("Error: Sensor Short Circuit");
  }
  else
  {
    float resistance = calculate_resistance(voltage);

    // Ensure resistance is valid before calculating temperature
    if (resistance > 0.0)
    {
      float tempC = calculate_temp(resistance);
      if (isnan(tempC))
      {
        Serial.println("Error: Calculation NaN");
      }
      else
      {
        Serial.println(tempC);
      }
    }
    else
    {
      Serial.println("Error: Negative Resistance Calculated");
    }
  }

  delay(1000);
}