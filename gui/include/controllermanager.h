// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#ifndef CHIAKI_CONTROLLERMANAGER_H
#define CHIAKI_CONTROLLERMANAGER_H

#include <chiaki/controller.h>

#include <QObject>
#include <QSet>
#include <QMap>
#include <QString>

#ifdef CHIAKI_GUI_ENABLE_SDL_GAMECONTROLLER
#include <SDL.h>
#include <chiaki/orientation.h>
#endif

#define PS_TOUCHPAD_MAX_X 1920
#define PS_TOUCHPAD_MAX_Y 1079

class Controller;
class Settings;

class ControllerManager : public QObject
{
	Q_OBJECT

	friend class Controller;

	private:
#ifdef CHIAKI_GUI_ENABLE_SDL_GAMECONTROLLER
		QSet<SDL_JoystickID> available_controllers;
#endif
		QMap<int, Controller *> open_controllers;
		Settings *settings = nullptr;

		void ControllerClosed(Controller *controller);

	private slots:
		void UpdateAvailableControllers();
		void HandleEvents();
#ifdef CHIAKI_GUI_ENABLE_SDL_GAMECONTROLLER
		void ControllerEvent(SDL_Event evt);
#endif

	public:
		static ControllerManager *GetInstance();

		ControllerManager(QObject *parent = nullptr);
		~ControllerManager();

		QSet<int> GetAvailableControllers();
		Controller *OpenController(int device_id);

		// Settings must be provided once at startup (see main.cpp) so that
		// Controller instances can look up the user's real-gamepad button
		// mapping.
		void SetSettings(Settings *s) { settings = s; }
		Settings *GetSettings() { return settings; }

	signals:
		void AvailableControllersUpdated();
};

class Controller : public QObject
{
	Q_OBJECT

	friend class ControllerManager;

	private:
		Controller(int device_id, ControllerManager *manager);

#ifdef CHIAKI_GUI_ENABLE_SDL_GAMECONTROLLER
		void UpdateState(SDL_Event event);
		bool HandleButtonEvent(SDL_ControllerButtonEvent event);
		bool HandleAxisEvent(SDL_ControllerAxisEvent event);
		// Applies a button_mapping target (a ChiakiControllerButton bit, or
		// one of the ANALOG_BUTTON_L2/R2 constants) to `state`, whether it
		// came from a real button event or a trigger-as-digital-button axis.
		void ApplyMappedButton(int chiaki_target, bool pressed);
#if SDL_VERSION_ATLEAST(2, 0, 14)
		bool HandleSensorEvent(SDL_ControllerSensorEvent event);
		bool HandleTouchpadEvent(SDL_ControllerTouchpadEvent event);
#endif
#endif

		ControllerManager *manager;
		int id;
		ChiakiOrientationTracker orientation_tracker;
		ChiakiControllerState state;
		bool is_dualsense;

#ifdef CHIAKI_GUI_ENABLE_SDL_GAMECONTROLLER
		QMap<QPair<Sint64, Sint64>, uint8_t> touch_ids;
		SDL_GameController *controller;
		// source (SDL_GameControllerButton, or Settings::kTriggerLeftSource/
		// kTriggerRightSource for the two physical triggers) -> chiaki_button
		// (ChiakiControllerButton or ChiakiControllerAnalogButton), loaded once
		// at construction from Settings::GetControllerButtonMapping(). The
		// stick axes are not in here -- they're handled unconditionally in
		// HandleAxisEvent.
		QMap<int, int> button_mapping;
#endif

	public:
		~Controller();

		bool IsConnected();
		int GetDeviceID();
		QString GetName();
		ChiakiControllerState GetState();
		void SetRumble(uint8_t left, uint8_t right);
		void SetTriggerEffects(uint8_t type_left, const uint8_t *data_left, uint8_t type_right, const uint8_t *data_right);
		bool IsDualSense();

	signals:
		void StateChanged();
};

/* PS5 trigger effect documentation:
   https://controllers.fandom.com/wiki/Sony_DualSense#FFB_Trigger_Modes

   Taken from SDL2, licensed under the zlib license,
   Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>
   https://github.com/libsdl-org/SDL/blob/release-2.24.1/test/testgamecontroller.c#L263-L289
*/
typedef struct
{
    Uint8 ucEnableBits1;                /* 0 */
    Uint8 ucEnableBits2;                /* 1 */
    Uint8 ucRumbleRight;                /* 2 */
    Uint8 ucRumbleLeft;                 /* 3 */
    Uint8 ucHeadphoneVolume;            /* 4 */
    Uint8 ucSpeakerVolume;              /* 5 */
    Uint8 ucMicrophoneVolume;           /* 6 */
    Uint8 ucAudioEnableBits;            /* 7 */
    Uint8 ucMicLightMode;               /* 8 */
    Uint8 ucAudioMuteBits;              /* 9 */
    Uint8 rgucRightTriggerEffect[11];   /* 10 */
    Uint8 rgucLeftTriggerEffect[11];    /* 21 */
    Uint8 rgucUnknown1[6];              /* 32 */
    Uint8 ucLedFlags;                   /* 38 */
    Uint8 rgucUnknown2[2];              /* 39 */
    Uint8 ucLedAnim;                    /* 41 */
    Uint8 ucLedBrightness;              /* 42 */
    Uint8 ucPadLights;                  /* 43 */
    Uint8 ucLedRed;                     /* 44 */
    Uint8 ucLedGreen;                   /* 45 */
    Uint8 ucLedBlue;                    /* 46 */
} DS5EffectsState_t;

#endif // CHIAKI_CONTROLLERMANAGER_H
