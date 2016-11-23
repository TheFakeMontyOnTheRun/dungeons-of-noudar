package br.odb;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;
import java.net.URL;
import java.net.URLConnection;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

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
        return "http://192.168.1.5:8080/multiplayer-server";
    }

    private void sendData( String posX, String posY, OnRequestResult handler ) {
        String toSend = getServerUrl() + "/SendMove?gameId=" + gameId + "&playerId=" + playerId;
        toSend += "&x=" + posX;
        toSend += "&y=" + posY;

        request( toSend, handler );
    }

    public void connectToServer(final MultiplayerHelper.GameClient client) {

        request( getServerUrl() + "/GetGameId", new MultiplayerHelper.OnRequestResult() {
            @Override
            public void onDataResulting(String data) {
                try {
                    DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
                    DocumentBuilder db = null;

                    db = dbf.newDocumentBuilder();
                    InputSource is = new InputSource(new StringReader(data));
                    Document doc = db.parse(is);
                    doc.getDocumentElement().normalize();

                    NodeList nodeLst;
                    nodeLst = doc.getElementsByTagName("game");
                    Node gameNode = nodeLst.item(0);
                    NodeList list = gameNode.getChildNodes();

                    Node node;

                    for (int c = 0; c < list.getLength(); ++c) {

                        node = list.item(c);

                        if ("playerId".equalsIgnoreCase(node.getNodeName())) {
                            playerId = node.getChildNodes().item(0).getNodeValue().trim();
                        } else if ("gameId".equalsIgnoreCase(node.getNodeName())) {
                            gameId = node.getChildNodes().item(0).getNodeValue().trim();
                        }
                    }

                    client.setPlayerInitialPosition( playerId, playerId );

                } catch (ParserConfigurationException e) {
                    e.printStackTrace();
                } catch (SAXException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }

            }
        });
    }

    public void updateServerState(final MultiplayerHelper.GameClient client) {

        request( getServerUrl() + "/GetGameStatus?gameId=" + gameId, new MultiplayerHelper.OnRequestResult() {
            @Override
            public void onDataResulting(String data) {


                try {
                    DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
                    DocumentBuilder db = null;

                    db = dbf.newDocumentBuilder();
                    InputSource is = new InputSource(new StringReader(data));
                    Document doc = db.parse(is);
                    doc.getDocumentElement().normalize();

                    NodeList nodeLst;
                    nodeLst = doc.getElementsByTagName("game");
                    Node gameNode = nodeLst.item(0);
                    NodeList list = gameNode.getChildNodes();

                    Node node;
                    String newState = "";

                    for (int c = 0; c < list.getLength(); ++c) {

                        node = list.item(c);

                        if ("state".equalsIgnoreCase(node.getNodeName())) {
                            data = node.getChildNodes().item(0).getNodeValue();
                            int p = 0;

                            for ( int y = 0; y < 20; ++y ) {
                                for ( int x = 0; x < 20; ++x ) {
                                    newState += data.charAt( p );
                                    ++p;
                                }
                                newState += '\n';
                            }

                            data = newState;
                        } else if ("current".equalsIgnoreCase(node.getNodeName())) {
                            client.setCurrentPlayerId(node.getChildNodes().item(0).getNodeValue().trim());
                        }
                    }

                } catch (ParserConfigurationException e) {
                    e.printStackTrace();
                } catch (SAXException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                client.setServerStateText(data);
            }
        });
    }

    public void sendData(String posX, String posY, final MultiplayerHelper.GameClient client) {
        String toSend = getServerUrl() + "/SendMove?gameId=" + gameId + "&playerId=" + playerId;
        toSend += "&x=" + posX;
        toSend += "&y=" + posY;

        request( toSend, new MultiplayerHelper.OnRequestResult() {
            @Override
            public void onDataResulting(String data) {
                updateServerState( client );
            }
        });
    }
}
