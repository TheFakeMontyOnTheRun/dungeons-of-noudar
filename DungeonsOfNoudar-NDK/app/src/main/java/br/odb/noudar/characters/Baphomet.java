package br.odb.noudar.characters;

import br.odb.droidlib.Vector2;
import br.odb.noudar.GameLevel;
import br.odb.noudar.GameLevelLoader;
import br.odb.noudar.GameViewGLES2;
import br.odb.noudar.Knight;
import br.odb.noudar.Monster;

public class Baphomet extends Monster {

    public Baphomet() {
        super(100, 1);
    }

    @Override
    public void updateTarget(GameLevel level) {
        Vector2 myPosition = getPosition();
        Vector2 scan = new Vector2();
        int newX;
        int newY;

        for (int x = -10; x < 10; ++x) {
            for (int y = -10; y < 10; ++y) {

                newX = (int) (x + myPosition.x);
                newY = (int) (y + myPosition.y);
                scan.x = newX;
                scan.y = newY;

                if (newX >= 0 && newY >= 0 && newX < GameLevel.MAP_SIZE
                        && newY < GameLevel.MAP_SIZE
                        && level.getTile(scan).getOccupant() instanceof Knight && !( (Knight)level.getTile(scan).getOccupant() ).hasExited) {

                    if (dealWith(level, x, y))
                        return;
                }
            }
        }
    }


    @Override
    public String getChar() {
        return String.valueOf(GameLevelLoader.FileMarkers.SPAWNPOINT_BAPHOMET);
    }

    @Override
    public GameViewGLES2.ETextures getTextureIndex() {
        return GameViewGLES2.ETextures.values()[ GameViewGLES2.ETextures.Boss0.ordinal() + getStateFrame() ];
    }
}
