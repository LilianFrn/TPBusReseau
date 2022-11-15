// Macros for setting rotation to clockwise or anti clockwise
enum {
	MOT_ROT_ACLK = 0,
	MOT_ROT_CLK	= 1,
};

// Command IDs
#define MOT_MAN_ID		0x60
#define MOT_ANG_ID		0x61
#define MOT_RES_ID		0x62

// Size of data sent
#define MOT_MAN_SIZE	3
#define MOT_ANG_SIZE	2
#define MOT_RES_SIZE	0

// Prototypes
int mot_manual(CAN_HandleTypeDef *hcan, uint8_t rot, uint8_t step, uint8_t speed);
int mot_angle(CAN_HandleTypeDef *hcan, uint8_t angle, uint8_t rot);
int mot_reset(CAN_HandleTypeDef *hcan);
