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

import br.odb.MultiplayerHelper;
import br.odb.noudar.R;

public class ConnectToServerActivity extends Activity {

    final MultiplayerHelper helper = new MultiplayerHelper();

    private final Timer mTimer = new Timer();

    final MultiplayerHelper.GameClient client = new MultiplayerHelper.GameClient() {

        @Override
        public void setServerStateText(final String data) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    EditText edtServerState = (EditText) findViewById(R.id.edtServerState);
                    edtServerState.setText(data);
                    findViewById(R.id.btnSend).setEnabled(helper.currentPlayerId.equals(helper.playerId));
                }
            });
        }

        @Override
        public void setCurrentPlayerId(String playerId) {
            helper.currentPlayerId = playerId;
        }

        @Override
        public void setPlayerInitialPosition(String posX, String posY) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {

                    ((EditText) findViewById(R.id.edtPosX)).setText("" + helper.playerId);
                    ((EditText) findViewById(R.id.edtPosY)).setText("" + helper.playerId);

                    helper.updateServerState(client);
                }
            });
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_connect_to_server);

        findViewById(R.id.btnConnect).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                helper.connectToServer(client);
            }
        });

        findViewById(R.id.btnSend).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String posX = getPosX();
                String posY = getPosY();

                helper.sendData(posX, posY, client);
            }
        });

        helper.connectToServer(client);

        mTimer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                helper.updateServerState(client);
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
}
