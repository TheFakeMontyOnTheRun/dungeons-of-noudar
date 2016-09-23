package br.odb.noudar;

import java.io.Serializable;

import br.odb.GL2JNILib;
import br.odb.droidlib.Renderable;
import br.odb.droidlib.Sprite;
import br.odb.droidlib.Vector2;

public abstract class Actor implements Renderable, Serializable {

	public enum Actions {MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT}

	final public int mId;
	final private Sprite visual;

	Vector2 previousPosition;
	final private Vector2 position;

	int healthPoints;
	final private int attackPoints;

	private boolean hasMovedSinceLastTurn = false;

	void notifyEndOfTurn() {
	}

	int getStateFrame() {
		return visual.getCurrentFrame();
	}

	public void attack(Actor actor) {

		this.healthPoints -= actor.attackPoints;

		setActiveStance();

		if (healthPoints <= 0) {
			setAsDead();
		}
	}

	public boolean isAlive() {
		return (healthPoints > 0);
	}

	Actor(int healthPoints, int attackPoints ) {
		super();
		mId = GL2JNILib.getNextId();
		position = new Vector2();
		visual = new Sprite();
		this.healthPoints = healthPoints;
		this.attackPoints = attackPoints;
	}

	public Vector2 getPosition() {
		return position;
	}

	public void setPosition(Vector2 myPos) {
		position.set(myPos);
	}

	public void act(Actions action) {

		switch (action) {
			case MOVE_UP:
				this.setPosition(getPosition().add(new Vector2(0, -1)));
				break;

			case MOVE_DOWN:
				this.setPosition(getPosition().add(new Vector2(0, 1)));
				break;

			case MOVE_LEFT:
				this.setPosition(getPosition().add(new Vector2(-1, 0)));
				break;

			case MOVE_RIGHT:
				this.setPosition(getPosition().add(new Vector2(1, 0)));
				break;
		}
		setActiveStance();
	}

	public void checkpointPosition() {
		previousPosition = new Vector2(getPosition());
	}

	public void undoMove() {
		setPosition(previousPosition);
	}

	public abstract String getChar();

	public String getStats() {
		return getChar() + "," + ((int) position.x) + "," + ((int) position.y) + "," + healthPoints + "|";
	}

	public void setRestedStance() {
		visual.setFrame(0);
	}

	public void setActiveStance() {
		hasMovedSinceLastTurn = true;
		visual.setFrame(1);
	}

	void setAsDead() {
		visual.setFrame(2);
	}
}