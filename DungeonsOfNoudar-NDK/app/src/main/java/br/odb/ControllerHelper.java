package br.odb;

import android.content.Context;
import android.content.res.Configuration;
import android.view.InputDevice;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by monty on 27/09/16.
 */
public class ControllerHelper {


	private Context mContext;

	public ControllerHelper(Context context) {
		mContext = context;
	}

	public boolean hasGamepad() {
		return getGameControllerIds().size() > 0;
	}

	public boolean hasPhysicalKeyboard() {
		return mContext.getResources().getConfiguration().keyboard != Configuration.KEYBOARD_NOKEYS;
	}

	private List<Integer> getGameControllerIds() {
		List<Integer> gameControllerDeviceIds = new ArrayList<>();

		int[] deviceIds = InputDevice.getDeviceIds();
		for (int deviceId : deviceIds) {
			InputDevice dev = InputDevice.getDevice(deviceId);
			int sources = dev.getSources();

			if (((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)
					|| ((sources & InputDevice.SOURCE_JOYSTICK)
					== InputDevice.SOURCE_JOYSTICK)) {

				if (!gameControllerDeviceIds.contains(deviceId)) {
					gameControllerDeviceIds.add(deviceId);
				}
			}
		}

		return gameControllerDeviceIds;
	}
}
