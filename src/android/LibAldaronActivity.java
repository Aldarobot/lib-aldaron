package com.libaldaron;

import android.app.*;
import android.os.*;
import android.widget.RelativeLayout;
import android.util.DisplayMetrics;
import android.view.*;
import android.util.Log;

import android.app.NativeActivity;

import java.lang.reflect.Field;

import com.millennialmedia.MMSDK;
import com.millennialmedia.MMLog;
import com.millennialmedia.InlineAd;
import com.millennialmedia.MMException;
import com.millennialmedia.UserData;
import com.millennialmedia.AppInfo;

public class LibAldaronActivity extends NativeActivity {

	// Layout
	private static View nativeLayout;
	protected static ViewGroup layout;

	// Ad Variables
	protected static InlineAd inlineAd;
	protected static UserData userData;
	protected static AppInfo appInfo;
	protected static View adContainer;

	// Native Functions
	public static native void nativeLaSetFiles(String data, String logfile);
	public static native void nativeLaFraction(float fraction);

	// Function to print to the logcat for | grep Aldaron
	public void printf(String whatToPrint) {
		Log.i("Aldaron", whatToPrint);
	}

	// Overridden to create ads and send external storage directory.
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		try {
			System.loadLibrary("SDL2");
			System.loadLibrary("SDL2_image");
			System.loadLibrary("smpeg2");
			System.loadLibrary("SDL2_mixer");
			System.loadLibrary("SDL2_net");
			System.loadLibrary("clump");
			System.loadLibrary("zip");
			System.loadLibrary("main");
		} catch(UnsatisfiedLinkError e) {
			printf(e.getMessage());
		} catch(Exception e) {
			printf(e.getMessage());
		}

		String state = Environment.getExternalStorageState();
		if (! Environment.MEDIA_MOUNTED.equals(state)) {
			printf("Failed: No Media.");
			return;
		}
		String base_dir = Environment.getExternalStorageDirectory()
			.getAbsolutePath() + "/lib-aldaron/";
		nativeLaSetFiles(base_dir, base_dir + "log.txt");
		// Call Parent onCreate()
		super.onCreate(savedInstanceState);
		MMSDK.initialize(this); // Ad Init
		MMLog.setLogLevel(Log.VERBOSE); // Verbose Log
		// Set User Data
		try{
			userData = new UserData();
			MMSDK.setUserData(userData);
		}catch(MMException e){
			printf("Ads couldn't set user data.");
		}
		// Set App Data
		try{
			appInfo = new AppInfo();
			MMSDK.setAppInfo(appInfo);
		}catch(MMException e){
			printf("Ads couldn't set app data.");
		}
		// Create inline ad
		adContainer = new RelativeLayout(this);
		try {
			inlineAd = InlineAd.createInstance("203888",
				(ViewGroup) adContainer);

			inlineAd.setListener(new InlineAd.InlineListener() {
				@Override
				public void onRequestSucceeded(InlineAd inlineAd) {
					printf("Inline Ad loaded.");
					adContainer.setVisibility(View.VISIBLE);
				}

				@Override
				public void onRequestFailed(InlineAd inlineAd,
					InlineAd.InlineErrorStatus errorStatus)
				{
					printf("Requeset Failed: "
						+ errorStatus.toString());
				}

				@Override
				public void onClicked(InlineAd inlineAd) {
					printf("Inline Ad clicked.");
				}

				@Override
				public void onResize(InlineAd inlineAd,
					int width, int height)
				{
					printf("Inline Ad starting resize.");
				}

				@Override
				public void onResized(InlineAd inlineAd,
					int width, int height,
					boolean toOriginalSize)
				{
					printf("Inline Ad resized.");
				}

				@Override
				public void onExpanded(InlineAd inlineAd) {
					printf("Inline Ad expanded.");
				}

				@Override
				public void onCollapsed(InlineAd inlineAd) {
					printf("Inline Ad collapsed.");
				}

				@Override
				public void onAdLeftApplication(InlineAd inlineAd) {
					printf("Inline Ad left application.");
				}

			});
		} catch (MMException e) {
			printf("Error creating inline ad.");
		}

		if (inlineAd != null) {
			// set refresh rate to 30 seconds.
//			inlineAd.setRefreshInterval(30000);

			final InlineAd.InlineAdMetadata inlineAdMetadata = new InlineAd.InlineAdMetadata().setAdSize(InlineAd.AdSize.BANNER);

			inlineAd.request(inlineAdMetadata);
		}

		// Get Ad Height ( 50 dp in fraction of screen size)
		DisplayMetrics dm = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(dm);
		int dens = dm.densityDpi;
		float hi = 1.0f / (float)dens;
		float hdp = hi * 160.0f;
		float ad_height = 50.0f / hdp;
		nativeLaFraction(ad_height);

		try {
			Field field = NativeActivity.class
				.getDeclaredField("mNativeContentView");
			field.setAccessible(true);
			nativeLayout = (View) field.get(this);
			field.setAccessible(false);
		} catch (NoSuchFieldException e) {
			throw new RuntimeException(e);
		} catch (IllegalAccessException e) {
			throw new RuntimeException(e);
		}


		// Set layout
		layout = new RelativeLayout(this);
		setContentView(layout);
		layout.addView(nativeLayout);
		layout.addView(adContainer,
			ViewGroup.LayoutParams.WRAP_CONTENT,
			(int) (dm.heightPixels * ad_height) );
	}
}
