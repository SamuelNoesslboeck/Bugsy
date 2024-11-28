# Project log

Here the projects creation is documented, alongside with some of the experiences I made and their impacts on the final outcome (if there is any). Some more detailed articles are listed below:

- [Design and build of new battery pack](new_battery_pack.md) (2024/09/03)

## Design choices (2024/09/03)

The initial design intended it to look a lot more like an actually tank, but as I want it to look a little cute and the chains I bought will be rather impractical in a classical tank setup, I have chosen a newer design with a higher body and a lower back. 

![Bugsy concept art](concept/1_body_design.png)

Also the addition of a logo to universally recognize the project played an imporatant role. For more detailed infos on how all these logos where created, visit the design folder of the [articles repo](https://github.com/SamuelNoesslboeck/articles).

<p align="center">
  <img src="../design/logo/bugsy_logo_titled.svg" width="60%" />
</p>


## Planning the components to use (2024/09/20)

Another task is to define which electrical components should be used by the bugsy to get as much information as possible, while not spamming the network and communication methods used.

## First code (2024/10/27)

In October of 2024, I started with the first code for the microcontrollers and other hardware. Considering that I want to add a large number of remotes and internal devices like sensors and even an LCD-display, I figured that maybe the addition of another microcontroller to the Core ESP32 would be wise. 