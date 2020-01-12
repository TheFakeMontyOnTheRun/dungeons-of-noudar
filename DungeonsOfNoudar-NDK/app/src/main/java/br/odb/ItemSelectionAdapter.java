package br.odb;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Typeface;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.HashMap;
import java.util.Map;

import br.odb.noudar.R;


/**
 * Created by monty on 13/08/16.
 */
public class ItemSelectionAdapter extends ArrayAdapter<String> {

	private final Typeface font;
	private final Map<String, Bitmap> bitmapForItem;
	private final Map<String, String> nameForItem;

	public ItemSelectionAdapter(Context context, String[] items, Typeface font) {
		super(context, R.layout.item_view, items);
		this.font = font;

		bitmapForItem = new HashMap<>();
		nameForItem = new HashMap<>();
		bitmapForItem.put("y", BitmapFactory.decodeResource( context.getResources(), R.drawable.crossbow ));
		bitmapForItem.put("t", BitmapFactory.decodeResource( context.getResources(), R.drawable.falcata ));
		bitmapForItem.put("v", BitmapFactory.decodeResource( context.getResources(), R.drawable.shield ));

		nameForItem.put("y",  "Crossbow of damnation" );
		nameForItem.put("t", "Sword of sorrow");
		nameForItem.put("v", "Shield of restoration" );

	}



	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		return getViewForKnight(parent, getItem(position ));
	}

	private View getViewForKnight(ViewGroup parent, String i) {
		LayoutInflater inflater = (LayoutInflater) getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);

		View v = inflater.inflate(R.layout.item_view, parent, false);

		((TextView)v.findViewById( R.id.tvItemName )).setText( nameForItem.get(i) );

		if (  !"t".equals(i) ) {
			int availabilityForItem = GL2JNILib.getAvailabilityForItem( i.charAt(0) );
			((TextView)v.findViewById( R.id.tvCapacity)).setText( "" + availabilityForItem + "/20");

		} else {
			((TextView)v.findViewById( R.id.tvCapacity)).setText( "" );
		}



		((ImageView)v.findViewById( R.id.ivItemIcon )).setImageBitmap(bitmapForItem.get( i ));




		return v;
	}

	@Override
	public View getDropDownView(int position, View convertView, ViewGroup parent) {
		return getViewForKnight(parent, getItem(position ));
	}
}
