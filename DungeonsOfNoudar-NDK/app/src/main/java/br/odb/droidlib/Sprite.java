package br.odb.droidlib;

import java.io.Serializable;

import br.odb.noudar.GameViewGLES2;

public class Sprite implements Renderable, Serializable {
    private int currentFrame = 0;

    public void setFrame(int frame) {
        this.currentFrame = frame;
    }

    public int getCurrentFrame() {
        return currentFrame;
    }

    @Override
    public GameViewGLES2.ETextures getTextureIndex() {
        return GameViewGLES2.ETextures.None;
    }
}
