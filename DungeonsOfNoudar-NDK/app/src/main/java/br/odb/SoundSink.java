package br.odb;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

import java.util.ArrayList;

/**
 * Created by monty on 15/10/16.
 */
public class SoundSink {

	private ArrayList<AudioTrack> soundPool = new ArrayList<>();
	private ArrayList<byte[]> soundData = new ArrayList<>();

	public int bufferData(  byte[] data, int sampleRate, int channels, int bits ) {
		int key = soundPool.size();

		int formatChannels;
		int formatBits;

		if ( channels == 1 ) {
			formatChannels = AudioFormat.CHANNEL_OUT_MONO;
		} else {
			formatChannels = AudioFormat.CHANNEL_OUT_STEREO;
		}

		if ( bits == 8 ) {
			formatBits = AudioFormat.ENCODING_PCM_8BIT;
		} else {
			formatBits = AudioFormat.ENCODING_PCM_16BIT;
		}


		int bufferSize = AudioTrack.getMinBufferSize(sampleRate, formatChannels, formatBits);

		AudioTrack player = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, formatChannels, formatBits, bufferSize, AudioTrack.MODE_STREAM);

		soundPool.add( player );
		soundData.add( data );

		return key;
	}

	public void play( int id, final float volumeLeft, final float volumeRight ) {

		final byte[] data = soundData.get( id );
		final AudioTrack player = soundPool.get( id );

			Thread thread = new Thread( new Runnable(){

				@Override
				public void run() {
					player.setVolume( ( volumeLeft + volumeRight ) * 0.5f );
					player.play();
					player.write(data, 0, data.length);;
					player.stop();
				}
			} );

			thread.start();
	}
}
