package br.odb.noudar.characters;

import br.odb.noudar.GameLevelLoader;
import br.odb.noudar.GameViewGLES2;
import br.odb.noudar.Knight;

public class Crusader extends Knight {

    public Crusader() {
        super(40, 6);
    }

    @Override
    public String getChar() {
        return String.valueOf(GameLevelLoader.FileMarkers.SPAWNPOINT_CRUSADER);
    }

    @Override
    public GameViewGLES2.ETextures getTextureIndex() {
        return GameViewGLES2.ETextures.Crusader;
    }

    public int getHealth() {
        return this.healthPoints;
    }
}
