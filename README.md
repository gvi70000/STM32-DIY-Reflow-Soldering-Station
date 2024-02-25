# STM32-DIY-Reflow-Soldering-Station

Warning: Electrical Safety Notice

Before attempting to build or operate the DIY reflow soldering station described in this article, it is imperative to be aware that the hardware operates at 220V AC. Working with high-voltage systems poses inherent risks and demands a thorough understanding of electrical safety precautions.
![Front](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/4f2105c1-44f3-451f-93e3-7e3d3e008b3c)
![Back](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/1feeb97b-829e-42c7-9005-bcffe356d9ec)

Introduction:

For electronics enthusiasts seeking a precise and customizable reflow soldering experience, constructing a DIY reflow soldering station is an exciting project. This article provides a detailed overview of the key components and their functionalities, offering a comprehensive guide for building your own soldering station.

Power Supply:

The 220V AC is supplied through the AC Power Entry Module IEC-320 C-14 BVA01/Z0000/02. A Transformer Myrra 47152 steps down the voltage to a crucial 5V, supplying power to essential components such as the BluePill with STM32F302 microcontroller board, fan, and the G3MB-202P 5V DC 1 Channel Solid-State Relay.

![PowerSocket](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/0646a75a-bc3b-4ce9-a87a-2c9d9fd71ff4)
![Relay](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/a07a94ea-6959-47dc-acb7-2fea954fb576)
![NTC](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/d3630d77-b9d2-4ac8-9ba8-257286b412dd)
![Myrra](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/d87579f2-11ab-4cd1-86fe-e481114b0bd4)
![LCD](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/38ac51ce-70b8-485e-b637-edb2925c8d95)
![Plate](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/125f49ae-78c7-4988-b7df-3c12dd0aec34)
![PEC11R-4220F-S0024](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/090fda57-534f-46ed-badf-cc595a9148fa)
![EnclosureFront](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/205c42c6-8cf8-40b7-a5f2-e53feb9e7332)
![EnclosureBack](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/073030b8-0ae6-40ae-b83a-ba8ff3260bb7)
![InnerView](https://github.com/gvi70000/STM32-DIY-Reflow-Soldering-Station/assets/248221/53995738-647b-49f3-a915-8bb7cc494793)

User Interface:

The station features a user-friendly interface displayed on a 1.8" ST7735 LCD. User inputs are facilitated through an encoder PEC11R-4220F-S0024 and an RS PRO 17mm Black Potentiometer Knob for 6mm Shaft with a D-shaped grip, ensuring precise control over the soldering process.
Temperature Control and Cooling:

Maintaining optimal temperature is achieved through two strategically positioned 100K ohm 3950 NTC's, calibrated with meticulous accuracy using an FLUKE 51 II Thermometer as a reference. The reflow soldering station incorporates a CFM-6020S-040-320 fan, intelligently activated to cool down the hot plate when needed, ensuring temperature stability.

Operational Modes:

The DIY reflow soldering station offers two distinct modes: rework and reflow. In rework mode, the temperature is kept at a preset value, allowing for precise touch-up soldering. In reflow mode, the temperature follows a preset profile, ideal for soldering components with specific temperature requirements. Both modes utilize a PID controller, ensuring consistent and accurate temperature control throughout the soldering process.

Connections:

The intricate network of connections ensures seamless communication and control within the system. Notable connections include those for SD Card, Encoder, ADC for NTCs, Fan control, Hot plate power control, Buzzer, Encoder button, LCD command, and SPI interface.
Conclusion:

Building your own DIY reflow soldering station offers the satisfaction of creating a precise and reliable tool tailored to your needs. With a robust power supply, intuitive user interface, accurate temperature control, and intelligent cooling, this reflow soldering station provides the necessary features for successful soldering projects. As you embark on this DIY journey, ensure meticulous attention to detail in the assembly and follow safety guidelines to enjoy a versatile and efficient soldering tool for your electronics projects.

The enclosure outline for LASER cutting from 4mm thick acrylic sheet can be downloaded here.

KiCAD Schematic and PCB for controller are available here.

The software for STM32F302 can be downloaded here.

Please note that the PID parameters are generic and you need to change them according to your system, the values are found in Reflow.h file
