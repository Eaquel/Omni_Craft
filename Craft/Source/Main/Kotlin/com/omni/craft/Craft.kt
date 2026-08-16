package com.omni.craft

import android.app.Activity as AndroidActivity
import android.app.Application
import android.content.Context
import android.content.pm.ActivityInfo
import android.graphics.Color
import android.graphics.drawable.GradientDrawable
import android.opengl.GLSurfaceView
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.*
import android.widget.*
import java.io.File
import java.io.FileWriter
import java.io.PrintWriter
import java.io.StringWriter
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.concurrent.Executors
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class CraftApp : Application() {
    override fun onCreate() {
        super.onCreate()
        CraftLogger.init(this)
    }
}

object CraftLogger {
    private const val TAG = "OmniCraft"
    private val logExecutor = Executors.newSingleThreadExecutor()
    private var logDir: File? = null
    private val timeFormat = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault())
    private val fileFormat = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.getDefault())

    fun init(context: Context) {
        val docsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS)
        logDir = File(docsDir, "Craft_Log").apply {
            if (!exists()) mkdirs()
        }

        Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
            handleFatalException(thread, throwable)
        }

        info("CraftLogger baslatildi. Dizin: ${logDir?.absolutePath}")
    }

    fun getLogDirPath(): String = logDir?.absolutePath ?: ""

    fun info(message: String) = logToFile("INFO", message)
    fun warn(message: String) = logToFile("WARN", message)
    fun error(message: String, throwable: Throwable? = null) {
        val stackTrace = throwable?.let {
            val sw = StringWriter()
            it.printStackTrace(PrintWriter(sw))
            "\n" + sw.toString()
        } ?: ""
        logToFile("ERROR", "$message$stackTrace")
    }

    private fun logToFile(level: String, message: String) {
        val timestamp = timeFormat.format(Date())
        val entry = "[$timestamp] [$level] [${Thread.currentThread().name}]: $message\n"
        Log.println(if (level == "ERROR") Log.ERROR else Log.INFO, TAG, message)

        logExecutor.execute {
            try {
                logDir?.let { dir ->
                    val file = File(dir, "engine_runtime.log")
                    FileWriter(file, true).use { it.write(entry) }
                }
            } catch (e: Exception) {
                Log.e(TAG, "Log dosyasina yazilamadi: ${e.message}")
            }
        }
    }

    private fun handleFatalException(thread: Thread, throwable: Throwable) {
        try {
            val timestamp = fileFormat.format(Date())
            val crashFile = File(logDir, "crash_jvm_$timestamp.log")
            val sw = StringWriter()
            throwable.printStackTrace(PrintWriter(sw))

            val report = buildString {
                appendLine("================ OMNI CRAFT CRASH RAPORU ================")
                appendLine("Tarih: ${timeFormat.format(Date())}")
                appendLine("Cihaz: ${Build.MANUFACTURER} ${Build.MODEL} (Android ${Build.VERSION.RELEASE}, API ${Build.VERSION.SDK_INT})")
                appendLine("Hata Alan Thread: ${thread.name} (ID: ${thread.id})")
                appendLine("----------------- YIGIN IZI (STACKTRACE) -----------------")
                appendLine(sw.toString())
                appendLine("==========================================================")
            }

            FileWriter(crashFile, false).use { it.write(report) }
            Log.e(TAG, "Kritik JVM cokmesi kaydedildi: ${crashFile.absolutePath}")
        } catch (e: Exception) {
            Log.e(TAG, "Cokme logu kaydedilemedi: ${e.message}")
        }
    }
}

object Engine {
    init {
        System.loadLibrary("Omni_Craft")
    }

    external fun nativeSetupCrashHandler(logPath: String)
    external fun nativeInit(w: Int, h: Int, seed: Long)
    external fun nativeResize(w: Int, h: Int)
    external fun nativeFrame(dt: Float)
    external fun nativeIsInitialized(): Boolean
    external fun nativeJoystick(x: Float, y: Float)
    external fun nativeCameraInput(dx: Float, dy: Float)
    external fun nativeTap(type: Int)
    external fun nativeJump()
    external fun nativeSneak(on: Boolean)
    external fun nativeSprint(on: Boolean)
    external fun nativeFlyUp(on: Boolean)
    external fun nativeFlyDown(on: Boolean)
    external fun nativeStartBreak()
    external fun nativeStopBreak()
    external fun nativeDestroy()

    class GameRenderer : GLSurfaceView.Renderer {
        private var lastTimeNs = System.nanoTime()

        override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
            nativeSetupCrashHandler(CraftLogger.getLogDirPath())
            CraftLogger.info("GLES 3.2 Render Yuzeyi Olusturuldu.")
        }

        override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
            if (!nativeIsInitialized()) {
                nativeInit(width, height, 133742069L)
            } else {
                nativeResize(width, height)
            }
        }

        override fun onDrawFrame(gl: GL10?) {
            val now = System.nanoTime()
            val dt = ((now - lastTimeNs) / 1_000_000_000.0).toFloat().coerceIn(0.001f, 0.05f)
            lastTimeNs = now
            nativeFrame(dt)
        }
    }
}

class Activity : AndroidActivity() {
    private lateinit var glView: GLSurfaceView
    private var lastTouchX = 0f
    private var lastTouchY = 0f
    private var isCamDragging = false
    private var camPointerId = -1

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE
        hideSystemUI()

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.attributes.preferredDisplayModeId =
                display?.supportedModes?.maxByOrNull { it.refreshRate }?.modeId ?: 0
        }

        val root = FrameLayout(this)
        glView = GLSurfaceView(this).apply {
            setEGLContextClientVersion(3)
            setPreserveEGLContextOnPause(true)
            setRenderer(Engine.GameRenderer())
            renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
        }
        root.addView(glView)

        setupTurkishUI(root)
        setContentView(root)
    }

    private fun setupTurkishUI(root: FrameLayout) {
        val hud = FrameLayout(this)

        val joyBase = View(this).apply {
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(80, 255, 255, 255))
                setStroke(3, Color.argb(150, 200, 200, 200))
            }
            layoutParams = FrameLayout.LayoutParams(320, 320).apply {
                gravity = Gravity.BOTTOM or Gravity.START
                setMargins(70, 0, 0, 70)
            }
        }
        hud.addView(joyBase)

        val rightPanel = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            ).apply {
                gravity = Gravity.BOTTOM or Gravity.END
                setMargins(0, 0, 60, 60)
            }
        }

        fun createBtn(title: String, color: Int, onClick: () -> Unit, onTouchAction: ((Boolean) -> Unit)? = null): Button {
            return Button(this).apply {
                text = title
                textSize = 14f
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    cornerRadius = 24f
                    setColor(color)
                    setStroke(2, Color.WHITE)
                }
                layoutParams = LinearLayout.LayoutParams(220, 110).apply {
                    setMargins(0, 10, 0, 10)
                }
                if (onTouchAction != null) {
                    setOnTouchListener { _, event ->
                        when (event.actionMasked) {
                            MotionEvent.ACTION_DOWN -> { onTouchAction(true); true }
                            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> { onTouchAction(false); true }
                            else -> false
                        }
                    }
                } else {
                    setOnClickListener { onClick() }
                }
            }
        }

        rightPanel.addView(createBtn(getString(R.string.btn_break), Color.argb(180, 180, 40, 40), {}, { pressed ->
            if (pressed) Engine.nativeStartBreak() else Engine.nativeStopBreak()
        }))
        rightPanel.addView(createBtn(getString(R.string.btn_place), Color.argb(180, 40, 140, 40), { Engine.nativeTap(1) }))
        rightPanel.addView(createBtn(getString(R.string.btn_jump), Color.argb(180, 30, 80, 180), { Engine.nativeJump() }))
        hud.addView(rightPanel)

        val topBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            ).apply {
                gravity = Gravity.TOP or Gravity.CENTER_HORIZONTAL
                topMargin = 30
            }
        }

        topBar.addView(createBtn(getString(R.string.btn_sprint), Color.argb(160, 100, 100, 100), {}, { Engine.nativeSprint(it) }))
        topBar.addView(createBtn(getString(R.string.btn_sneak), Color.argb(160, 100, 100, 100), {}, { Engine.nativeSneak(it) }))
        topBar.addView(createBtn(getString(R.string.btn_fly_up), Color.argb(160, 50, 150, 200), {}, { Engine.nativeFlyUp(it) }))
        topBar.addView(createBtn(getString(R.string.btn_fly_down), Color.argb(160, 50, 150, 200), {}, { Engine.nativeFlyDown(it) }))
        hud.addView(topBar)

        hud.setOnTouchListener { _, event ->
            val pointerIndex = event.actionIndex
            val pId = event.getPointerId(pointerIndex)
            val x = event.getX(pointerIndex)
            val y = event.getY(pointerIndex)

            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                    if (x < root.width * 0.45f && y > root.height * 0.4f) {
                        updateJoystick(x, y, joyBase)
                    } else if (x > root.width * 0.4f && !isCamDragging) {
                        isCamDragging = true
                        camPointerId = pId
                        lastTouchX = x
                        lastTouchY = y
                    }
                }
                MotionEvent.ACTION_MOVE -> {
                    for (i in 0 until event.pointerCount) {
                        val currId = event.getPointerId(i)
                        val px = event.getX(i)
                        val py = event.getY(i)
                        if (currId == camPointerId) {
                            val dx = (px - lastTouchX) * 0.22f
                            val dy = (py - lastTouchY) * 0.22f
                            Engine.nativeCameraInput(dx, -dy)
                            lastTouchX = px
                            lastTouchY = py
                        } else if (px < root.width * 0.45f) {
                            updateJoystick(px, py, joyBase)
                        }
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                    if (pId == camPointerId) {
                        isCamDragging = false
                        camPointerId = -1
                    } else {
                        Engine.nativeJoystick(0f, 0f)
                    }
                }
                MotionEvent.ACTION_CANCEL -> {
                    isCamDragging = false
                    camPointerId = -1
                    Engine.nativeJoystick(0f, 0f)
                }
            }
            true
        }

        root.addView(hud)
    }

    private fun updateJoystick(touchX: Float, touchY: Float, base: View) {
        val centerX = base.x + base.width / 2f
        val centerY = base.y + base.height / 2f
        var dx = touchX - centerX
        var dy = touchY - centerY
        val maxRadius = base.width / 2f
        val dist = Math.hypot(dx.toDouble(), dy.toDouble()).toFloat()

        if (dist > maxRadius) {
            dx = (dx / dist) * maxRadius
            dy = (dy / dist) * maxRadius
        }
        Engine.nativeJoystick(dx / maxRadius, dy / maxRadius)
    }

    private fun hideSystemUI() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.insetsController?.apply {
                hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            )
        }
    }

    override fun onResume() {
        super.onResume()
        glView.onResume()
        hideSystemUI()
    }

    override fun onPause() {
        super.onPause()
        glView.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        Engine.nativeDestroy()
    }
}
