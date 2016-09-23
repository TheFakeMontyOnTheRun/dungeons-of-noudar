package br.odb.droidlib;

import java.io.Serializable;

import br.odb.noudar.Actor;
import br.odb.noudar.GameViewGLES2;
import br.odb.noudar.Knight;

public class Tile implements Renderable, Serializable{
	private int kind;
	private boolean block;
	private Renderable occupant;
	private final GameViewGLES2.ETextures textureId;

	public boolean isBlock() {
		return block;
	}

	public void setBlock(boolean block) {
		this.block = block;
	}

	public int getKind() {
		return kind;
	}

	public void setKind(int kind) {
		this.kind = kind;
	}

	public Tile(int kind, GameViewGLES2.ETextures texture) {
		if (kind < 0) {
			kind = 0;
		}

		textureId = texture;
		setKind(kind);
	}

	@Override
	public GameViewGLES2.ETextures getTextureIndex() {

		if (occupant != null) {

			if (occupant instanceof Knight && ((Knight) occupant).hasExited) {
				return textureId;
			}

			return occupant.getTextureIndex();
		} else {
			return textureId;
		}
	}

	public Renderable getOccupant() {
		return occupant;
	}

	public void setOccupant(Actor actor) {
		occupant = actor;
	}

	public GameViewGLES2.ETextures getMapTextureIndex() {
		return textureId;
	}
}
