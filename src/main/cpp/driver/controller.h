#pragma once
#include "PSMoveClient_CAPI.h"
#include "constants.h"
#include "config.h"
#include "trackable_device.h"

#include <map>

namespace steamvrbridge {

	/*
		Base class for the controller configuration
	*/
	class ControllerConfig : public Config
	{
	public:
		static const int CONFIG_VERSION;

		ControllerConfig(const std::string &fnamebase = "ControllerConfig");

		virtual configuru::Config WriteToJSON();
		virtual bool ReadFromJSON(const configuru::Config &pt);

		void ReadEmulatedTouchpadAction(const configuru::Config &pt, const ePSMButtonID psButtonID);
		void WriteEmulatedTouchpadAction(configuru::Config &pt, const ePSMButtonID psButtonID);

	    bool is_valid;
	    long version;

		std::string override_model;

		// Used to map buttons to the emulated touchpad
		eEmulatedTrackpadAction ps_button_id_to_emulated_touchpad_action[k_PSMButtonID_Count];
	};

	/*
		Interface that defines what a controller is and can do in the context of this driver. 
	*/
	class Controller : public TrackableDevice {

		/*
			The following methods must be overridden in order for an implementing object to be considered
			a Controller object. They also offer guidelines as to how to they should generally be
			implemented while still	respecting each controllers implementation may be different.

			With regards to function naming conventions used, functions prefixed with 'Update' suggest
			both object state change and OpenVR/PSMS notification whereas functions prefixed with 'Set/Get'
			suggest object state change only.
		*/
	public:
		struct HapticState
		{
			vr::VRInputComponentHandle_t hapticComponentHandle;
			float pendingHapticDurationSecs;
			float pendingHapticAmplitude;
			float pendingHapticFrequency;
			std::chrono::time_point<std::chrono::high_resolution_clock> lastTimeRumbleSent;
			bool lastTimeRumbleSentValid;
		};

	public:

		/** Controller Interface */
		Controller();
		virtual ~Controller();
		
		bool CreateButtonComponent(ePSMButtonID button_id);
		bool CreateAxisComponent(ePSMAxisID axis_id);
		bool CreateHapticComponent(ePSMHapicID haptic_id);

		void UpdateButton(ePSMButtonID button_id, PSMButtonState button_state, double time_offset=0.0);
		void UpdateAxis(ePSMAxisID axis_id, float axis_value, double time_offset=0.0);
		void UpdateHaptics(const vr::VREvent_HapticVibration_t &hapticData);

		bool HasButton(ePSMButtonID button_id) const;
		bool HasAxis(ePSMAxisID axis_id) const;
		bool HasHapticState(ePSMHapicID haptic_id) const;

		bool GetButtonState(ePSMButtonID button_id, PSMButtonState &out_button_state) const;
		bool GetAxisState(ePSMAxisID axis_id, float &out_axis_value) const;
		HapticState * GetHapticState(ePSMHapicID haptic_id);

		// Returns the controller name used to look-up the input profile.
		virtual const char *GetControllerSettingsPrefix() const = 0;

		// Returns true if the controller has a PSM assigned the given ControllerID.
		virtual bool HasPSMControllerId(int ControllerID) const = 0;

		// Returns the PSM controller view.
		virtual const PSMController * GetPSMControllerView() const = 0;

		// Returns the PSM controller serial number.
		virtual std::string GetPSMControllerSerialNo() const = 0;

		// Returns the PSM controller type.
		virtual PSMControllerType GetPSMControllerType() const = 0;

		/** TrackableDevice Interface */
		vr::EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId) override;
		void Deactivate() override;

	protected:
		virtual ControllerConfig *AllocateControllerConfig() { return new ControllerConfig(); }

	private:
		struct ButtonState
		{
			vr::VRInputComponentHandle_t buttonComponentHandle;
			PSMButtonState lastButtonState;
		};

		struct AxisState
		{
			vr::VRInputComponentHandle_t axisComponentHandle;
			float lastAxisState;
		};

		// Component handle registered upon Activate() and called to update button/touch/axis/haptic events
		std::map<ePSMButtonID, ButtonState> m_buttonStates;
		std::map<ePSMAxisID, AxisState> m_axisStates;
		std::map<ePSMHapicID, HapticState> m_hapticStates;

	protected:
		ControllerConfig *m_config;
	};
}