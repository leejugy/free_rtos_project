/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eth.c
  * @brief   This file provides code for the configuration
  *          of the ETH instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "FreeRTOS_IP.h"
#include "usart.h"
#include "lan8742.h"
#include "phyHandling.h"
#include "app_freertos.h"
#include "status.h"
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "eth.h"
#include "string.h"

ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT * 2U];
ETH_TxPacketConfigTypeDef TxConfig;
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

ETH_TxPacketConfigTypeDef TxConfig;

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

ETH_HandleTypeDef heth;

/* ETH init function */
void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfigTypeDef));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

void HAL_ETH_MspInit(ETH_HandleTypeDef* ethHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(ethHandle->Instance==ETH)
  {
  /* USER CODE BEGIN ETH_MspInit 0 */

  /* USER CODE END ETH_MspInit 0 */
    /* ETH clock enable */
    __HAL_RCC_ETH_CLK_ENABLE();
    __HAL_RCC_ETHTX_CLK_ENABLE();
    __HAL_RCC_ETHRX_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    /**ETH GPIO Configuration
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PA2     ------> ETH_MDIO
    PA7     ------> ETH_CRS_DV
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1
    PB15     ------> ETH_TXD1
    PG11     ------> ETH_TX_EN
    PG13     ------> ETH_TXD0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* ETH interrupt Init */
    HAL_NVIC_SetPriority(ETH_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
    HAL_NVIC_SetPriority(ETH_WKUP_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ETH_WKUP_IRQn);
  /* USER CODE BEGIN ETH_MspInit 1 */

  /* USER CODE END ETH_MspInit 1 */
  }
}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef* ethHandle)
{

  if(ethHandle->Instance==ETH)
  {
  /* USER CODE BEGIN ETH_MspDeInit 0 */

  /* USER CODE END ETH_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ETH_CLK_DISABLE();
    __HAL_RCC_ETHTX_CLK_DISABLE();
    __HAL_RCC_ETHRX_CLK_DISABLE();

    /**ETH GPIO Configuration
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PA2     ------> ETH_MDIO
    PA7     ------> ETH_CRS_DV
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1
    PB15     ------> ETH_TXD1
    PG11     ------> ETH_TX_EN
    PG13     ------> ETH_TXD0
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_11|GPIO_PIN_13);

    /* ETH interrupt Deinit */
    HAL_NVIC_DisableIRQ(ETH_IRQn);
    HAL_NVIC_DisableIRQ(ETH_WKUP_IRQn);
  /* USER CODE BEGIN ETH_MspDeInit 1 */

  /* USER CODE END ETH_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void eth_init()
{
    uint8_t dev_ip[] = {
        ipconfigIP1, ipconfigIP2, ipconfigIP3, ipconfigIP4};
    uint8_t mask[] = {
        ipconfigMASK1, ipconfigMASK2, ipconfigMASK3, ipconfigMASK4};
    uint8_t gate[] = {
        ipconfigGATE1, ipconfigGATE2, ipconfigGATE3, ipconfigGATE4};
    uint8_t dns[] = {
        ipconfigDNS1, ipconfigDNS2, ipconfigDNS3, ipconfigDNS4};

    if (!FreeRTOS_IPInit(dev_ip, mask, gate, dns, heth.Init.MACAddr))
    {
        printfail("ETH : init fail");
        return;
    }
    printok("ETH : init end");
}



int check_valid_ip(char *ip)
{
    if (ip == NULL)
    {
        return -1;
    }

    char ip_add[IP_LEN] = {0, };
    char *ptr = NULL;
    int dot_cnt = 0;
    int ip_int = 0;

    strncpy(ip_add, ip, sizeof(ip_add));
    ptr = strtok(ip_add, ".");

    while(ptr != NULL)
    {
        ip_int = atoi(ptr);
        if (!chk_valid_ip(ip_int))
        {
            return -1;
        }
        dot_cnt++;
        ptr = strtok(NULL, ".");
    }

    if (dot_cnt != 4)
    {
        return -1;
    }

    return 1;
}

BaseType_t xApplicationGetRandomNumber(uint32_t *pulValue)
{
    int ret = 0;

	ret = HAL_RNG_GenerateRandomNumber(&hrng, pulValue);
	if (ret == HAL_OK)
	{
		return pdPASS;
	}
	else
	{
		return pdFAIL;
	}
}

uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress, uint16_t usSourcePort, uint32_t ulDestinationAddress, uint16_t usDestinationPort)
{
    uint32_t val = 0x12347080;
    HAL_RNG_GenerateRandomNumber(&hrng, &val);
	return val;
}

#if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{

}
#else
void vApplicationIPNetworkEventHook_Multi(eIPCallbackEvent_t eNetworkEvent, struct xNetworkEndPoint *pxEndPoint)
{
    status_set_int(STATUS_INTEGER_TCP_CLIENT, STATUS_TCP_UP);
}
#endif

#if (ipconfigUSE_DHCP_HOOK != 0)
eDHCPCallbackAnswer_t xApplicationDHCPHook(eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress)
{
	eDHCPCallbackAnswer_t eAnswer = eDHCPContinue;
	return eAnswer;
}
#endif

#if (ipconfigSUPPORT_OUTGOING_PINGS == 1)
void vApplicationPingReplyHook( ePingReplyStatus_t eStatus, uint16_t usIdentifier )
{
    if (status_get_int(STATUS_INTEGER_PING) != STATUS_PING_WAIT)
    {
        return;
    }

    switch (eStatus)
    {
    case eInvalidData:
    case eInvalidChecksum:
        status_set_int(STATUS_INTEGER_PING, STATUS_PING_FAIL);
        break;
    
    case eSuccess:
        status_set_int(STATUS_INTEGER_PING, STATUS_PING_OK);
        break;
    }
}
#endif

#if ( ipconfigIPv4_BACKWARD_COMPATIBLE == 1 )
BaseType_t xApplicationDNSQueryHook( const char *pcName )
{
    (void) pcName;
    return pdPASS;
}
#else
BaseType_t xApplicationDNSQueryHook_Multi(&xEndPoint, xSet.pcName)
{
    (void) pcName;
    return pdPASS;
}
#endif

#if ( ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES != 0 )
BaseType_t eApplicationProcessCustomFrameHook( NetworkBufferDescriptor_t *pxNetworkBuffer )
{
    (void) pxNetworkBuffer;
    return eReleaseBuffer;
}
#endif
lan8742_Object_t lan_handle = {0, };
lan8742_IOCtx_t  io_func = {0, };

int lan_link_up()
{
    if (LAN8742_DisablePowerDownMode(&lan_handle) != LAN8742_STATUS_OK)
    {
        return -1;
    }

    if (LAN8742_StartAutoNego(&lan_handle) != LAN8742_STATUS_OK)
    {
        return -1;
    }

    return 0;
}

int lan_link_down()
{
    /* if we set power down bit at BCR, it will automatically reset when power on */
    if (LAN8742_EnablePowerDownMode(&lan_handle) != LAN8742_STATUS_OK)
    {
        return -1;
    }

    return 0;
}

int ifconfig_up()
{
    if (lan_link_up() < 0)
    {
        return -1;
    }

    if (HAL_ETH_Init(&heth) != HAL_OK)
    {
        return -1;
    }
    return 0;
}

int ifconfig_down()
{
    if (lan_link_down() < 0)
    {
        return -1;
    }

    if (HAL_ETH_DeInit(&heth) != HAL_OK)
    {
        return -1;
    }

    NetworkEndPoint_t *ep = FreeRTOS_FirstEndPoint(NULL);
    FreeRTOS_NetworkDown(ep->pxNetworkInterface);
    return 0;
}

void vPhyInitialise(EthernetPhy_t * pxPhyObject, xApplicationPhyReadHook_t fnPhyRead, xApplicationPhyWriteHook_t fnPhyWrite)
{
    io_func.Init     = NULL;
    io_func.DeInit   = NULL;
    io_func.WriteReg = fnPhyWrite;
    io_func.ReadReg  = fnPhyRead;  
    io_func.GetTick  = osKernelGetTickCount;  

    LAN8742_RegisterBusIO(&lan_handle, &io_func);

    LAN8742_Init(&lan_handle);
}

BaseType_t xPhyDiscover( EthernetPhy_t * pxPhyObject )
{
    /* pyh chip 수가 1개임 */
    return 1;
}

BaseType_t xPhyConfigure( EthernetPhy_t * pxPhyObject,  const PhyProperties_t * pxPhyProperties )
{
    (void) pxPhyProperties; 

    /* up lan 8742 */
    if (lan_link_up() < 0)
    {
        FreeRTOS_printf(("Fail to up link"));
        return -1;
    }

    return 0;
}

/**
 * @brief check the phy link status
 * @note prvEMACHandlerTask will call this function continuosly 
 * 100ms wait for task wake up from HAL_ETH_RxCpltCallback
 * @param pxPhyObject ethernet phy object
 * @param xHadReception pdTRUE : packet has been received since the 
 * last call to this function
 * @return link status is poor it will be retured pdFALSE else pdTRUE
 */
BaseType_t xPhyCheckLinkStatus( EthernetPhy_t * pxPhyObject, BaseType_t xHadReception )
{
    static BaseType_t old_status = 0;
    BaseType_t cur_status = 0;
    int32_t ret = 0;

    /* get link status */
    ret = LAN8742_GetLinkState(&lan_handle);
    
    /* LAN8742_STATUS_LINK_DOWN is set when network connection is physically disconnected. */
    if (ret != LAN8742_STATUS_LINK_DOWN && ret != LAN8742_STATUS_AUTONEGO_NOTDONE &&
        ret != LAN8742_STATUS_READ_ERROR && ret != LAN8742_STATUS_WRITE_ERROR)
    {
        cur_status = pdTRUE;
    }
    
    /* check link status change */
    if( old_status != cur_status )
    {
        old_status = cur_status;
        switch (cur_status)
        {
        case pdTRUE:
            /* this will make initialize heth intterupt and recover from critical error */
            pxPhyObject->ulLinkStatusMask = 1; 
            /* close all remaining socket */
            status_set_int(STATUS_INTEGER_TCP_CLIENT, STATUS_TCP_UP);
            status_set_int(STATUS_INTEGER_TCP_SERVER1, STATUS_TCP_UP);
            break;

        case pdFALSE:
            /* this will make deinitialize heth intterupt
             * and eEventType is set eNetworkDownEvent
             */
            pxPhyObject->ulLinkStatusMask = 0;
            /* close all remaining socket */
            status_set_int(STATUS_INTEGER_TCP_CLIENT, STATUS_TCP_DOWN);
            status_set_int(STATUS_INTEGER_TCP_CLIENT, STATUS_TCP_DOWN);
            break;
        }
        FreeRTOS_printf( ( "Link status changed to %s\n", (cur_status ? "UP" : "DOWN") ) );
    }

    return cur_status;
}

BaseType_t xPhyStartAutoNegotiation( EthernetPhy_t * pxPhyObject, uint32_t ulPhyMask )
{
    int32_t status = LAN8742_STATUS_ERROR;

    /*
     * These parameters are for advanced multi-PHY configurations.
     * Since we have a single, fixed PHY on the Nucleo board, we can
     * ignore them and operate directly on our static lan_handle.
     */
    (void) pxPhyObject;
    (void) ulPhyMask;

    /* Call the actual driver function to start auto-negotiation. */
    status = LAN8742_StartAutoNego(&lan_handle);

    if( status == LAN8742_STATUS_OK )
    {
        /* The command to start auto-negotiation was successful. */
        return pdPASS;
    }

    /* The command failed. */
    return pdFAIL;
}
/* USER CODE END 1 */
