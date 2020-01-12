package br.odb.menu;

import android.app.Activity;
import android.content.res.Configuration;
import android.graphics.Typeface;
import android.os.Bundle;
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

public class GameActivity extends Activity implements AdapterView.OnItemSelectedListener {

    private ControllerHelper mControllerHelper;
    private GameViewGLES2 view;
    private boolean mHaveController;
    private TextView tvFaith;
    private Spinner spnItemSelector;
    ItemSelectionAdapter adapter;
    String[] pickedItems = new String[]{"t"};
    Typeface font;
    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

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
    }

    @Override
    protected void onPause() {
        super.onPause();

        view.onPause();
        view.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();

        view.onCreate(getAssets());
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
