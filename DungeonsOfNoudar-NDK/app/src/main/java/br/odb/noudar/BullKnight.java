package br.odb.noudar;

public class BullKnight extends Knight {

    public BullKnight() {
        super(20, 14);
    }

//    @Override
//    public String toString() {
//        return res.getText( R.string.bull_knight ) + " - " + super.toString();
//    }

    @Override
    public String getChar() {
        return String.valueOf(KnightsConstants.SPAWNPOINT_BULL);
    }

    @Override
    public GameViewGLES2.ETextures getTextureIndex() {
        return GameViewGLES2.ETextures.values()[ GameViewGLES2.ETextures.Bull0.ordinal() + getStateFrame() ];
    }
}
