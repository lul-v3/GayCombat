#pragma once
#include "helper.h"
#include "mem.h"

uintptr_t moduleBase = (uintptr_t)GetModuleHandle("Project.dll");

// Cheats Addresses
uintptr_t primaryAmmoAddress = mem::findeAddress(moduleBase + 0x04B23AB8, { 0xB8, 0x8, 0x20, 0x20, 0x40, 0xA0, 0x230 });
uintptr_t secondaryAmmoAddress = mem::findeAddress(moduleBase + 0x04B23AB8, { 0xB8, 0x8, 0x20, 0x68, 0x10, 0xA0, 0x248 });
uintptr_t primaryMagAddress = 0x1D78C4712D8;    // Address for magazine value
uintptr_t grenadeAddress = mem::findeAddress(moduleBase + 0x04B23AB8, { 0xB8, 0x8, 0x20, 0x68, 0x10, 0xA0, 0x260 });

// Cheat settings flags
bool unlimitedPrimaryAmmo = false;              // Flag for unlimited primary ammo
bool unlimitedSecondaryAmmo = false;            // Flag for unlimited secondary ammo
bool unlimitedPrimaryMag = false;               // Flag for unlimited primary magazine
bool unlimitedGrenade = false;                  // Flag for unlimited grenades

void HandleCheats(HANDLE hProcess) 
{
    // Handle unlimited ammo setting
    if (unlimitedPrimaryAmmo) {
        int currentAmmo = helper::ReadInt(hProcess, primaryAmmoAddress);    // Read current ammo
        std::cout << "Current Ammo: " + std::to_string(currentAmmo);        // Log current ammo
        std::cout << "Setting Ammo to 150.";                                // Log the action
        helper::WriteInt(hProcess, primaryAmmoAddress, 150);                // Set ammo to 150
    }
    if (unlimitedSecondaryAmmo) {
        int currentAmmo = helper::ReadInt(hProcess, secondaryAmmoAddress);  // Read current ammo
        std::cout << "Current Ammo: " + std::to_string(currentAmmo);        // Log current ammo
        std::cout << "Setting Ammo to 50.";                                 // Log the action
        helper::WriteInt(hProcess, secondaryAmmoAddress, 50);               // Set ammo to 50
    }

    // Handle unlimited magazine setting
    if (unlimitedPrimaryMag) {
        std::cout << "Setting Magazine to 150.";                            // Log the action
        helper::WriteInt(hProcess, primaryMagAddress, 150);                 // Set magazine to 150
    }

    // Handle unlimited grenade setting
    if (unlimitedGrenade) {
        std::cout << "Setting Grenade to 2.";                               // Log the action
        helper::WriteInt(hProcess, grenadeAddress, 2);                      // Set grenades to 2
    }

    CloseHandle(hProcess);                                                  // Close the handle to the process
}