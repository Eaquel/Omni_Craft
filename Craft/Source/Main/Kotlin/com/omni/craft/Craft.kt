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

    fun init(context: Context) {
        val docsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS)
        logDir = File(docsDir, "Craft_Log").apply { if (!exists()) mkdirs() }
        Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
            val sw = StringWriter()
            throwable.printStackTrace(PrintWriter(sw))
            Log.e(TAG, "Fatal Crash: $sw")
        }
    }

    fun getLogDirPath(): String = logDir?.absolutePath ?: ""
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
    external fun nativeInput(x: Float, z: Float)
    external fun nativeCameraInput(dx: Float, dy: Float)
    external fun nativeTap(type: Int)
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
            val dt = ((now - lastTimeNs) / 1_000_000_000.0).toFloat().coerceIn(0.001f, 0.04f)
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

    // D-Pad Hareket Durumu
    private var moveFwd = false
    private var moveBack = false
    private var moveLeft = false
    private var moveRight = false

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

        setupMCPEHUD(root)
        setContentView(root)
        root.post { applyFullScreen() }
    }

    private fun applyFullScreen() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.insetsController?.let {
                it.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                it.systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
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

    private fun updateMovement() {
        var x = 0f
        var z = 0f
        if (moveFwd) z += 1.0f
        if (moveBack) z -= 1.0f
        if (moveLeft) x -= 1.0f
        if (moveRight) x += 1.0f
        Engine.nativeInput(x, z)
    }

    private fun setupMCPEHUD(root: FrameLayout) {
        val hud = FrameLayout(this)

        // 1. Crosshair
        val chV = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(180, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(4, 30).apply { gravity = Gravity.CENTER }
        }
        val chH = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(180, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(30, 4).apply { gravity = Gravity.CENTER }
        }
        hud.addView(chV)
        hud.addView(chH)

        // 2. Sol Klasik MCPE D-Pad
        val dpad = RelativeLayout(this).apply {
            layoutParams = FrameLayout.LayoutParams(320, 320).apply {
                gravity = Gravity.BOTTOM or Gravity.START
                setMargins(40, 0, 0, 40)
            }
        }

        fun createDpadBtn(text: String, rule: Int): Button {
            return Button(this).apply {
                setText(text)
                textSize = 16f
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    setColor(Color.argb(120, 40, 40, 40))
                    cornerRadius = 12f
                    setStroke(2, Color.argb(150, 180, 180, 180))
                }
                layoutParams = RelativeLayout.LayoutParams(100, 100).apply {
                    addRule(rule)
                    if (rule == RelativeLayout.ALIGN_PARENT_TOP || rule == RelativeLayout.ALIGN_PARENT_BOTTOM) {
                        addRule(RelativeLayout.CENTER_HORIZONTAL)
                    } else {
                        addRule(RelativeLayout.CENTER_VERTICAL)
                    }
                }
            }
        }

        val btnUp = createDpadBtn("▲", RelativeLayout.ALIGN_PARENT_TOP).apply {
            setOnTouchListener { _, e ->
                moveFwd = (e.action == MotionEvent.ACTION_DOWN || e.action == MotionEvent.ACTION_MOVE)
                updateMovement(); true
            }
        }
        val btnDown = createDpadBtn("▼", RelativeLayout.ALIGN_PARENT_BOTTOM).apply {
            setOnTouchListener { _, e ->
                moveBack = (e.action == MotionEvent.ACTION_DOWN || e.action == MotionEvent.ACTION_MOVE)
                updateMovement(); true
            }
        }
        val btnLeft = createDpadBtn("◀", RelativeLayout.ALIGN_PARENT_START).apply {
            setOnTouchListener { _, e ->
                moveLeft = (e.action == MotionEvent.ACTION_DOWN || e.action == MotionEvent.ACTION_MOVE)
                updateMovement(); true
            }
        }
        val btnRight = createDpadBtn("▶", RelativeLayout.ALIGN_PARENT_END).apply {
            setOnTouchListener { _, e ->
                moveRight = (e.action == MotionEvent.ACTION_DOWN || e.action == MotionEvent.ACTION_MOVE)
                updateMovement(); true
            }
        }

        dpad.addView(btnUp)
        dpad.addView(btnDown)
        dpad.addView(btnLeft)
        dpad.addView(btnRight)
        hud.addView(dpad)

        // 3. Sağ MCPE Zıplama & Eğilme ve Hızlı Eylem Butonları
        val rightActionArea = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            ).apply {
                gravity = Gravity.BOTTOM or Gravity.END
                setMargins(0, 0, 40, 40)
            }
        }

        fun createRoundActionBtn(txt: String, size: Int, color: Int, onAction: () -> Unit): Button {
            return Button(this).apply {
                text = txt
                textSize = 12f
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    shape = GradientDrawable.OVAL
                    setColor(color)
                    setStroke(2, Color.WHITE)
                }
                layoutParams = LinearLayout.LayoutParams(size, size).apply {
                    setMargins(8, 8, 8, 8)
                }
                setOnClickListener { onAction() }
            }
        }

        // Kır & Koy & Zıpla
        rightActionArea.addView(createRoundActionBtn("KIR", 120, Color.argb(150, 180, 50, 50)) { Engine.nativeTap(0) })
        rightActionArea.addView(createRoundActionBtn("KOY", 120, Color.argb(150, 50, 150, 50)) { Engine.nativeTap(1) })
        rightActionArea.addView(createRoundActionBtn("⯅", 130, Color.argb(150, 70, 70, 70)) { Engine.nativeJump() })
        hud.addView(rightActionArea)

        // 4. MCPE Orijinal 9'lu Hotbar ve "..." Envanter Butonu
        val hotbarContainer = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            ).apply {
                gravity = Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL
                bottomMargin = 16
            }
        }

        val hotbarSlots = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            background = GradientDrawable().apply {
                setColor(Color.argb(160, 20, 20, 20))
                cornerRadius = 8f
                setStroke(2, Color.argb(180, 100, 100, 100))
            }
            setPadding(4, 4, 4, 4)
        }

        val blockNames = arrayOf("Tahta", "Kırıktaş", "Elmas", "Masa", "Meşale", "6", "7", "8", "9")

        for (i in 0 until 9) {
            val slot = TextView(this).apply {
                text = blockNames[i]
                textSize = 9f
                setTextColor(Color.WHITE)
                gravity = Gravity.CENTER
                background = GradientDrawable().apply {
                    setColor(if (i == 0) Color.argb(180, 120, 120, 120) else Color.argb(60, 0, 0, 0))
                    cornerRadius = 6f
                    setStroke(if (i == 0) 3 else 1, if (i == 0) Color.WHITE else Color.GRAY)
                }
                layoutParams = LinearLayout.LayoutParams(90, 90).apply {
                    setMargins(3, 3, 3, 3)
                }
                setOnClickListener {
                    Engine.nativeSelectSlot(i)
                    for (j in 0 until hotbarSlots.childCount) {
                        (hotbarSlots.getChildAt(j).background as GradientDrawable).apply {
                            setColor(if (j == i) Color.argb(180, 120, 120, 120) else Color.argb(60, 0, 0, 0))
                            setStroke(if (j == i) 3 else 1, if (j == i) Color.WHITE else Color.GRAY)
                        }
                    }
                }
            }
            hotbarSlots.addView(slot)
        }
        hotbarContainer.addView(hotbarSlots)

        // MCPE "..." Envanter Butonu
        val invBtn = Button(this).apply {
            text = "•••"
            textSize = 14f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(160, 35, 35, 35))
                cornerRadius = 8f
                setStroke(2, Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(90, 90).apply {
                marginStart = 12
            }
            setOnClickListener { showInventoryDialog() }
        }
        hotbarContainer.addView(invBtn)
        hud.addView(hotbarContainer)

        // 5. Üst Duraklatma Butonu
        val pauseBtn = Button(this).apply {
            text = "II"
            textSize = 14f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(140, 40, 40, 40))
                cornerRadius = 8f
                setStroke(1, Color.WHITE)
            }
            layoutParams = FrameLayout.LayoutParams(80, 80).apply {
                gravity = Gravity.TOP or Gravity.CENTER_HORIZONTAL
                topMargin = 20
            }
            setOnClickListener { showPauseMenu() }
        }
        hud.addView(pauseBtn)

        // Dokunmatik Kamera Kaydırma Katmanı
        hud.setOnTouchListener { _, event ->
            val pIdx = event.actionIndex
            val pId = event.getPointerId(pIdx)
            val x = event.getX(pIdx)
            val y = event.getY(pIdx)

            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                    // Sadece sağ ekranda boş alana basınca kamera sürükle
                    if (x > root.width * 0.35f && !isCamDragging) {
                        isCamDragging = true
                        camPointerId = pId
                        lastTouchX = x
                        lastTouchY = y
                    }
                }
                MotionEvent.ACTION_MOVE -> {
                    for (i in 0 until event.pointerCount) {
                        if (event.getPointerId(i) == camPointerId) {
                            val px = event.getX(i)
                            val py = event.getY(i)
                            val dx = (px - lastTouchX) * 0.22f
                            val dy = (py - lastTouchY) * 0.22f
                            Engine.nativeCameraInput(dx, -dy)
                            lastTouchX = px
                            lastTouchY = py
                        }
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                    if (pId == camPointerId) {
                        isCamDragging = false
                        camPointerId = -1
                    }
                }
                MotionEvent.ACTION_CANCEL -> {
                    isCamDragging = false
                    camPointerId = -1
                }
            }
            true
        }

        root.addView(hud)
    }

    private fun showInventoryDialog() {
        val dialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.argb(230, 25, 25, 25))
        }

        val title = TextView(this).apply {
            text = "ENVANTER"
            textSize = 20f
            setTextColor(Color.WHITE)
            setPadding(0, 20, 0, 20)
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
                        width = 105
                        height = 105
                        setMargins(4, 4, 4, 4)
                    }
                }
                grid.addView(slotView)
            }
        } catch (_: Exception) {}

        layout.addView(grid)

        val closeBtn = Button(this).apply {
            text = "Kapat"
            setOnClickListener { dialog.dismiss(); applyFullScreen() }
            layoutParams = LinearLayout.LayoutParams(250, 90).apply { topMargin = 20 }
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
            setBackgroundColor(Color.argb(235, 20, 20, 20))
        }

        val title = TextView(this).apply {
            text = "OYUN DURAKLATILDI"
            textSize = 22f
            setTextColor(Color.WHITE)
            setPadding(0, 0, 0, 30)
        }
        layout.addView(title)

        fun createMenuBtn(txt: String, onClick: () -> Unit): Button {
            return Button(this).apply {
                text = txt
                textSize = 15f
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    cornerRadius = 12f
                    setColor(Color.argb(200, 60, 60, 60))
                    setStroke(2, Color.WHITE)
                }
                layoutParams = LinearLayout.LayoutParams(380, 100).apply {
                    setMargins(0, 10, 0, 10)
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
