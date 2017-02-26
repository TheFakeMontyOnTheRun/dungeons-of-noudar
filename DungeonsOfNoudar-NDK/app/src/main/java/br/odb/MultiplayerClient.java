package br.odb;

import org.w3c.dom.Node;

public interface MultiplayerClient {
    void receiveGameId(int gameId);

    void receivePlayerId(int playerId);

    void receiveTeamId(int teamId);

    void receiveMove(Node data);

    void endOfTurn();

    void allowedToSendMove();

    int getAppId();

    int getUserId();
}
