# LoRaWAN® end-device example project, v1.0
​
![Alt text](/Docs/move-x_logo.png?raw=true)
    
## AT DOCUMENTATION
​
The aim of this project is to show an example of an end-device LoRaWAN® stack implementation with particular AT Commands. 
​
### Introduction
​
This FW example permits to interface with the LoRaWAN® to manage LoRa® wireless link via AT commands or with ANY microprocessor or microcontroller via serial interface. The LoRaWAN® bands compliant with this FW are the **EU868, AU915 and US915**, while the classes admitted are **A** and **C**. In the official documentation are listed all the characteristics of this code, like the admitted microcontrollers (**STM32WLE5JB** and **STM32WLE5JC**), power consumptions (only **2uA** in sleep mode, class C), memory occupation (**79.92 KB** and **99.73 KB** respectively in release and debug modes), a brief guide on how to set serial communication with the microcontrollers and how to import this project into STM32CubeIDE (proprietary IDE of STMicroelectronics). 
This FW can be:
- the starting point of any more complex project;
- used for validating a node or an external application; 
- tried **JUST FOR FUN** :smile:.
​
The FW explained in this note is the **1.0 version**.
​
### Release and debug mode
​
This FW has two operational modes: **release** and **debug**. The former is the **commercial mode**, with minimal and fixed output from the node, minimum dimension and power consumption, while the latter is the **prototyping mode**, with extensive message returned from the node. Going in release mode automatically **disable the debugger** and **activate low power mode**. In the official documentation is described how to switch between the two modes and how to set one of them to the node (***see pp.6-7-8 of official documentation***).
​
## AT COMMAND
​
These improved commands permits to send directly in LoRa® level (physical) or LoRaWAN® (MAC) and to retrieve some important information from the node (battery level, time, send parameters, active channels ecc.). 
​
Here are the list of AT commands implemented in this FW version, together with the parameters you must insert (if present):
- ATZ 
- AT? 
- AT+TXP=Power Index 
- AT+BAND=LoRa Region 
- AT+DEUI=Device EUI 
- AT+APPEUI=Application Key 
- AT+NWKKEY=Network Key 
- AT+APPKEY=Application Key 
- AT+JOIN=Join type 
- AT+CHANNEL=Mask0:Mask1:Mask2:Mask3:Mask4:Mask5:Mask6:Mask7:Mask8:Mask9:Mask10:Mask11 
- AT+ADR=ADR type 
- AT+DR=Data Rate 
- AT+CW=Timeout:Frequency:Power level 
- AT+STOPCW 
- AT+CFGSEND=Port:Number of send:Ack:Send timer:Datarate:Power index 
- AT+SEND=Payload Length:Payload raw 
- AT+STOPSEND 
- AT+VL=verbosity level:timestamp flag 
- AT+VER=? 
- AT+DADDR=device address 
- AT+DCS=value 
- AT+RX2FQ=frequency
- AT+RX2DR=Datarate 
- AT+RX1DL=delay
- AT+RX2DL=delay 
- AT+JN1DL=delay 
- AT+JN2DL=delay 
- AT+NWKID=network ID 
- AT+CLASS=class value 
- AT+LTIME=? 
- AT+TREQ 
- AT+BAT=? 
- AT+TTH=Start frequency:Stop frequency:frequency resolution:number of packets to send 
- AT+TCONF=TX frequency:TX power:BW:SF:4/CR:LNA state:Boost PA state:Modulation type:Payload length:Low DR optimization:BT FSK product 
- AT+TTX=Send’s number:Channel frequency:Spreading Factor:BW:Payload Length:Payload 
- AT+TRX=Packet’s number to receive:Channel frequency:Spreading Factor:BW 
​
In the official documentation there are exhaustive examples and some relevant screenshots.

## SERIAL EXAMPLE

Inside "serial_example" folder you can find an example of serial communication with a board (keys and parameters, like serial port and others, must be set by the user). The example permits to make a join, configure send' parameters and transmits a dummy payload (2 bytes set to 1).
​
## REFERENCE DOCUMENTS
1. [FW Documentation](https://www.move-x.it/wp-content/uploads/2022/01/MAMWLE-FW-AT-Command-Document.docx.pdf)
2. [MAMWLExx Module](https://www.move-x.it/mamwle-module/)
3. [LoRaWAN 1.0.3 Specification by LoRa Alliance® Specification Protocol – 2018, January](https://lora-alliance.org/resource_hub/lorawan-specification-v1-0-3/)
4. [Application note How to build a LoRa application with STM32CubeWL (AN5406)](https://www.st.com/resource/en/application_note/an5406-how-to-build-a-lora-application-with-stm32cubewl-stmicroelectronics.pdf)
5. [User manual Description of STM32WL HAL and low-layer drivers (UM2642)](https://www.st.com/resource/en/user_manual/dm00660673-description-of-stm32wl-hal-and-lowlayer-drivers-stmicroelectronics.pdf)
6. [STM32WLE5JB documentation (datasheet, manuals, application examples etc.)](https://www.st.com/en/microcontrollers-microprocessors/stm32wle5jb.html#documentation)
7. [STM32WLE5JC documentation (datasheet, manuals, application examples etc.)](https://www.st.com/en/microcontrollers-microprocessors/stm32wle5jc.html)
8. [LoRa alliance certification deepening](https://lora-alliance.org/wp-content/uploads/2020/11/lora_alliance_certification_deep_dive.pdf)
​
## ACKNOWLEDGMENTS
* This FW is an improved version of an example of LoRaWAN® application present in the STM32CubeWL (STM32Cube MCU Package for STM32WL series), [v1.1](https://www.st.com/content/my_st_com/en/products/embedded-software/mcu-mpu-embedded-software/stm32-embedded-software/stm32cube-mcu-mpu-packages/stm32cubewl.license=1639757194897.product=STM32CubeWL.version=1.1.0.html).
