package br.odb;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

/**
 * Created by monty on 20/11/16.
 */

public class MultiplayerHelper {

    public static interface OnRequestResult {
        public void onDataResulting(String data);
    }

    public static interface GameClient {
        public void setServerStateText(String data);

        void setCurrentPlayerId(String playerId);

        void setPlayerInitialPosition(String posX, String posY);

        void setGameId(String gameId);
    }

    public String gameId = "";
    public String playerId = "";
    public String currentPlayerId = "";

    public String readFully(InputStream inputStream, String encoding)
            throws IOException {
        return new String(readFully(inputStream), encoding);
    }

    private byte[] readFully(InputStream inputStream) throws IOException {

        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        byte[] buffer = new byte[1024];
        int length = 0;

        while ((length = inputStream.read(buffer)) != -1) {
            baos.write(buffer, 0, length);
        }

        return baos.toByteArray();
    }


    public void request(final String url, final OnRequestResult resultHandler) {

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    System.out.println("url: " + url);

                    URLConnection connection = null;

                    connection = new URL(url).openConnection();
                    connection.setRequestProperty("Accept-Charset", "utf8");
                    InputStream response = connection.getInputStream();

                    if (resultHandler != null) {
                        resultHandler.onDataResulting(readFully(response, "utf8"));
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }


    public String getServerUrl() {
        return "http://192.168.1.7:8080/multiplayer-server";
    }

    private void sendData(String posX, String posY, OnRequestResult handler) {
        String toSend = getServerUrl() + "/SendMove?gameId=" + gameId + "&playerId=" + playerId;
        toSend += "&x=" + posX;
        toSend += "&y=" + posY;

        request(toSend, handler);
    }

    public void connectToServer(final MultiplayerHelper.GameClient client) {

        request(getServerUrl() + "/GetGameId", new MultiplayerHelper.OnRequestResult() {
            @Override
            public void onDataResulting(String data) {
                playerId = data;
                gameId = "1";
                client.setGameId(gameId);
                client.setPlayerInitialPosition(data, data);
            }
        });
    }

    public void updateServerState(final MultiplayerHelper.GameClient client) {

        request(getServerUrl() + "/GetGameStatus?gameId=" + gameId + "&playerId=" + playerId, new MultiplayerHelper.OnRequestResult() {
            @Override
            public void onDataResulting(String data) {
                client.setServerStateText(data);
            }
        });
    }

    public void sendData(String posX, String posY, final MultiplayerHelper.GameClient client) {
        String toSend = getServerUrl() + "/SendMove?gameId=" + gameId + "&playerId=" + playerId;
        toSend += "&x=" + posX;
        toSend += "&y=" + posY;

        request(toSend, new MultiplayerHelper.OnRequestResult() {
            @Override
            public void onDataResulting(String data) {
                updateServerState(client);
            }
        });
    }
}
