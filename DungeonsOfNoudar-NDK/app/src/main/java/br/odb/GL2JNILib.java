/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package br.odb;

// Wrapper for native library

import android.content.res.AssetManager;
import android.graphics.Bitmap;

public class GL2JNILib {

	private static int currentId = 0;

	public static int getNextId() {
		currentId++;

		return currentId;
	}


	static {
		System.loadLibrary("NdkGlue");
	}

	/**
	 * @param width  the current view width
	 * @param height the current view height
	 */
	public static native void init(int width, int height);

	public static native void step();

	public static native void onDestroy();

	public static native void onCreate(AssetManager assetManager);

	public static native void setTextures(Bitmap[] bitmaps);

	public static native void setMapWithSplatsAndActors( int[] map, int[] actors, int[] splats);

	public static native void setActorIdPositions( int[] ids);

	public static native void setCameraPosition(float x, float y);

	public static native void setCurrentCursorPosition(float x, float y);

	public static native void toggleCloseupCamera();

	public static native void setClearColour(float r, float g, float b);

	public static native void fadeIn();

	public static native void fadeOut();

	public static native void rotateLeft();

	public static native void rotateRight();

	public static native boolean isAnimating();

	public static native void setFloorNumber( long floor );

	public static native void tick( long delta );

	public static native void onReleasedLongPressingMove();

	public static native void onLongPressingMove();

	public static native void setEyeMatrix(float[] eyeView);

	public static native void setPerspectiveMatrix(float[] perspectiveMatrix) ;

	public static native void setXZAngle(float xz);

	public static native void setYZAngle(float yz);
}
