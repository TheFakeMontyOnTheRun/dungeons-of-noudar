package br.odb.menu;

import android.app.Application;
import android.content.Context;
import android.media.AudioManager;

public class NoudarApplication extends Application  {

	boolean mayEnableSound() {
		AudioManager am = (AudioManager)this.getSystemService(Context.AUDIO_SERVICE);

		if (am.getRingerMode() == AudioManager.RINGER_MODE_NORMAL) {
			return true;
		} else {
			return false;
		}
	}
}
