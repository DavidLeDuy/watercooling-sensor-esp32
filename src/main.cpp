#include <Arduino.h>
#include <esp_adc_cal.h>

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
  esp_adc_cal_characteristics_t characteristics;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100.0, &characteristics);
}

float convert_to_mVoltage(int adcValue)
{
  esp_adc_cal_characteristics_t characteristics;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100.0, &characteristics);

  return esp_adc_cal_raw_to_voltage(adcValue, &characteristics); // since its mV we divide by 1000 to get V
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
  // setup led pins
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  pinMode(buzzerPin, OUTPUT);
  adc1_config_width(ADC_WIDTH_BIT_12);
  analogReadResolution(12);                                   // set the analog read resolution to 12 bits (4096 levels)
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); // set the attenutation for channel 0 (A0) to 0dB (the full range of 0v-3.3v
}

void loop()
{

  float mVoltage = take_reading();

  float resistance = calculate_resistance(mVoltage / 1000);

  float tempC = calculate_temp(resistance);

  // Serial.println("Reading: " + String(mVoltage));
  // Serial.println("Thermistor Resistance: " + String(resistance));
  // Serial.println("Temperature (Celsius): " + String(tempC));
  // Serial.println("#######################");
  Serial.println(tempC);

  delay(1000);
}