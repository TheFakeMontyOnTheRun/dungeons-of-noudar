package br.odb;

public interface MultiplayerClient {
    void receiveGameId(int gameId);

    void receivePlayerId(int playerId);

    void receiveTeamId(int teamId);

    void endOfTurn();

    void allowedToSendMove();

    int getAppId();

    int getUserId();
}
