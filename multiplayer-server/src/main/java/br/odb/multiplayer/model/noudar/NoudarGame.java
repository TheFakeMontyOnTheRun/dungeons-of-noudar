package br.odb.multiplayer.model.noudar;

import br.odb.multiplayer.model.Game;
import br.odb.multiplayer.model.Player;

import java.io.IOException;
import java.io.OutputStream;
import java.util.HashMap;

public class NoudarGame extends Game {

    final static int kGameMapSize = 20;

    public final int[][] table = new int[kGameMapSize][];

    public NoudarGame(int gameId) {
        super(gameId, 1);

        for (int c = 0; c < kGameMapSize; ++c) {
            table[c] = new int[kGameMapSize];
        }
    }

    @Override
    public void checkForGameEnd() {
    }

    @Override
    synchronized
    public void sendMove(HashMap<String, String> params) {
        String x = params.get("x");
        String y = params.get("y");

        int decodedX = Integer.parseInt(x);
        int decodedY = Integer.parseInt(y);
        int playerId = Integer.parseInt(params.get("playerId"));

        System.out.println("receiving from " + playerId + " when current is  " + currentPlayerId);

        if (playerId == currentPlayerId) {
            System.out.println("placing position for " + playerId + " at " + decodedX + ", " + decodedY);


            for (int c = 0; c < kGameMapSize; ++c) {
                for (int d = 0; d < kGameMapSize; ++d) {
                    if (table[c][d] == playerId) {
                        table[c][d] = 0;
                    }
                }
            }
            table[decodedY][decodedX] = playerId;

            printGameState();

            setTheNextPlayerAsCurrent();
        }
    }

    @Override
    public int addNewPlayer() {
        int id = super.addNewPlayer();

        table[id][id] = id;

        return id;
    }

    synchronized
    public void writeState(OutputStream os) {

        StringBuilder sb = new StringBuilder("<?xml version='1.0'?><game><state>");


        try {

            for (int c = 0; c < kGameMapSize; ++c) {
                for (int d = 0; d < kGameMapSize; ++d) {
                    sb.append(table[c][d]);
                }
            }

            Player p = players.get(currentPlayerId);

            if (p != null) {
                sb.append("</state><current>");
                sb.append(currentPlayerId);
                sb.append("</current><winner>");
                sb.append(winnerPlayerId);
                sb.append("</winner></game>");
            }

            os.write(sb.toString().getBytes());
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private void printGameState() {
        System.out.println();
        for (int c = 0; c < kGameMapSize; ++c) {
            for (int d = 0; d < kGameMapSize; ++d) {
                System.out.print(table[c][d]);
            }
            System.out.println();
        }
        System.out.println("currentPlayerId " + currentPlayerId);
    }

    @Override
    public int getNumberOfRequiredPlayers() {
        return 2;
    }
}
