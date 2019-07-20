package br.odb;

import android.content.Context;

/**
 * Created by monty on 23/06/16.
 */
public class SoundManager {
    private final Context mContext;
//	private MediaPlayer mMusic;

    public SoundManager(Context context) {
        this.mContext = context;
    }

    private boolean mayEnableSound() {
        android.media.AudioManager am = (android.media.AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);

        switch (am.getRingerMode()) {
            case android.media.AudioManager.RINGER_MODE_SILENT:
            case android.media.AudioManager.RINGER_MODE_VIBRATE:
                return false;
            case android.media.AudioManager.RINGER_MODE_NORMAL:
                return true;
            default:
                return false;
        }
    }

    public void playMusic(int musicRes) {
        if (mayEnableSound()) {
//			mMusic = MediaPlayer.create( mContext, musicRes );
//			mMusic.start();
        }
    }

    public void stop() {
//		if ( mMusic != null ) {
//			mMusic.stop();
//			mMusic.release();
//			mMusic = null;
//		}
    }
}
