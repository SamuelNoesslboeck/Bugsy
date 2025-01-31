<p align="center">
  <img src="design/logo/bugsy_logo_titled.svg" width="60%" />
</p>

---

> Project creation date: 14/06/2024  

A small, robust little robot equipped with a powerful and versatile control system.

- [Code](code/README.md): Detailed description of the software with all of it's components
- [Project Log](documentation/project_log.md): A detailed project log featuring a changelog with the decisions made along the way

## Idea

![Initial Design](documentation/concept/1_body_design.png)

The Bugsy is concepted as a robust little rover-like robot, that just uses two chains for movement, meaning no complex steering or other kind of movement is required.

## Control system

![Basic build-up](code/export/bugsy_code_structure.svg)

The controls are split up into three controllers, ensuring a powerful and versatile control system, which are:
- **Core**: A *NodeMCU-32S* microcontroller, responsible for motor control and basic connections
- **Trader**: An *Arduino-Mega* microcontroller with a large set of IO-ports, responsible for peripheral devices, not required for core functionality
- **RPi**: An additional *Raspberry Pi Zero*, responsible for expensive add-ons like camera and microphone