package br.odb;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;

import junit.framework.Assert;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.internal.verification.api.VerificationMode;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import javax.microedition.khronos.opengles.GL10;

import br.odb.GL2JNILib;
import br.odb.menu.GameActivity;

import static junit.framework.Assert.assertFalse;
import static junit.framework.Assert.assertTrue;
import static org.mockito.Matchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.powermock.api.mockito.PowerMockito.verifyStatic;
import br.odb.GL2JNILib;
/**
 * To work on unit tests, switch the Test Artifact in the Build Variants view.
 */


@RunWith(PowerMockRunner.class)
@PrepareForTest({GL10.class})
public class TrivialTest {

    @Test
    public void testPushingEventWillEventuallyResultInItsExecution() {
//        System.setProperty("java.library.path", "/home/monty/gitcenter/dungeons-of-noudar/DungeonsOfNoudar-NDK/app/build/intermediates/binaries/debug/all/lib/x86");
//        System.loadLibrary("NdkGlue");

//        AssetManager mockAssetManager = mock(AssetManager.class);
//
//        Context context = mock(Context.class);
//        GameViewGLES2 view = new GameViewGLES2(context);
//        Assert.fail();
//        GL2JNILib.onCreate( mockAssetManager );
//        verifyStatic();
//        GL2JNILib.onDestroy();

//        view.onDestroy();
//        Assert.assertFalse( GL2JNILib.isAnimating() );

//        EventHandler handler = new EventHandler(0L);
//        final EventResultCallback mockCallback = mock( EventResultCallback.class );
//        final EventResponse mockResponse = mock( EventResponse.class );
//
//        AbstractAsyncEvent goodEventToPush = new AbstractAsyncEvent( mockCallback ) {
//            @Override
//            public void perform() {
//                reportSuccess(mockResponse);
//            }
//        };
//
//        handler.startHandling();
//        handler.pushEvent(goodEventToPush);
//        Thread.sleep(10L);
//        verify( mockCallback ).onSuccess(mockResponse);

    }

    @Test
    public void ensureFailedEventsWillReportTheirFailure()  {
//        EventHandler handler = new EventHandler(0L);
//        final EventResultCallback mockCallback = mock( EventResultCallback.class );
//
//        AbstractAsyncEvent goodEventToPush = new AbstractAsyncEvent( mockCallback ) {
//            @Override
//            public void perform() {
//                reportFailure();
//            }
//        };
//
//        handler.startHandling();
//        handler.pushEvent(goodEventToPush);
//        Thread.sleep(10L);
//        verify( mockCallback ).onFailure();
    }

    @Test
    public void verifyStoppingAndStartingTheEventHandling() throws InterruptedException {

//        final EventResultCallback mockCallback = mock( EventResultCallback.class );
//        final EventResponse mockResponse = mock( EventResponse.class );
//
//        AbstractAsyncEvent goodEventToPush = new AbstractAsyncEvent( mockCallback ) {
//            @Override
//            public void perform() {
//                reportSuccess(mockResponse);
//            }
//        };
//
//        AbstractAsyncEvent badEventToPush = new AbstractAsyncEvent( mockCallback ) {
//            @Override
//            public void perform() {
//                reportFailure();
//            }
//        };
//
//
//        EventHandler handler = new EventHandler(0L);
//        handler.pushEvent(goodEventToPush);
//        assertFalse(handler.isRunning());
//        Thread.sleep(10L);
//        verify( mockCallback, never() ).onSuccess(mockResponse);
//        handler.startHandling();
//        assertTrue(handler.isRunning());
//        Thread.sleep(10L);
//        verify( mockCallback ).onSuccess(mockResponse);
//        handler.stopHandling();
//        assertFalse(handler.isRunning());
//        Thread.sleep(10L);
//        handler.pushEvent(badEventToPush);
//        Thread.sleep(10L);
//        verify( mockCallback, never() ).onFailure();
    }
}