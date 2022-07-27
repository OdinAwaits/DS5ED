#pragma once
#include <Windows.h>
#include <iostream>
#include "vjoyinterface.h"
#include <time.h>
#include <DualSenseWindows/IO.h>
#include <DualSenseWindows/Device.h>
#include <DualSenseWindows/Helpers.h>
#include "ViGEm/Client.h"
#include <time.h>

class ControllerHandler
{

public:
	float offsetX = 0;
	float offsetY = 0;
	float offsetZ = 0;
	unsigned int offsetTX1 = 0;
	unsigned int offsetTY1 = 0;
	unsigned int offsetTX2 = 0;
	unsigned int offsetTY2 = 0;
	bool T1 = false;
	bool T2 = false;
	unsigned int b = 0;

	DS5W::DeviceEnumInfo infos[16];
	unsigned int controllersCount = 0;
	DS5W::DS5InputState inState = DS5W::DS5InputState();
	DS5W::DeviceContext* con = new DS5W::DeviceContext();
	ControllerHandler()
	{

		DS5W_ReturnValue err = DS5W::enumDevices(infos, 16, &controllersCount);

		// check size
		if (controllersCount == 0) {
			printf("No DualSense controller found!");
			system("pause");
		}

		DS5W::initDeviceContext(&infos[0], con);

		if (!vJoyEnabled())
		{
			printf("Failed Getting vJoy attributes.\n");
		}
		WORD VerDll, VerDrv;
		if (!DriverMatch(&VerDll, &VerDrv))
			printf("Failed\r\nvJoy Driver (version %04x) does not match vJoyInterface DLL (version %04x)\n", VerDrv, VerDll);
		else
			printf("OK - vJoy Driver and vJoyInterface DLL match vJoyInterface DLL (version %04x)\n", VerDrv);
		VjdStat status = GetVJDStatus(1);
		if ((status == VJD_STAT_OWN) || \
			((status == VJD_STAT_FREE) && (!AcquireVJD(1))))
		{
			printf("Failed to acquire vJoy device number %d.\n", 1);
		}
		else
			printf("Acquired: vJoy device number %d.\n", 1);
		ResetVJD(1);

		VjdStat status2 = GetVJDStatus(2);
		if ((status2 == VJD_STAT_OWN) || \
			((status2 == VJD_STAT_FREE) && (!AcquireVJD(2))))
		{
			printf("Failed to acquire vJoy device number %d.\n", 2);
		}
		else
			printf("Acquired: vJoy device number %d.\n", 2);
		ResetVJD(1);
	}
	~ControllerHandler()
	{
		RelinquishVJD(1);
		RelinquishVJD(2);
		void JslDisconnectAndDisposeAll();
		DS5W::freeDeviceContext(con);
		delete con;
	}
	void main()
	{

		while (true)
		{

			if (DS5W_SUCCESS(DS5W::getDeviceInputState(con, &inState)))
			{
				// Create struct and zero it
				DS5W::DS5OutputState outState;
				ZeroMemory(&outState, sizeof(DS5W::DS5OutputState));

				//if (inState.leftTrigger >= 0xee)
				outState.leftTriggerEffect.effectType = DS5W::TriggerEffectType::ContinuousResitance;
				outState.leftTriggerEffect.Continuous.force = 0xff;
				outState.leftTriggerEffect.Continuous.startPosition = 0x90;
				outState.rightTriggerEffect.effectType = DS5W::TriggerEffectType::ContinuousResitance;
				outState.rightTriggerEffect.Continuous.force = 0xff;
				outState.rightTriggerEffect.Continuous.startPosition = 0x90;

				// Send output to the controller
				DS5W::setDeviceOutputState(con, &outState);

				if (b & 0x10000)
				{
					offsetX = m.quatX;
					offsetY = m.quatY;
					offsetZ = m.quatZ;
					SetAxis(0x4000, 1, HID_USAGE_X);
					SetAxis(0x4000, 1, HID_USAGE_Y);
					SetAxis(0x4000, 1, HID_USAGE_Z);
				}
				else
				{
					SetAxis(0x4000 - long((m.quatY - offsetY) * 0x8000), 1, HID_USAGE_X);
					SetAxis(0x4000 - long((m.quatX - offsetX) * 0x8000), 1, HID_USAGE_Y);
					SetAxis(0x4000 - long((m.quatZ - offsetZ) * 0x8000), 1, HID_USAGE_Z);
				}

				unsigned char LRoffset = 0;

				if (inState.touchPoint1.down)
				{
					LRoffset = 0;
					if (offsetTX1 > 960)
						LRoffset = 0x03;
					if (!T1)
					{
						offsetTX1 = inState.touchPoint1.x;
						offsetTY1 = inState.touchPoint1.y;
						T1 = 1;
					}
					SetAxis(0x4000 + (offsetTX1 - inState.touchPoint1.x) * 30, 2, 0x30 + LRoffset);
					SetAxis(0x4000 + (offsetTY1 - inState.touchPoint1.y) * 30, 2, 0x31 + LRoffset);
				}
				if (inState.touchPoint2.down)
				{
					LRoffset = 0;
					if (offsetTX2 > 960)
						LRoffset = 0x03;
					if (!T2)
					{
						offsetTX2 = inState.touchPoint2.x;
						offsetTY2 = inState.touchPoint2.y;
						T2 = 1;
						if (inState.touchPoint2.x > 960)
							LRoffset = 0x03;
					}
					SetAxis(0x4000 + (offsetTX2 - inState.touchPoint2.x) * 30, 2, 0x30 + LRoffset);
					SetAxis(0x4000 + (offsetTY2 - inState.touchPoint2.y) * 30, 2, 0x31 + LRoffset);
				}
				if (!inState.touchPoint1.down && !inState.touchPoint2.down)
				{
					SetAxis(0x4000, 2, HID_USAGE_X);
					SetAxis(0x4000, 2, HID_USAGE_Y);
					SetAxis(0x4000, 2, HID_USAGE_RX);
					SetAxis(0x4000, 2, HID_USAGE_RY);
					/*offsetTX1 = inState.touchPoint1.x;
					offsetTY1 = inState.touchPoint1.y;
					offsetTX2 = inState.touchPoint2.x;
					offsetTY2 = inState.touchPoint2.y;*/
					T1 = false;
					T2 = false;
				}

				SetAxis(0x4000 - inState.leftStick.x * 0x80, 1, HID_USAGE_SL0);
				SetAxis(0x4000 - inState.leftStick.y * 0x80, 1, HID_USAGE_SL1);
				SetAxis(0x4000 - inState.rightStick.x * 0x80, 2, HID_USAGE_SL0);
				SetAxis(0x4000 - inState.rightStick.y * 0x80, 2, HID_USAGE_SL1);
				SetAxis(0x4000 - (inState.rightTrigger - inState.leftTrigger) * 0x50, 1, HID_USAGE_RX);

				inState.buttonMap |= unsigned int(inState.leftTrigger == 0xff) << 24 | unsigned int(inState.rightTrigger == 0xff) << 25;

				if (b != inState.buttonMap)
					for (size_t i = 1; i < 27; i++)
					{
						SetBtn((inState.buttonMap >> i - 1) & 1, 1, i);
					}
				/*else
					ResetButtons(1);*/

				if (b == 0x10080)
				{
					break;
				}
				b = inState.buttonMap | unsigned int(inState.leftTrigger == 0xff) << 25 | unsigned int(inState.rightTrigger == 0xff) << 26;
			}

		}
	}
};

