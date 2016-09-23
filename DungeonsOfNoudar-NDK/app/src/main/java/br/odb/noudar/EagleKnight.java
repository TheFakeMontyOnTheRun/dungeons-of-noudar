package br.odb.noudar;

public class EagleKnight extends Knight {
    public EagleKnight() {
        super(25, 10);
    }

//    @Override
//    public String toString() {
//        return res.getText( R.string.falcon_knight ) + " - " + super.toString();
//    }

    @Override
    public String getChar() {
        return String.valueOf(KnightsConstants.SPAWNPOINT_EAGLE);
    }

    @Override
    public GameViewGLES2.ETextures getTextureIndex() {
        return GameViewGLES2.ETextures.values()[ GameViewGLES2.ETextures.Falcon0.ordinal() + getStateFrame() ];
    }
}
