package br.odb.noudar;

public abstract class Knight extends Actor {

    public boolean hasExited;

    Knight(int healthPoints, int attackPoints) {
        super(healthPoints, attackPoints);
    }

    @Override
    public String toString() {
        return Math.max( super.healthPoints, 0 ) + " HP";
    }

    public void setAsExited() {
        hasExited = true;
    }
}
