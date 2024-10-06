# New battery pack - Unlimited power

> Creation date: 03/09/2024 (yes i was that motivated on the first day)

The bugsy uses, like most small-dimension robots, LiPo battery cells and as practical as they turn out to be, they sadly also have a lot of issues ...

## LiPo cells and their pros and cons

Probably the best thing about LiPo cells is their high energy density. That means while taking very little space they deliver decent voltage with magnificent discarge rates, even described in a spectial unit **C**. If a battery has 20 *C*, it can completely discharge itself from max to zero *20 times per hour*. Why such a specific unit one, including myself, might ask. However just by simply multiplying this unit with the capacity given in **Amp-Hours**, one can get the *short circuit or max output amps of this battery*. So if our battery mentioned earlier has a capacity of 1500mAh, then it can output 30000mA or 30A.

However due to the chemical reaction in such a LiPo cell it can only procude around 3.7V, meaning that it even multicell versions are drastically limited to these kind of steps:

- 1 Cell -> 3.7V
- 2 Cell -> 7.4V
- 3 Cell -> 11.1V

## Issues with these kind of voltage-steps

The problem now is that when not using special robotics motors, or when trying to get full potential out of the given electronics, these voltages are far too low compared to their possible amperage. 

Aso the gearmotors I have in use prove to be rather slow, but offer a great amount of strength even at low amperages. 

## Solution and creation of the new pack

The simple solution to this problem: Chaining multiple batteries together. Using multiple packs from the same manufacturer can increase voltages and provide a practical way to get more out of the electronics at hand.

> TODO: Insert images taken of first battery pack creation

## Upgrades and improvemnts to the robot through the new battery pack

The new battery pack increases the voltage of the robot up to 22.2 Volts, which also greatly increases movement speed and ease of voltage conversions. 