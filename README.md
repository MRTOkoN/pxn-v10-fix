# PXN-V10 Fix

## 📌 Description

This project solves a common issue with the PXN-V10 steering wheel.

If you’ve tried using this wheel in games like My Summer Car, The Long Drive, and others, you’ve likely encountered incorrect behavior:

- the game recognizes only one direction of rotation  
- the neutral position is detected as fully turned to the left  
- further movement is only registered to the right  
- the game does not detect 5th, 6th, reverse gears, and some other buttons  

## 🛠 Solution

PXN-V10 Fix works in combination with the virtual driver ([vJoy](https://github.com/shauleiz/vJoy/tree/master)) and corrects the wheel behavior:

- normalizes axis input  
- makes the wheel compatible with how games expect input  
- emulates standard behavior of other steering wheels  

## ⚙️ How It Works

At the moment, this is not a fully native solution, so the following setup is required:

- 🎮 Steering — via virtual wheel ([vJoy](https://github.com/shauleiz/vJoy/tree/master))  
- 🔁 Force Feedback — via the real PXN-V10  

## ⚠️ Limitations

- this is a partial solution and requires manual setup  
- no full Force Feedback integration yet  
- behavior may vary between different games  
- you need to install ([vJoy](https://github.com/shauleiz/vJoy/tree/master)) in addition to this program  

## 🚀 Plans

Planned improvements include:

- adding full Force Feedback support  
- simplifying the setup process  
- improving compatibility with more games  
- making the solution more flexible and configurable  
- adding support for other PXN wheels (if you use a different model, please share your experience — it will help evaluate adding support for your device)  

## 🎯 Summary

With PXN-V10 Fix, you can comfortably play many games despite the wheel’s original compatibility issues.

## ⚙️ Installation

🚧 This section is currently under development. Please stay tuned.
