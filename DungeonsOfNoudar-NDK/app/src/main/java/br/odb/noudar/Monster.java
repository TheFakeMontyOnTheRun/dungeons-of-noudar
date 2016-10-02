/**
 *
 */
package br.odb.noudar;

import br.odb.droidlib.Tile;
import br.odb.noudar.characters.Actor;

/**
 * @author monty
 */
public abstract class Monster extends Actor {

    protected Monster(int healthPoints, int attackPoints) {
        super(healthPoints, attackPoints);
    }

    protected void updateTarget(GameLevel level) {
    }

    protected boolean dealWith(GameLevel level, int relX, int relY) {

        boolean moved = false;

        checkpointPosition();

        if (relY > 0) {
            act(Actions.MOVE_DOWN);
            moved = true;
        } else if (relY < 0) {
            act(Actions.MOVE_UP);
            moved = true;
        } else if (relX > 0) {
            act(Actions.MOVE_RIGHT);
            moved = true;
        } else if (relX < 0) {
            act(Actions.MOVE_LEFT);
            moved = true;
        }

        if (moved) {

            Tile loco = level.getTile(getPosition());

            if (!level.validPositionFor(this)) {

                if (!isAlive()) {
                    loco.setOccupant(this);
                    level.createSplatAt(getPosition());
                    return false;
                } else if (loco.getOccupant() instanceof Knight) {
                    Knight k = (Knight) loco.getOccupant();
                    if (!k.hasExited && k.isAlive()) {
                        k.attack(this);
	                    level.createSplatAt( k.getPosition());
                    }
                }
                this.undoMove();
            } else {
                loco = level.getTile(previousPosition);
                loco.setOccupant(null);
                loco = level.getTile(getPosition());
                loco.setOccupant(this);
            }

        }

        return moved;
    }
}
