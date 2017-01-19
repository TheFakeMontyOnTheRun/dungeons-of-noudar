package br.odb;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.support.test.espresso.action.ViewActions;
import android.support.test.espresso.matcher.BoundedMatcher;
import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;
import android.test.suitebuilder.annotation.LargeTest;
import android.widget.ImageView;

import org.hamcrest.Description;
import org.hamcrest.Matcher;
import org.hamcrest.Matchers;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import br.odb.GameViewGLES2;
import br.odb.menu.GameActivity;
import br.odb.menu.RootActivity;

import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.matcher.ViewMatchers.withClassName;

/**
 * Created by monty on 09/02/16.
 *
 * Interface tests can be fragile. I'm still researching how to make those stronger.
 *
 *
 */
@RunWith(AndroidJUnit4.class)
@LargeTest
public class GameActivityTest {

    @Rule
    public ActivityTestRule<RootActivity> mActivityRule = new ActivityTestRule<>(
            RootActivity.class);

    @Test
    public void verifySwipingLeftWillRotateThePlayerCharacter() throws InterruptedException {

//        ImageView catView1 = ((ImageView)mActivityRule.getActivity().findViewById( R.id.catImageView1) );
//
//        //load some image first
//        onView(withId(R.id.btnLoadCatImage1)).perform(click());
//
        Thread.sleep(3000L);
//
//        Drawable originalImage = catView1.getDrawable();
//

        onView( withClassName(Matchers.equalTo(GameViewGLES2.class.getName()))).perform(ViewActions.swipeLeft());

//
//        onView(withId(R.id.btnLoadCatImage1)).perform(click());
//
        Thread.sleep(3000L);
//
//        onView(withId(R.id.catImageView1))
//                .check(matches(not( doesImagesContainTheSamePixels(originalImage))));
    }

//    @Test
//    public void verifySwipingLeftWillRota() throws InterruptedException {
//
//        ImageView catView1 = ((ImageView)mActivityRule.getActivity().findViewById( R.id.catImageView2) );
//
//        //load some image first
//        onView(withId(R.id.btnLoadCatImage2)).perform(click());
//
//        Thread.sleep(3000L);
//
//        Drawable originalImage = catView1.getDrawable();
//
//        onView(withId(R.id.catImageView2))
//                .check(matches(doesImagesContainTheSamePixels(originalImage)));
//
//        onView(withId(R.id.btnLoadCatImage2)).perform(click());
//
//        Thread.sleep(3000L);
//
//        onView(withId(R.id.catImageView2))
//                .check(matches(not( doesImagesContainTheSamePixels(originalImage))));
//    }

//    @Test
//    public void verifyAutomaticCatChangeWillActuallyChangeThePhotoFromTimeToTime() throws InterruptedException {
//
//
//        ImageView catView3 = ((ImageView)mActivityRule.getActivity().findViewById( R.id.catImageView3) );
//
//        //Wait to make sure there is some image there first.
//        Thread.sleep(4000L);
//
//        Drawable originalImage = catView3.getDrawable();
//
//        Thread.sleep(4000L);
//
//        onView(withId(R.id.catImageView3))
//                .check(matches(not( doesImagesContainTheSamePixels(originalImage))));
//    }


    //http://hitherejoe.com/testing-imageview-changes-android-espresso-automated-tests/
//    public static Matcher doesImagesContainTheSamePixels( final Drawable drawable ) {
//        return new BoundedMatcher(ImageView.class) {
//
//
//            @Override
//            public void describeTo(Description description) {
//                description.appendText("is image the same as: ");
//                description.appendValue(drawable);
//            }
//
//            @Override
//            protected boolean matchesSafely(Object item) {
//                ImageView view = (ImageView) item;
//                Bitmap bitmapCompare = Bitmap.createBitmap(drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
//                Drawable drawable = view.getDrawable();
//                Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
//                return bitmapCompare.sameAs(bitmap);
//            }
//        };
//    };
}
