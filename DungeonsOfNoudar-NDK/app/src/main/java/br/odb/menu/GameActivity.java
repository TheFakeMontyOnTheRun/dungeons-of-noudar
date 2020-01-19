package br.odb.menu;

import android.app.Activity;
import android.content.res.Configuration;
import android.graphics.Typeface;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;

import br.odb.ControllerHelper;
import br.odb.GL2JNILib;
import br.odb.GameViewGLES2;
import br.odb.ItemSelectionAdapter;
import br.odb.noudar.R;

import static android.view.View.SYSTEM_UI_FLAG_FULLSCREEN;
import static android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
import static android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;

public class GameActivity extends Activity implements AdapterView.OnItemSelectedListener {

    private ControllerHelper mControllerHelper;
    private GameViewGLES2 view;
    private boolean mHaveController;
    private TextView tvFaith;
    private Spinner spnItemSelector;
    ItemSelectionAdapter adapter;
    private SoundPool soundPool;

    String[] pickedItems = new String[]{"t"};
    Typeface font;
    private int[] sounds = new int[8];
    boolean shouldInitAudio = true;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        shouldInitAudio = getIntent().getExtras().getString("hasSound").equals("y");

        mControllerHelper = new ControllerHelper(this);

        configureUiForInputDevice();

        setContentView(R.layout.game3d_layout);

        font = Typeface.createFromAsset(getAssets(), "fonts/MedievalSharp.ttf");

        if (mHaveController ) {
            if(getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE){
                findViewById(R.id.rlLeftPanel).setVisibility(View.GONE);
                findViewById(R.id.rlRightPanel).setVisibility(View.GONE);
            } else {
                findViewById(R.id.llControllers).setVisibility(View.GONE);
            }
        }

        tvFaith = (TextView)findViewById(R.id.tvFaith );

        view = findViewById(R.id.gameView1);

        view.init(this, 0, mHaveController);

        if (Build.VERSION.SDK_INT >= 29) {
            enterStickyImmersiveMode();
        }

        findViewById(R.id.btnUp).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.moveUp();
            }
        });

        findViewById(R.id.btnDown).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.moveDown();
            }
        });

        findViewById(R.id.btnLeft).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.rotateLeft();
            }
        });

        findViewById(R.id.btnRight).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.rotateRight();
            }
        });

        findViewById(R.id.btnAttack).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.useItem();
            }
        });

        findViewById(R.id.btnPick).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                GL2JNILib.pickItem();
            }
        });

        spnItemSelector = (Spinner)findViewById(R.id.spnCurrentItem);



        adapter = new ItemSelectionAdapter(
                this,
                pickedItems, font);

        spnItemSelector.setAdapter(adapter);

        spnItemSelector.setOnItemSelectedListener(this);


        new Thread(new Runnable() {
            @Override
            public void run() {
                while ( true )
                try {
                    //HAS TO BE CALLED BEFORE GET FAITH!
                    final int tint = GL2JNILib.getTint();
                    final String faith = "Faith: " + GL2JNILib.getFaith();
                    int selectedItemIndex = 0;
                    final ArrayList<String> items = new ArrayList<>();

                    char selectedItem = GL2JNILib.getCurrentItem();

                    items.add("t" );

                    if (GL2JNILib.hasItem('y')) {
                        items.add("y" );
                        if ( selectedItem == 'y') {
                            selectedItemIndex = 1;
                        }
                    }

                    if (GL2JNILib.hasItem('v')) {
                        items.add("v" );
                        if ( selectedItem == 'v') {
                            selectedItemIndex = 2;
                        }
                    }

                    final int finalSelectedItemIndex = selectedItemIndex;
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            if ( items.size() != pickedItems.length ) {
                                synchronized (spnItemSelector) {
                                    pickedItems = items.toArray(pickedItems);
                                    adapter = new ItemSelectionAdapter(
                                            GameActivity.this,
                                            pickedItems, font);

                                    spnItemSelector.setAdapter(adapter);
                                }
                            } else {
                                adapter.notifyDataSetChanged();
                            }
                            spnItemSelector.setSelection(finalSelectedItemIndex, true);

                            tvFaith.setText(faith);
                            if ( tint < 0 ) {
                                Toast.makeText(GameActivity.this, "ooph!", Toast.LENGTH_SHORT).show();
                            } else if ( tint > 0 ) {
                                Toast.makeText(GameActivity.this, "Well!", Toast.LENGTH_SHORT).show();
                            }
                         }
                    });
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }).start();

        GL2JNILib.setAssets(getAssets());

        if ( savedInstanceState == null ) {
            GL2JNILib.onCreate();
        } else {
            GL2JNILib.tick(1);
        }
    }

    private void initAudio() {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
        soundPool =
            new SoundPool.Builder().setAudioAttributes(new AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_GAME)
                    .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                    .build()).build();
        } else {
            soundPool = new SoundPool(5, AudioManager.STREAM_MUSIC, 0 );
        }

        sounds[0] = soundPool.load(this, R.raw.grasssteps, 1);
        sounds[1] = soundPool.load(this, R.raw.stepsstones, 1);
        sounds[2] = soundPool.load(this, R.raw.stepsstones, 1);
        sounds[3] = soundPool.load(this, R.raw.monsterdamage, 1);
        sounds[4] = soundPool.load(this, R.raw.playerdamage, 1);
        sounds[5] = soundPool.load(this, R.raw.swing, 1);
        sounds[6] = soundPool.load(this, R.raw.monsterdead, 1);
        sounds[7] = soundPool.load(this, R.raw.playerdead, 1);
    }

    private void enterStickyImmersiveMode() {
        getWindow().getDecorView().setSystemUiVisibility(SYSTEM_UI_FLAG_IMMERSIVE_STICKY | SYSTEM_UI_FLAG_FULLSCREEN | SYSTEM_UI_FLAG_HIDE_NAVIGATION);
    }

    @Override
    protected void onPause() {
        super.onPause();

        view.onPause();
        view.onDestroy();

        if (soundPool != null ) {
            soundPool.release();
        }
    }


    @Override
    protected void onResume() {
        super.onResume();

        if (shouldInitAudio) {
            initAudio();

            new Thread(new Runnable() {
                @Override
                public void run() {
                    while (true) {
                        int sound = GL2JNILib.getSoundToPlay();
                        if ( sound >= 0 && sound <= 8 ) {
                            Log.d("Noudar", "Sound to play: " + sound );
                            soundPool.play(sounds[sound], 1f, 1f, 0, 0, 1f);
                        }
                        try {
                            Thread.sleep(10);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }).start();
        }

        view.onResume();

    }
    //presentation and interaction

    private void configureUiForInputDevice() {
        mHaveController = mControllerHelper.hasGamepad() || mControllerHelper.hasPhysicalKeyboard();
    }

    @Override
    public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
        GL2JNILib.setCurrentItem(pickedItems[i].charAt(0));
    }

    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {

    }
}
