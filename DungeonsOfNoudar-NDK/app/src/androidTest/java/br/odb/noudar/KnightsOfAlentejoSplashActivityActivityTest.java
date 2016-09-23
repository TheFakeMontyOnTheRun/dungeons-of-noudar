package br.odb.noudar;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.support.test.espresso.matcher.BoundedMatcher;
import android.support.test.espresso.matcher.ViewMatchers;
import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;
import android.test.suitebuilder.annotation.LargeTest;
import android.widget.ImageView;

import org.hamcrest.Description;
import org.hamcrest.Matcher;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import br.odb.menu.KnightsOfAlentejoSplashActivity;

import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.action.ViewActions.click;
import static android.support.test.espresso.matcher.ViewMatchers.withId;
import static org.hamcrest.Matchers.not;

@RunWith(AndroidJUnit4.class)
@LargeTest
public class KnightsOfAlentejoSplashActivityActivityTest {

    @Rule
    public ActivityTestRule<KnightsOfAlentejoSplashActivity> mActivityRule = new ActivityTestRule<>(
            KnightsOfAlentejoSplashActivity.class);

    @Test
    public void verifyClickingOnButtonForFirstImageWillActuallyChangeTheImage() throws InterruptedException {

        //load some image first
        onView(ViewMatchers.withId(br.odb.noudar.R.id.btnCredits)).perform(click());



//        onView(withId(R.id.catImageView1))
//                .check(matches(not( doesImagesContainTheSamePixels(originalImage))));
    }

    //http://hitherejoe.com/testing-imageview-changes-android-espresso-automated-tests/
    public static Matcher doesImagesContainTheSamePixels( final Drawable drawable ) {
        return new BoundedMatcher(ImageView.class) {


            @Override
            public void describeTo(Description description) {
                description.appendText("is image the same as: ");
                description.appendValue(drawable);
            }

            @Override
            protected boolean matchesSafely(Object item) {
                ImageView view = (ImageView) item;
                Bitmap bitmapCompare = Bitmap.createBitmap(drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
                Drawable drawable = view.getDrawable();
                Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
                return bitmapCompare.sameAs(bitmap);
            }
        };
    };
}
