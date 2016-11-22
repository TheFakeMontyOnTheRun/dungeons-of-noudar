package br.odb.menu;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.Timer;
import java.util.TimerTask;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import br.odb.noudar.R;

public class ConnectToServerActivity extends Activity {

    String gameId = "";
    String playerId = "";
    String currentPlayerId = "";
    private final Timer mTimer = new Timer();

    public static interface OnRequestResult {
        public void onDataResulting(String data);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_connect_to_server);

        findViewById(R.id.btnConnect).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                connectToServer();
            }
        });

        findViewById(R.id.btnSend).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                sendData();
            }
        });

        connectToServer();

        mTimer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                updateServerState();
            }
        }, 2000L, 2000L);
    }

    String getPosX() {
        EditText edtData = (EditText) findViewById(R.id.edtPosX);
        return edtData.getText().toString();
    }

    String getPosY() {
        EditText edtData = (EditText) findViewById(R.id.edtPosY);
        return edtData.getText().toString();
    }

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


    private void request(final String url, final OnRequestResult resultHandler) {

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

    private void sendData() {
        String toSend = getServerUrl() + "/SendMove?gameId=" + gameId + "&playerId=" + playerId;
        toSend += "&x=" + getPosX();
        toSend += "&y=" + getPosY();

        request( toSend, new OnRequestResult() {
            @Override
            public void onDataResulting(String data) {
                updateServerState();
            }
        });
    }

    public Node getGameNodeFromURL(String url) throws ParserConfigurationException, SAXException, IOException {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        DocumentBuilder db = dbf.newDocumentBuilder();
        Document doc = db.parse(url);
        doc.getDocumentElement().normalize();

        NodeList nodeLst;
        nodeLst = doc.getElementsByTagName("game");
        return nodeLst.item(0);
    }

    public Node getGameNode(InputStream is) throws ParserConfigurationException, SAXException, IOException {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        DocumentBuilder db = dbf.newDocumentBuilder();
        Document doc = db.parse(is);
        doc.getDocumentElement().normalize();

        NodeList nodeLst;
        nodeLst = doc.getElementsByTagName("game");
        return nodeLst.item(0);
    }

    private void connectToServer() {

        request(getServerUrl() + "/GetGameId", new OnRequestResult() {
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


                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {

                            ((EditText) findViewById(R.id.edtPosX)).setText( "" + playerId );
                            ((EditText) findViewById(R.id.edtPosY)).setText( "" + playerId );

                            updateServerState();
                        }
                    });

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

    private void updateServerState() {

        request(getServerUrl() + "/GetGameStatus?gameId=" + gameId, new OnRequestResult() {
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
                            currentPlayerId = node.getChildNodes().item(0).getNodeValue().trim();
                        }
                    }

                } catch (ParserConfigurationException e) {
                    e.printStackTrace();
                } catch (SAXException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }


                setServerStateText(data);
            }
        });
    }

    private String getServerUrl() {
        return "http://192.168.1.5:8080/multiplayer-server";
    }


    private void setServerStateText(final String state) {

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                EditText edtServerState = (EditText) findViewById(R.id.edtServerState);
                edtServerState.setText(state);
                findViewById(R.id.btnSend).setEnabled( currentPlayerId.equals( playerId ) );
            }
        });
    }
}
