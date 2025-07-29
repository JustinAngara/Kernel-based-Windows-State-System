﻿#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <thread>
#include <map>

#include "guidev.h"
#include "Theme/Theme.h"
#include "imgui/imgui_impl_win32.h"

#include "global.h"
#include "Memory.h"
#include "Utils.h"

#define MSGBOX(x) \
{ \
   std::ostringstream oss; \
   oss << x; \
   std::string str = oss.str(); \
   std::wstring stemp = std::wstring(str.begin(), str.end()); \
   LPCWSTR sw = stemp.c_str(); \
   MessageBox(NULL, sw, L"Msg Title", MB_OK | MB_ICONQUESTION); \
}



// Static Pointers //

uint32_t LocalPlayerAdress;

// Static Pointers End //

DWORD BaseModuleAdress;
DWORD ProcessId;

bool SimulateLeftClick = false;
char nameBuffer[16]; // Buffer for Player Name

void ReadWriteThreadController() {
	// stored variables in guiimpl.h
	while (true) {
		if (enableLegitAimbot || enableRageAimbot) { // Aimbot <- Legit / Rage
			uintptr_t EntityList = Driver.ReadVirtualMemory<uintptr_t>(ProcessId, BaseModuleAdress + Memory::Adress::EntityList, sizeof(uintptr_t));

			if (EntityList) { // Check for null
				float Dist = 1000;

				uintptr_t TargetEntity = LocalPlayerAdress;

				int CurrentPlayerCount = Driver.ReadVirtualMemory<int>(ProcessId, BaseModuleAdress + 0x18AC0C, sizeof(int));

				//MSGBOX(CurrentPlayerCount);

				for (int i = 1; i < CurrentPlayerCount; i++) {
					uintptr_t Entity = Driver.ReadVirtualMemory<uintptr_t>(ProcessId, EntityList + (i * 0x4), sizeof(uintptr_t));

					/* Assign Vars */
					Vec3 EntityHeadPos = Driver.ReadVirtualMemory<Vec3>(ProcessId, Entity + Memory::EntityOffsets::HeadX, sizeof(Vec3));
					Vec3 LocalPlayerHeadPos = Driver.ReadVirtualMemory<Vec3>(ProcessId, LocalPlayerAdress + Memory::EntityOffsets::HeadX, sizeof(Vec3));
					int EntityHealth = Driver.ReadVirtualMemory<int>(ProcessId, Entity + Memory::EntityOffsets::HP, sizeof(int));
					int EntityTeam = Driver.ReadVirtualMemory<int>(ProcessId, Entity + Memory::EntityOffsets::Team, sizeof(int));
					int LocalPlayerTeam = Driver.ReadVirtualMemory<int>(ProcessId, LocalPlayerAdress + Memory::EntityOffsets::Team, sizeof(int));

					//MSGBOX("Entity: 0x" << std::hex << Entity << " EntityHealth: " << EntityHealth << " EntityTeam: " << EntityTeam << " LocalPlayerTeam: " << LocalPlayerTeam)

					//MSGBOX("Local Player Head Pos: " << LocalPlayerHeadPos.x << ", " << LocalPlayerHeadPos.y << ", " << LocalPlayerHeadPos.z << "\nEntity Head Pos: " << EntityHeadPos.x << ", " << EntityHeadPos.y << ", " << EntityHeadPos.z);

					if (enableLegitAimbot) { /* Legit Targetter kun on screen */
						ViewMat ViewMatrix = Driver.ReadVirtualMemory<ViewMat>(ProcessId, BaseModuleAdress + 0x17DFD0, sizeof(ViewMat)); // Get ViewMatrix
						Vec2 screen;  // This will hold the 2D screen position if the conversion is successful
						WindowSize windowSize = GetWinSize(ProcessId);  // Get the window size

						bool isOnScreen = WorldToScreen(EntityHeadPos, screen, ViewMatrix.Matrix, windowSize.width, windowSize.height);

						if ((LocalPlayerHeadPos - EntityHeadPos).hypo3() < Dist && EntityHealth > 0 && EntityTeam != LocalPlayerTeam && isOnScreen && screen.x != 0.f && screen.y != 0.f) // Do Checks
						{
							TargetEntity = Entity; // Set Target
							Dist = (LocalPlayerHeadPos - EntityHeadPos).hypo3(); // Set Distance

							//MSGBOX("Target Entity: 0x" << std::hex << TargetEntity << " Dist: " << Dist);
						}
					}

					if (enableRageAimbot) { /* Rage Targetter Alt */
						if ((LocalPlayerHeadPos - EntityHeadPos).hypo3() < Dist && EntityHealth > 0 && EntityTeam != LocalPlayerTeam) // Do Checks
						{
							TargetEntity = Entity; // Set Target
							Dist = (LocalPlayerHeadPos - EntityHeadPos).hypo3(); // Set Distance

							//MSGBOX("Target Entity: 0x" << std::hex << TargetEntity << " Dist: " << Dist);
						}
					}
				}

				Vec3 EntityHeadPos = Driver.ReadVirtualMemory<Vec3>(ProcessId, TargetEntity + Memory::EntityOffsets::HeadX, sizeof(Vec3));
				Vec3 LocalPlayerHeadPos = Driver.ReadVirtualMemory<Vec3>(ProcessId, LocalPlayerAdress + Memory::EntityOffsets::HeadX, sizeof(Vec3));

				//MSGBOX("Local Player Head Pos: " << LocalPlayerHeadPos.x << ", " << LocalPlayerHeadPos.y << ", " << LocalPlayerHeadPos.z << "\nEntity Head Pos: " << EntityHeadPos.x << ", " << EntityHeadPos.y << ", " << EntityHeadPos.z);

				Vec3 Delta = EntityHeadPos - LocalPlayerHeadPos; // Get Delta

				float yaw = atan2f(Delta.y, Delta.x) * 180 / 3.141592653589793238463; // Get Yaw
				float hyp = sqrt(Delta.x * Delta.x + Delta.y * Delta.y); // Get Hypo
				float pitch = atan2f(Delta.z, hyp) * 180 / 3.141592653589793238463; // Get Pitch

				pitch + 90; // 90 til pitch eller kigger man det forkerte sted af en grund??

				if (enableLegitAimbot) {
					if (IsKeyPressed(VK_LCONTROL) && TargetEntity != LocalPlayerAdress) {
						// Check if aimbot key is pressed
						Driver.WriteVirtualMemory<float>(ProcessId, LocalPlayerAdress + Memory::EntityOffsets::ViewAngleX, yaw + 90, sizeof(float)); // Write Pitch
						Driver.WriteVirtualMemory<float>(ProcessId, LocalPlayerAdress + Memory::EntityOffsets::ViewAngleY, pitch, sizeof(float)); // Write Yaw
					}
				}

				if (enableRageAimbot) {
					Driver.WriteVirtualMemory<float>(ProcessId, LocalPlayerAdress + Memory::EntityOffsets::ViewAngleX, yaw + 90, sizeof(float)); // Write Pitch
					Driver.WriteVirtualMemory<float>(ProcessId, LocalPlayerAdress + Memory::EntityOffsets::ViewAngleY, pitch, sizeof(float)); // Write Yaw

					SimulateLeftClick = true;
					//Simulate_LeftClick();
				}
			}
		}

		if (!enableRageAimbot) {
			if (SimulateLeftClick) {
				SimulateLeftClick = false;
			}
		}
	}

	return;
}

