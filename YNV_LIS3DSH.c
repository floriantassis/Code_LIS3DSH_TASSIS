#include "YNV_LIS3DSH.h"

LIS3DSH_Status_t LIS3DSH_Write_Reg(SPI_HandleTypeDef *hspi, uint8_t reg_addr, uint8_t *dataW, uint8_t size)
{
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
	dataW[0] = reg_addr;
	if(HAL_SPI_Transmit(hspi, dataW, size, 10) == HAL_OK)
	{
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
		return LIS3DSH_OK;
	}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
	return LIS3DSH_ERROR;
}

LIS3DSH_Status_t LIS3DSH_Read_Reg(SPI_HandleTypeDef *hspi,uint8_t reg_addr,uint8_t *dataR,uint8_t size)
{
	uint8_t dataT = 0x80 | reg_addr;
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
	if(HAL_SPI_TransmitReceive(hspi, &dataT, dataR, size, 10) == HAL_OK)
	{
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
		return LIS3DSH_OK;

	}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);


	return LIS3DSH_ERROR;
}

LIS3DSH_Status_t LIS3DSH_Init_Reg(SPI_HandleTypeDef *hspi, LIS3DSH_init_t *initDef)
{

	uint8_t spiData[3] = {0x00, 0x00, 0x00};
	uint8_t spiCheckData[2] = {0x00, 0x00};
	LIS3DSH_Read_Reg(hspi, 0x00, spiCheckData, 1);

	spiData[1] |= (initDef->fScaleSelec & 0x00);

	spiData[2] |= (initDef->odr & 0xF0);
	spiData[2] |= (initDef->xenable & 0x01);
	spiData[2] |= (initDef->yenable & 0x02);
	spiData[2] |= (initDef->zenable & 0x04);
	spiData[2] |= (initDef->bdu & 0x08);


	if(LIS3DSH_Write_Reg(hspi, LIS3DSH_CTRLREG4_ADDR, spiData, 3) == LIS3DSH_OK)
	{
		if(LIS3DSH_Read_Reg(hspi, LIS3DSH_CTRLREG4_ADDR, spiCheckData, 1) == LIS3DSH_OK)
		{
			if(spiData[2] == spiCheckData[0])
			{
				return LIS3DSH_OK;
			}
		}
	}
	return LIS3DSH_ERROR;
}


LIS3DSH_Status_t LIS3DSH_Get_Position(SPI_HandleTypeDef *hspi, LIS3DSH_init_t *lis3dsh, float pitch, float roll)
{

	uint8_t dataRx[2] = {0x00, 0x00};
	uint8_t dataRy[2] = {0x00, 0x00};
	uint8_t dataRz[2] = {0x00, 0x00};
	int16_t x, y, z;

	if(LIS3DSH_Read_Reg(hspi, LIS3DSH_OUT_X_L_ADDR, dataRx, 2) == LIS3DSH_OK)
	{
		if(LIS3DSH_Read_Reg(hspi, LIS3DSH_OUT_X_H_ADDR, dataRx, 2) == LIS3DSH_OK)
		{
			x = ((dataRx[1]<<8) | dataRx[0]);
		}
	}

	if(LIS3DSH_Read_Reg(hspi, LIS3DSH_OUT_Y_L_ADDR, dataRy, 2) == LIS3DSH_OK)
	{
		if(LIS3DSH_Read_Reg(hspi, LIS3DSH_OUT_Y_H_ADDR, dataRy, 2) == LIS3DSH_OK)
		{
			y = ((dataRy[1]<<8) | dataRy[0]);
		}
	}

	if(LIS3DSH_Read_Reg(hspi, LIS3DSH_OUT_Z_L_ADDR, dataRz, 2) == LIS3DSH_OK)
	{
		if(LIS3DSH_Read_Reg(hspi, LIS3DSH_OUT_Z_H_ADDR, dataRz, 2) == LIS3DSH_OK)
		{
			z = ((dataRz[1]<<8) | dataRz[0]);
		}
	}

	lis3dsh->accX = x/2000.0;
	lis3dsh->accY = y/2000.0;
	lis3dsh->accZ = z/2000.0;

	pitch = 180 * atan2(lis3dsh->accY, lis3dsh->accZ) / M_PI;
	roll = 180 * atan2(lis3dsh->accX, lis3dsh->accZ) / M_PI;

	if(pitch>0)
	{
		//High Led ON

		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
		return LIS3DSH_OK;
	}
	else if(pitch<0)
	{
		//Low Led ON
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		return LIS3DSH_OK;
	}

	if(roll>0)
	{
		//Right Led ON
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		return LIS3DSH_OK;
	}
	else if(roll<0)
	{
		//Left Led ON
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
		return LIS3DSH_OK;
	}
	return LIS3DSH_ERROR;
}



