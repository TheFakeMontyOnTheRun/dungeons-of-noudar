package br.odb.multiplayer.model;

import java.io.Serializable;

public class Player implements Serializable {

    /**
     *
     */
    private static final long serialVersionUID = -7223769425852477544L;
    public final int playerId;
    public final int gameId;

    public Player(int playerId, int gameId) {
        this.playerId = playerId;
        this.gameId = gameId;
    }
}
