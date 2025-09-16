# FreeRTOS TCP 포팅하기

1. 소스 폴더를 워크 플레이스에 추가 (FreeRTOS-Plus-TCP)

2. 소스 폴더에 존재하는 모든 c 파일 재귀적으로 CMakeLists.txt에 추가
    - file(GLOB_RECURSE FREERTOS_TCP_SOURCES "FreeRTOS-Plus-TCP/source/*.c")
    - target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${FREERTOS_TCP_SOURCES})

3. ETH_IRQHandler 이더넷 핸들러에 __ETH_IRQHandler 함수 추가

4. lan8742.c lan8742.h를 Src, Inc에 각각 추가 (phy 칩에 맞게 하기)
    - LISENSE 아파치 2.0
    - https://github.com/STMicroelectronics/stm32-lan8742

5. Inc에 FreeRTOSIPConfig.h를 추가

6. Src/etc.c에 작성된 FreeRTOS 관련 함수 작성