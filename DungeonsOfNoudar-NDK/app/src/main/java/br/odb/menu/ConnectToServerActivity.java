package br.odb.menu;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;

import java.util.Timer;
import java.util.TimerTask;

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
                    int index = data.replace("\n", "" ).indexOf( helper.playerId );
                    int y = ( index / 20 );
                    int x = (index % 20 );
                    ((EditText) findViewById(R.id.edtPosX)).setText("" + x );
                    ((EditText) findViewById(R.id.edtPosY)).setText("" + y );
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

        @Override
        public void setGameId(String gameId) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    ((EditText) findViewById(R.id.edtGameId)).setText("" + helper.gameId);
                }
            });
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_connect_to_server);
        findViewById(R.id.btnSend).setEnabled(true);

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


        findViewById(R.id.btnUp).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String posX = getPosX();
                String posY = "" + ( Integer.parseInt( getPosY() ) - 1 );

                helper.sendData(posX, posY, client);
            }
        });

        findViewById(R.id.btnDown).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String posX = getPosX();
                String posY = "" + ( Integer.parseInt( getPosY() ) + 1 );

                helper.sendData(posX, posY, client);
            }
        });

        findViewById(R.id.btnLeft).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String posX = "" + ( Integer.parseInt( getPosX() ) - 1 );
                String posY = getPosY();

                helper.sendData(posX, posY, client);
            }
        });

        findViewById(R.id.btnRight).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String posX = "" + ( Integer.parseInt( getPosX() ) + 1 );
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
