package br.odb.noudar;

import android.content.res.Resources;

import java.io.InputStream;
import java.util.Scanner;

import br.odb.menu.GameActivity;

public class GameLevelLoader {

    public static final int NUMBER_OF_LEVELS = 6;

    public static GameLevel loadLevel(int currentLevel, Resources res, GameActivity.GameDelegate delegate, GameViewGLES2.GameRenderer renderer) {

        InputStream in;
        switch (currentLevel) {
            case 1:
                in = res.openRawResource(R.raw.map_tiles1);
                break;
            case 2:
                in = res.openRawResource(R.raw.map_tiles2);
                break;
            case 3:
                in = res.openRawResource(R.raw.map_tiles3);
                break;
            case 4:
                in = res.openRawResource(R.raw.map_tiles4);
                break;
            case 5:
                in = res.openRawResource(R.raw.map_tiles5);
                break;
            case 6:
                in = res.openRawResource(R.raw.map_tiles6);
                break;

            default:
                in = res.openRawResource(R.raw.map_tiles0);
        }

        Scanner scanner = new Scanner( in );

	    int[][] map = new int[GameLevel.MAP_SIZE][];

        int c = 0;

        while ( scanner.hasNextLine() ) {
            String line = scanner.nextLine();

	        int[] mapLine = new int[ line.length() ];
	        int d = 0;

            for ( byte b : line.getBytes() ) {
                mapLine[d++] = b - '0';
            }
	        map[ c++ ] = mapLine;
        }

        return new GameLevel(map, currentLevel, delegate, renderer);
    }
}
