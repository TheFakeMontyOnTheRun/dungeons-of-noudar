package br.odb.noudar;

import br.odb.noudar.characters.Actor;

public abstract class Knight extends Actor {

    public boolean hasExited;

    protected Knight(int healthPoints, int attackPoints) {
        super(healthPoints, attackPoints);
    }

    public void setAsExited() {
        hasExited = true;
    }
}
