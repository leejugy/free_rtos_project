# RTOS 동작 분석

## 어떻게 RTOS가 TCP 데이터를 가져오는가(수신동작)

1. HAL_ETH_Start_IT
    - 내부에서 ETH_UpdateDescriptor -> HAL_ETH_RxAllocateCallback(&buff); 호출
    - NetworkBufferDescriptor_t를 할당하여
    - buff에 pxBufferDescriptor->pucEthernetBuffer 주소 저장
    - WRITE_REG(dmarxdesc->BackupAddr0, (uint32_t)buff); BackupAddr0씀
    - WRITE_REG(dmarxdesc->DESC0, (uint32_t)buff); DESC0씀
    - DESC0 : 이더넷 수신 dma가 수신된 데이터를 채워넣을 버퍼의 주소

2. HAL_ETH_RxCpltCallback
    - 함수가 xTaskNotifyFromISR로 notify 전송 (일종의 시그널 인듯)
    - 테스크는 수신 컴플리트가 되었음을 알아챔
    - prvNetworkInterfaceInput가 notify 이벤트가 eMacEventRx인 경우 호출됨
    - HAL_ETH_ReadData 호출
    - HAL_ETH_RxLinkCallback 호출
    - HAL_ETH_RxLinkCallback(&heth->RxDescList.pRxStart, &heth->RxDescList.pRxEnd,
                             (uint8_t *)dmarxdesc->BackupAddr0, (uint16_t) bufflength);
    - pucBuff = dmarxdesc->BackupAddr0 여기서 포인터 수를 감소시켜서 HAL_ETH_RxAllocateCallback여기서 할당되었던
    NetworkBufferDescriptor_t 주소를 가져옴
    - ppvStart가 null인 경우 즉 주어진 디스크립터에서 한번 읽어서 디스크립터가 비어있는 경우 수신 패킷의 주소 저장
    - *ppxStartDescriptor = pxCurDescriptor
    - *pAppBuff = heth->RxDescList.pRxStart 즉 dmarxdesc->BackupAddr0 의 주소가 저장될 거임 (디스크립터의 BackupAddr0)
    - prvNetworkInterfaceInput -> prvSendRxEvent 여기서 큐 푸쉬
    - xSendEventStructToIPTask -> xQueueSendToBack으로 xNetworkEventQueue여기에 푸쉬

3. prvIPTask
    - prvProcessIPEventsAndTimers 호출
    - eNetworkRxEvent 로 브랜치 분기하여 prvHandleEthernetPacket 여기서 파싱될 것임

## TCP 파싱