package com.omni.craft

import android.app.Activity as AndroidActivity
import android.app.Application
import android.app.Dialog
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
import org.json.JSONArray

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
        val threadName = Thread.currentThread().name
        val entry = "[$timestamp] [$level] [$threadName]: $message\n"
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
                appendLine("Hata Alan Thread: ${thread.name}")
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
    external fun nativeInit(w: Int, h: Int, saveDir: String)
    external fun nativeResize(w: Int, h: Int)
    external fun nativeFrame(dt: Float)
    external fun nativeIsInitialized(): Boolean
    external fun nativeJoystick(x: Float, y: Float)
    external fun nativeCameraInput(dx: Float, dy: Float)
    external fun nativeTap(type: Int)
    external fun nativeDropItem()
    external fun nativeJump()
    external fun nativeSneak(on: Boolean)
    external fun nativeSprint(on: Boolean)
    external fun nativeSelectSlot(slot: Int)
    external fun nativeSaveWorld()
    external fun nativeGetInventory(): String
    external fun nativeDestroy()

    class GameRenderer(private val saveDir: String) : GLSurfaceView.Renderer {
        private var lastTimeNs = System.nanoTime()

        override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
            nativeSetupCrashHandler(CraftLogger.getLogDirPath())
            CraftLogger.info("GLES 3.2 Render Yuzeyi Hazirlandi.")
        }

        override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
            if (!nativeIsInitialized()) {
                nativeInit(width, height, saveDir)
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

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.attributes.preferredDisplayModeId =
                display?.supportedModes?.maxByOrNull { it.refreshRate }?.modeId ?: 0
        }

        val root = FrameLayout(this)
        val saveDir = File(filesDir, "world_data").apply { if (!exists()) mkdirs() }.absolutePath

        glView = GLSurfaceView(this).apply {
            setEGLContextClientVersion(3)
            setPreserveEGLContextOnPause(true)
            setRenderer(Engine.GameRenderer(saveDir))
            renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
        }
        root.addView(glView)

        setupMinecraftHUD(root)
        setContentView(root)

        root.post { applyFullScreen() }
    }

    private fun applyFullScreen() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.insetsController?.let { controller ->
                controller.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                controller.systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
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

    private fun setupMinecraftHUD(root: FrameLayout) {
        val hud = FrameLayout(this)

        // 1. Vektörel Nişangah (Crosshair)
        val crosshairV = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(200, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(6, 36).apply { gravity = Gravity.CENTER }
        }
        val crosshairH = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(200, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(36, 6).apply { gravity = Gravity.CENTER }
        }
        hud.addView(crosshairV)
        hud.addView(crosshairH)

        // 2. Sol Vektörel Joystick
        val joyBase = View(this).apply {
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(70, 255, 255, 255))
                setStroke(3, Color.argb(160, 200, 200, 200))
            }
            layoutParams = FrameLayout.LayoutParams(280, 280).apply {
                gravity = Gravity.BOTTOM or Gravity.START
                setMargins(60, 0, 0, 60)
            }
        }
        hud.addView(joyBase)

        // 3. Minecraft 9'lu Vektörel Hotbar
        val hotbarLayout = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER
            background = GradientDrawable().apply {
                setColor(Color.argb(140, 20, 20, 20))
                cornerRadius = 12f
                setStroke(3, Color.argb(180, 150, 150, 150))
            }
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                110
            ).apply {
                gravity = Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL
                bottomMargin = 24
            }
        }

        for (i in 0 until 9) {
            val slot = TextView(this).apply {
                text = "${i + 1}"
                textSize = 12f
                setTextColor(Color.WHITE)
                gravity = Gravity.CENTER
                background = GradientDrawable().apply {
                    setColor(if (i == 0) Color.argb(180, 100, 100, 100) else Color.argb(80, 50, 50, 50))
                    cornerRadius = 8f
                    setStroke(2, Color.argb(160, 200, 200, 200))
                }
                layoutParams = LinearLayout.LayoutParams(96, 96).apply {
                    setMargins(6, 6, 6, 6)
                }
                setOnClickListener {
                    Engine.nativeSelectSlot(i)
                    for (j in 0 until hotbarLayout.childCount) {
                        (hotbarLayout.getChildAt(j).background as GradientDrawable).setColor(
                            if (j == i) Color.argb(180, 100, 100, 100) else Color.argb(80, 50, 50, 50)
                        )
                    }
                }
            }
            hotbarLayout.addView(slot)
        }
        hud.addView(hotbarLayout)

        // 4. Sağ Aksiyon Butonları
        val rightPanel = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            ).apply {
                gravity = Gravity.BOTTOM or Gravity.END
                setMargins(0, 0, 50, 50)
            }
        }

        fun createBtn(title: String, color: Int, onClick: () -> Unit, onTouchAction: ((Boolean) -> Unit)? = null): Button {
            return Button(this).apply {
                text = title
                textSize = 13f
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    cornerRadius = 20f
                    setColor(color)
                    setStroke(2, Color.WHITE)
                }
                layoutParams = LinearLayout.LayoutParams(190, 95).apply {
                    setMargins(0, 8, 0, 8)
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

        rightPanel.addView(createBtn(getString(R.string.btn_break), Color.argb(180, 180, 40, 40), { Engine.nativeTap(0) }))
        rightPanel.addView(createBtn(getString(R.string.btn_place), Color.argb(180, 40, 140, 40), { Engine.nativeTap(1) }))
        rightPanel.addView(createBtn(getString(R.string.btn_drop), Color.argb(180, 180, 120, 20), { Engine.nativeDropItem() }))
        rightPanel.addView(createBtn(getString(R.string.btn_jump), Color.argb(180, 30, 80, 180), { Engine.nativeJump() }))
        hud.addView(rightPanel)

        // 5. Üst Duraklatma & Çanta Paneli
        val topBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            ).apply {
                gravity = Gravity.TOP or Gravity.END
                setMargins(0, 30, 50, 0)
            }
        }

        topBar.addView(createBtn(getString(R.string.btn_inv), Color.argb(160, 80, 80, 80), { showInventoryDialog() }))
        topBar.addView(createBtn(getString(R.string.btn_pause), Color.argb(160, 120, 40, 40), { showPauseMenu() }))
        hud.addView(topBar)

        // Dokunmatik Kontrol Katmanı
        hud.setOnTouchListener { _, event ->
            val pointerIndex = event.actionIndex
            val pId = event.getPointerId(pointerIndex)
            val x = event.getX(pointerIndex)
            val y = event.getY(pointerIndex)

            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                    if (x < root.width * 0.4f && y > root.height * 0.35f) {
                        updateJoystick(x, y, joyBase)
                    } else if (x > root.width * 0.35f && !isCamDragging) {
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
                        } else if (px < root.width * 0.4f) {
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

    private fun showInventoryDialog() {
        val dialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.argb(220, 20, 20, 20))
        }

        val title = TextView(this).apply {
            text = "ENVANTER (36 Slot)"
            textSize = 20f
            setTextColor(Color.WHITE)
            setPadding(0, 20, 0, 30)
        }
        layout.addView(title)

        val grid = GridLayout(this).apply {
            columnCount = 9
            rowCount = 4
        }

        try {
            val invData = JSONArray(Engine.nativeGetInventory())
            for (i in 0 until invData.length()) {
                val item = invData.getJSONObject(i)
                val count = item.getInt("count")
                val id = item.getInt("id")
                val slotView = TextView(this).apply {
                    text = if (count > 0) "ID:$id\nx$count" else ""
                    textSize = 10f
                    setTextColor(Color.YELLOW)
                    gravity = Gravity.CENTER
                    background = GradientDrawable().apply {
                        setColor(Color.argb(120, 60, 60, 60))
                        setStroke(2, Color.WHITE)
                        cornerRadius = 8f
                    }
                    layoutParams = GridLayout.LayoutParams().apply {
                        width = 110
                        height = 110
                        setMargins(6, 6, 6, 6)
                    }
                }
                grid.addView(slotView)
            }
        } catch (_: Exception) {}

        layout.addView(grid)

        val closeBtn = Button(this).apply {
            text = "Kapat"
            setOnClickListener { dialog.dismiss(); applyFullScreen() }
            layoutParams = LinearLayout.LayoutParams(250, 100).apply { topMargin = 30 }
        }
        layout.addView(closeBtn)

        dialog.setContentView(layout)
        dialog.show()
    }

    private fun showPauseMenu() {
        val dialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.argb(230, 15, 15, 15))
        }

        val title = TextView(this).apply {
            text = "OMNI CRAFT - DURAKLATILDI"
            textSize = 22f
            setTextColor(Color.WHITE)
            setPadding(0, 0, 0, 40)
        }
        layout.addView(title)

        fun createMenuBtn(txt: String, onClick: () -> Unit): Button {
            return Button(this).apply {
                text = txt
                textSize = 16f
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    cornerRadius = 16f
                    setColor(Color.argb(200, 50, 50, 50))
                    setStroke(2, Color.WHITE)
                }
                layoutParams = LinearLayout.LayoutParams(400, 110).apply {
                    setMargins(0, 12, 0, 12)
                }
                setOnClickListener { onClick() }
            }
        }

        layout.addView(createMenuBtn(getString(R.string.menu_resume)) {
            dialog.dismiss()
            applyFullScreen()
        })
        layout.addView(createMenuBtn(getString(R.string.menu_save)) {
            Engine.nativeSaveWorld()
            Toast.makeText(this, getString(R.string.save_success), Toast.LENGTH_SHORT).show()
        })
        layout.addView(createMenuBtn(getString(R.string.menu_exit)) {
            Engine.nativeSaveWorld()
            dialog.dismiss()
            finish()
        })

        dialog.setContentView(layout)
        dialog.show()
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) applyFullScreen()
    }

    override fun onResume() {
        super.onResume()
        glView.onResume()
        applyFullScreen()
    }

    override fun onPause() {
        super.onPause()
        glView.onPause()
        Engine.nativeSaveWorld()
    }

    override fun onDestroy() {
        super.onDestroy()
        Engine.nativeDestroy()
    }
}
