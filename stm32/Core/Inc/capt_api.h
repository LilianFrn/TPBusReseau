// Salve Address
#define BMP280_SLV_ADD			(0x77 << 1)

// Macros for ping command
#define BMP280_ID_REG			0xD0
#define BMP280_ID_EXP			0x58
#define BMP280_ID_SND_SIZE		1
#define BMP280_ID_RCV_SIZE		1

// Macros for param command
#define BMP280_CONF_REG			0xF4
#define BMP280_CONF_SET			0b01010111
#define BMP280_CONF_SND_SIZE	2
#define BMP280_CONF_RCV_SIZE	1

#define BMP280_ETAL_REG			0x88
#define BMP280_ETAL_RCV_SIZE	26

// MAcros for pres command
#define BMP280_PRES_REG			0xF7
#define BMP280_PRES_RCV_SIZE	3

// Macros for temp command
#define BMP280_TEMP_REG			0xFA
#define BMP280_TEMP_RCV_SIZE	3

// Prototypes
void capt_ping(void);
void capt_param(void);
float capt_pres(void);
float capt_temp(void);
