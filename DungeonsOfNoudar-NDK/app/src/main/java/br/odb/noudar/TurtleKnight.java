package br.odb.noudar;

public class TurtleKnight extends Knight {

    public TurtleKnight() {
        super(30, 6);
    }

    @Override
    public String getChar() {
        return String.valueOf(KnightsConstants.SPAWNPOINT_TURTLE);
    }

    @Override
    public GameViewGLES2.ETextures getTextureIndex() {
        return GameViewGLES2.ETextures.values()[ GameViewGLES2.ETextures.Turtle0.ordinal() + getStateFrame() ];
    }
}
