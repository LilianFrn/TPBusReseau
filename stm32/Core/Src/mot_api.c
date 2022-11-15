#include "main.h"
#include "mot_api.h"

static CAN_TxHeaderTypeDef header = {
	.StdId = 0,
	.ExtId = 0,
	.IDE = CAN_ID_STD,
	.RTR = CAN_RTR_DATA,
	.DLC = 0,
	.TransmitGlobalTime = DISABLE
};

static uint32_t pTxMailbox;

int mot_manual(CAN_HandleTypeDef *hcan, uint8_t rot, uint8_t step, uint8_t speed){
	uint8_t data[MOT_MAN_SIZE] = {rot, step, speed};
	header.StdId = MOT_MAN_ID;
	header.DLC = MOT_MAN_SIZE;

	return HAL_CAN_AddTxMessage(hcan, &header, data, &pTxMailbox);
}

int mot_angle(CAN_HandleTypeDef *hcan, uint8_t angle, uint8_t rot){
	uint8_t data[MOT_ANG_SIZE] = {angle, rot};
	header.StdId = MOT_ANG_ID;
	header.DLC = MOT_ANG_SIZE;

	return HAL_CAN_AddTxMessage(hcan, &header, data, &pTxMailbox);
}

int mot_reset(CAN_HandleTypeDef *hcan){
	header.StdId = MOT_RES_ID;
	header.DLC = MOT_RES_SIZE;

	return HAL_CAN_AddTxMessage(hcan, &header, 0, &pTxMailbox);
}
