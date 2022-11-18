#include "capt_api.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;

static int32_t t_fine = 0;

// Flag to change motor rotation
int temp_flag = 0;

// Calibration factors
static unsigned short dig_T1;
static signed short dig_T2;
static signed short dig_T3;
static unsigned short dig_P1;
static signed short dig_P2;
static signed short dig_P3;
static signed short dig_P4;
static signed short dig_P5;
static signed short dig_P6;
static signed short dig_P7;
static signed short dig_P8;
static signed short dig_P9;

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
int32_t bmp280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) -((int32_t)dig_T1)) * ((adc_T>>4) - \
			((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t bmp280_compensate_P_int64(int32_t adc_P)
{
	int64_t var1, var2, p;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)dig_P6;
	var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
	var2 = var2 + (((int64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;
	if (var1 == 0){
		return 0; // avoid exception caused by division by zero
	}
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);
	return ((uint32_t)p/256);
}

// Check if peripheral is online and working
void capt_ping(void){
	uint8_t id_get_snd = BMP280_ID_REG;
	uint8_t id_get_rcv = 0;

	HAL_I2C_Master_Transmit(&hi2c1, BMP280_SLV_ADD, &id_get_snd, BMP280_ID_SND_SIZE, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, BMP280_SLV_ADD, &id_get_rcv, BMP280_ID_RCV_SIZE, HAL_MAX_DELAY);
	if (id_get_rcv == BMP280_ID_EXP){
	  printf("Slave ID is 0x%x : Correct\r\n", id_get_rcv);
	}
	else {
	  printf("ERROR : Slave ID is 0x%x : Incorrect\r\n", id_get_rcv);
	}
}

// Sets configuration for the peripheral and gets its calibration data
// Sets the calibration data to the needed factors for Temp and Pres conversion
// This function HAS to be called at launch
void capt_param(void){
	uint8_t conf_set_snd[2] = {BMP280_CONF_REG, BMP280_CONF_SET};
	uint8_t conf_set_rcv = 0;
	uint8_t etal_rcv[26];

	HAL_I2C_Master_Transmit(&hi2c1, BMP280_SLV_ADD, conf_set_snd, BMP280_CONF_SND_SIZE, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, BMP280_SLV_ADD, &conf_set_rcv, BMP280_CONF_RCV_SIZE, HAL_MAX_DELAY);
	if (conf_set_rcv == BMP280_CONF_SET){
	  printf("Slave configuration set to desired parameters\r\n");
	}
	else {
	  printf("ERROR : Slave configuration failed\r\n");
	}
	HAL_I2C_Mem_Read(&hi2c1, BMP280_SLV_ADD, BMP280_ETAL_REG, 1,
			etal_rcv, BMP280_ETAL_RCV_SIZE, HAL_MAX_DELAY);

	dig_T1 = (etal_rcv[1]<<8) + etal_rcv[0];
	dig_T2 = (etal_rcv[3]<<8) + etal_rcv[2];
	dig_T3 = (etal_rcv[5]<<8) + etal_rcv[4];
	dig_P1 = (etal_rcv[7]<<8) + etal_rcv[6];
	dig_P2 = (etal_rcv[9]<<8) + etal_rcv[8];
	dig_P3 = (etal_rcv[11]<<8) + etal_rcv[10];
	dig_P4 = (etal_rcv[13]<<8) + etal_rcv[12];
	dig_P5 = (etal_rcv[15]<<8) + etal_rcv[14];
	dig_P6 = (etal_rcv[16]<<8) + etal_rcv[16];
	dig_P7 = (etal_rcv[19]<<8) + etal_rcv[18];
	dig_P8 = (etal_rcv[21]<<8) + etal_rcv[20];
	dig_P9 = (etal_rcv[23]<<8) + etal_rcv[22];
}

// API to get temperature, always has to be called at least once before any pres call
float capt_temp(void){
	float temp_comp;
	uint8_t temp_hex[3];
	int32_t temp;

	HAL_I2C_Mem_Read(&hi2c1, BMP280_SLV_ADD, BMP280_TEMP_REG, 1,
		  temp_hex, BMP280_TEMP_RCV_SIZE, HAL_MAX_DELAY),
	temp = (temp_hex[0]<<12) + (temp_hex[1]<<4) + (temp_hex[0]>>4);
	temp_comp = (float)bmp280_compensate_T_int32(temp)/100;
	printf("%f°C\r\n", temp_comp);
	temp_flag = 1;
	return(temp_comp);
}

// API to get pressure, t_fine needs to hvae been set
float capt_pres(void){
	float pres_comp;
	uint8_t pres_hex[3];
	int32_t pres;

	HAL_I2C_Mem_Read(&hi2c1, BMP280_SLV_ADD, BMP280_PRES_REG, 1,
		  pres_hex, BMP280_PRES_RCV_SIZE, HAL_MAX_DELAY),
	pres = (pres_hex[0]<<12) + (pres_hex[1]<<4) + (pres_hex[0]>>4);
	pres_comp = (float)bmp280_compensate_P_int64(pres)/3600;
	printf("%fhPa\r\n", pres_comp);
	return(pres_comp);
}


