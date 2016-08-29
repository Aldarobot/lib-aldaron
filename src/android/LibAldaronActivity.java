package com.libaldaron;

import android.app.*;
import android.os.*;
import android.widget.RelativeLayout;
import android.widget.LinearLayout;
import android.util.DisplayMetrics;
import android.view.*;
import android.util.Log;
import android.content.Context;

import android.opengl.GLSurfaceView;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import com.millennialmedia.MMSDK;
import com.millennialmedia.MMLog;
import com.millennialmedia.InlineAd;
import com.millennialmedia.MMException;
import com.millennialmedia.UserData;
import com.millennialmedia.AppInfo;

public class LibAldaronActivity extends Activity {

	// Layout
	private static View aldaronView;
	protected static ViewGroup layout;

	// Ad Variables
	protected static InlineAd inlineAd;
	protected static UserData userData;
	protected static AppInfo appInfo;
	protected static View adContainer;

	// Native Functions
	public static native void nativeLaSetFiles(String data, String logfile);
	public static native void nativeLaFraction(float fraction);
	public static native void nativeLaDraw();
	public static native void nativeLaResize(int w, int h);

	// Function to print to the logcat for | grep Aldaron
	public static void printf(String whatToPrint) {
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
		adContainer = new LinearLayout(this);
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

		// Set layout
		aldaronView = new AldaronView(this);
		layout = new RelativeLayout(this);
		setContentView(layout);
		layout.addView(aldaronView);
		layout.addView(adContainer);//,
//			ViewGroup.LayoutParams.WRAP_CONTENT,
//			(int) (dm.heightPixels * ad_height) );
	}

	private static class AldaronView extends GLSurfaceView {
		public AldaronView(Context context) {
			super(context);
			setEGLContextFactory(new ContextFactory());
			setEGLConfigChooser(new ConfigChooser());
			setRenderer(new Renderer());
		}

		private static class ConfigChooser implements
			GLSurfaceView.EGLConfigChooser
		{
			protected int mRedSize;
			protected int mGreenSize;
			protected int mBlueSize;
			protected int mAlphaSize;
			protected int mDepthSize;
			protected int mStencilSize;
			private int[] mValue = new int[1];

			public ConfigChooser() {
				    mRedSize = 8;
				    mGreenSize = 8;
				    mBlueSize = 8;
				    mAlphaSize = 8;
				    mDepthSize = 8;
				    mStencilSize = 0;
			}

			/* This EGL config specification is used to specify 2.0 rendering.
			* We use a minimum size of 4 bits for red/green/blue, but will
			* perform actual matching in chooseConfig() below.
			*/
			private static int[] s_configAttribs2 =
			{
				EGL10.EGL_RED_SIZE, 8,
				EGL10.EGL_GREEN_SIZE, 8,
				EGL10.EGL_BLUE_SIZE, 8,
				EGL10.EGL_RENDERABLE_TYPE, 4,
				EGL10.EGL_NONE
			};

			public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

				/* Get the number of minimally matching EGL configurations
				*/
				int[] num_config = new int[1];
				egl.eglChooseConfig(display, s_configAttribs2, null, 0,
					num_config);

				int numConfigs = num_config[0];

				if (numConfigs <= 0) {
					throw new IllegalArgumentException(
						"No configs match configSpec");
				}

				/* Allocate then read the array of minimally matching
				 EGL configs */
				EGLConfig[] configs = new EGLConfig[numConfigs];
				egl.eglChooseConfig(display, s_configAttribs2, configs,
					numConfigs, num_config);

				/* Now return the "best" one
				*/
				return chooseConfig(egl, display, configs);
			}

			public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display,
				EGLConfig[] configs)
			{
				for(EGLConfig config : configs) {
					int d = findConfigAttrib(egl, display,
						config,EGL10.EGL_DEPTH_SIZE, 0);
					int s = findConfigAttrib(egl, display,
						config, EGL10.EGL_STENCIL_SIZE, 0);

					// We need at least mDepthSize and mStencilSize bits
					if (d < mDepthSize || s < mStencilSize)
						continue;

					// We want an *exact* match for red/green/blue/alpha
					int r = findConfigAttrib(egl, display,	
						config, EGL10.EGL_RED_SIZE, 0);
					int g = findConfigAttrib(egl, display,
						config, EGL10.EGL_GREEN_SIZE, 0);
					int b = findConfigAttrib(egl, display,
						config, EGL10.EGL_BLUE_SIZE, 0);
					int a = findConfigAttrib(egl, display,
						config, EGL10.EGL_ALPHA_SIZE, 0);

					if (r == mRedSize && g == mGreenSize &&
					 b == mBlueSize && a == mAlphaSize)
						return config;
				}
				return null;
			}

			private int findConfigAttrib(EGL10 egl, EGLDisplay display,
				EGLConfig config, int attribute, int defaultValue)
			{
				if (egl.eglGetConfigAttrib(display, config,
					attribute, mValue))
				{
					return mValue[0];
				}
				return defaultValue;
			}
		}

		private static class ContextFactory implements
			GLSurfaceView.EGLContextFactory
		{
			private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

			public EGLContext createContext(EGL10 egl,
				EGLDisplay display, EGLConfig eglConfig)
			{
				Log.w("Aldaron", "creating OpenGL ES 2.0 context");
				checkEglError("Before eglCreateContext", egl);
				int[] attrib_list =
					{EGL_CONTEXT_CLIENT_VERSION, 2,
					 EGL10.EGL_NONE };
				EGLContext context = egl.eglCreateContext(
					display, eglConfig,
					EGL10.EGL_NO_CONTEXT, attrib_list);
				checkEglError("After eglCreateContext", egl);
				return context;
			}

			public void destroyContext(EGL10 egl,
				EGLDisplay display, EGLContext context)
			{
				egl.eglDestroyContext(display, context);
			}
		}

		private static void checkEglError(String prompt, EGL10 egl) {
			int error;
			while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) {
			    Log.e("Aldaron", String.format("%s: EGL error", prompt));
			}
		}

		private static class Renderer implements GLSurfaceView.Renderer{
			public void onDrawFrame(GL10 gl) {
				nativeLaDraw();
			}

			public void onSurfaceChanged(GL10 gl, int w, int h) {
				printf("width = " + w + " height = " + h);
				if(w != 0 && h != 0) {
					printf("RESIZE");
					nativeLaResize(w, h);
				}
			}

			public void onSurfaceCreated(GL10 gl,EGLConfig config){}
		}
	}
}
