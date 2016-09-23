package br.odb.droidlib;

import java.io.Serializable;

public class Vector2 implements Serializable {
    public float x;
    public float y;

    /**
     * Creates a new instance of Vec2
     */
    public Vector2(int aX, int aY) {
        x = aX;
        y = aY;
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof Vector2) {
            Vector2 v = (Vector2) o;
            return v.x == x && v.y == y;
        } else {
            return false;
        }
    }

    public Vector2(Vector2 position) {
        x = position.x;
        y = position.y;
    }

    public Vector2() {
        x = 0;
        y = 0;
    }

    private Vector2(float x, float y) {
        set(x, y);
    }

    private void set(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public void set(Vector2 myPos) {
        set(myPos.x, myPos.y);
    }

    public Vector2 add(Vector2 other) {
        return new Vector2(x + other.x, y + other.y);
    }
}
