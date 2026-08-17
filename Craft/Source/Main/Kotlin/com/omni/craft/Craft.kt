package com.omni.craft

import android.app.Activity as AndroidActivity
import android.app.Application
import android.app.Dialog
import android.content.Context
import android.content.pm.ActivityInfo
import android.graphics.*
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import android.graphics.drawable.GradientDrawable
import android.opengl.GLSurfaceView
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.*
import android.widget.*
import java.io.File
import java.io.PrintWriter
import java.io.StringWriter
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import org.json.JSONArray
import org.json.JSONObject

class CraftApp : Application() {
    override fun onCreate() {
        super.onCreate()
        CraftLogger.init(this)
    }
}

object CraftLogger {
    private const val TAG = "OmniCraft"
    private var logDir: File? = null

    fun init(context: Context) {
        val docsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS)
        logDir = File(docsDir, "Craft_Log").apply { if (!exists()) mkdirs() }
        Thread.setDefaultUncaughtExceptionHandler { _, throwable ->
            val sw = StringWriter()
            throwable.printStackTrace(PrintWriter(sw))
            Log.e(TAG, "Kritik Hata: $sw")
        }
    }

    fun getLogDirPath(): String = logDir?.absolutePath ?: ""
}

object BlockIconFactory {
    private val iconCache = HashMap<Int, Bitmap>()

    fun getIcon(context: Context, blockId: Int, size: Int = 96): Drawable {
        val cached = iconCache[blockId]
        if (cached != null) return BitmapDrawable(context.resources, cached)

        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG)

        val half = size / 2f
        val quarter = size / 4f
        val eighth = size / 8f

        val topCol: Int
        val sideLCol: Int
        val sideRCol: Int

        when (blockId) {
            1 -> { // Grass
                topCol = Color.rgb(85, 185, 50)
                sideLCol = Color.rgb(100, 70, 45)
                sideRCol = Color.rgb(125, 85, 55)
            }
            2 -> { // Dirt
                topCol = Color.rgb(125, 85, 55)
                sideLCol = Color.rgb(95, 65, 40)
                sideRCol = Color.rgb(135, 95, 60)
            }
            3 -> { // Stone
                topCol = Color.rgb(155, 155, 158)
                sideLCol = Color.rgb(120, 120, 122)
                sideRCol = Color.rgb(175, 175, 178)
            }
            4 -> { // Cobble
                topCol = Color.rgb(130, 130, 130)
                sideLCol = Color.rgb(100, 100, 100)
                sideRCol = Color.rgb(145, 145, 145)
            }
            5 -> { // Sand
                topCol = Color.rgb(235, 222, 160)
                sideLCol = Color.rgb(200, 188, 130)
                sideRCol = Color.rgb(248, 238, 178)
            }
            7, 10 -> { // Logs
                topCol = Color.rgb(168, 132, 85)
                sideLCol = Color.rgb(98, 72, 45)
                sideRCol = Color.rgb(122, 92, 60)
            }
            9, 11 -> { // Planks
                topCol = Color.rgb(182, 138, 82)
                sideLCol = Color.rgb(148, 108, 62)
                sideRCol = Color.rgb(202, 152, 92)
            }
            19 -> { // Diamond Ore
                topCol = Color.rgb(145, 145, 148)
                sideLCol = Color.rgb(45, 230, 230)
                sideRCol = Color.rgb(170, 170, 175)
            }
            32 -> { // Crafting Table
                topCol = Color.rgb(188, 142, 86)
                sideLCol = Color.rgb(152, 112, 66)
                sideRCol = Color.rgb(138, 98, 56)
            }
            36 -> { // Torch
                topCol = Color.rgb(255, 225, 55)
                sideLCol = Color.rgb(135, 92, 50)
                sideRCol = Color.rgb(165, 118, 65)
            }
            44 -> { // TNT
                topCol = Color.rgb(225, 50, 50)
                sideLCol = Color.rgb(180, 40, 40)
                sideRCol = Color.WHITE
            }
            else -> {
                topCol = Color.rgb(150, 150, 150)
                sideLCol = Color.rgb(115, 115, 115)
                sideRCol = Color.rgb(175, 175, 175)
            }
        }

        val topPath = Path().apply {
            moveTo(half, eighth)
            lineTo(size - eighth, quarter + eighth)
            lineTo(half, half + eighth)
            lineTo(eighth, quarter + eighth)
            close()
        }
        p.color = topCol
        c.drawPath(topPath, p)

        val leftPath = Path().apply {
            moveTo(eighth, quarter + eighth)
            lineTo(half, half + eighth)
            lineTo(half, size - eighth)
            lineTo(eighth, size - quarter)
            close()
        }
        p.color = sideLCol
        c.drawPath(leftPath, p)

        val rightPath = Path().apply {
            moveTo(half, half + eighth)
            lineTo(size - eighth, quarter + eighth)
            lineTo(size - eighth, size - quarter)
            lineTo(half, size - eighth)
            close()
        }
        p.color = sideRCol
        c.drawPath(rightPath, p)

        p.style = Paint.Style.STROKE
        p.strokeWidth = 2f
        p.color = Color.argb(120, 0, 0, 0)
        c.drawPath(topPath, p)
        c.drawPath(leftPath, p)
        c.drawPath(rightPath, p)

        iconCache[blockId] = bmp
        return BitmapDrawable(context.resources, bmp)
    }
}

object VectorIconFactory {
    fun createBreakIcon(context: Context, size: Int = 110): Drawable {
        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.STROKE
            strokeCap = Paint.Cap.ROUND
        }
        p.color = Color.rgb(185, 135, 80)
        p.strokeWidth = size * 0.10f
        c.drawLine(size * 0.25f, size * 0.75f, size * 0.70f, size * 0.30f, p)

        p.color = Color.WHITE
        p.strokeWidth = size * 0.12f
        val arcRect = RectF(size * 0.40f, size * 0.12f, size * 0.88f, size * 0.60f)
        c.drawArc(arcRect, 180f, 120f, false, p)
        return BitmapDrawable(context.resources, bmp)
    }

    fun createPlaceIcon(context: Context, size: Int = 110): Drawable {
        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.FILL
            color = Color.WHITE
        }
        val r = RectF(size * 0.25f, size * 0.25f, size * 0.75f, size * 0.75f)
        c.drawRoundRect(r, size * 0.08f, size * 0.08f, p)
        p.color = Color.rgb(35, 140, 85)
        p.style = Paint.Style.STROKE
        p.strokeWidth = size * 0.06f
        c.drawRoundRect(r, size * 0.08f, size * 0.08f, p)
        return BitmapDrawable(context.resources, bmp)
    }

    fun createJumpIcon(context: Context, size: Int = 110): Drawable {
        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.STROKE
            strokeWidth = size * 0.12f
            color = Color.WHITE
            strokeCap = Paint.Cap.ROUND
            strokeJoin = Paint.Join.ROUND
        }
        val path = Path().apply {
            moveTo(size * 0.25f, size * 0.55f)
            lineTo(size * 0.50f, size * 0.28f)
            lineTo(size * 0.75f, size * 0.55f)
        }
        c.drawPath(path, p)
        c.drawLine(size * 0.50f, size * 0.30f, size * 0.50f, size * 0.75f, p)
        return BitmapDrawable(context.resources, bmp)
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
    external fun nativeInput(x: Float, z: Float)
    external fun nativeCameraInput(dx: Float, dy: Float)
    external fun nativeSetBreaking(active: Boolean)
    external fun nativeTapPlaceAt(screenX: Float, screenY: Float)
    external fun nativeSetJumpState(holding: Boolean)
    external fun nativeSetSneakState(holding: Boolean)
    external fun nativeSetSprintState(active: Boolean)
    external fun nativeSelectSlot(slot: Int)
    external fun nativeSaveWorld()
    external fun nativeGetInventory(): String
    external fun nativeSetInventorySlot(slot: Int, blockId: Int, count: Int)
    external fun nativeGetPlayerStats(): String
    external fun nativeSetRenderDistance(distance: Int)
    external fun nativeSetFov(fov: Float)
    external fun nativeSetDayNight(enabled: Boolean)
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
            val dt = ((now - lastTimeNs) / 1_000_000_000.0).toFloat().coerceIn(0.001f, 0.033f)
            lastTimeNs = now
            nativeFrame(dt)
        }
    }
}

class Activity : AndroidActivity() {
    private lateinit var rootLayout: FrameLayout
    private var glView: GLSurfaceView? = null

    // Bağımsız Multi-Touch Takip Havuzu
    private var joyPointerId = -1
    private var joyOriginX = 0f
    private var joyOriginY = 0f
    private var joyKnobView: View? = null
    private var joyBaseView: View? = null

    private var camPointerId = -1
    private var lastCamX = 0f
    private var lastCamY = 0f
    private var touchDownX = 0f
    private var touchDownY = 0f
    private var touchDownTime = 0L

    private var sensitivity = 1.0f
    private var fov = 70.0f
    private var renderDistance = 6
    private var dayNightEnabled = true
    private var isSneaking = false
    private var isSprinting = false

    private val mainHandler = Handler(Looper.getMainLooper())
    private var statsUpdater: Runnable? = null
    private var statsTextView: TextView? = null

    private val prefs by lazy { getSharedPreferences("omni_settings", Context.MODE_PRIVATE) }

    private fun dp(v: Float): Int = (v * resources.displayMetrics.density + 0.5f).toInt()
    private fun uiScale(): Float {
        val w = rootLayout.width.coerceAtLeast(1)
        val h = rootLayout.height.coerceAtLeast(1)
        return (minOf(w, h) / 900f).coerceIn(0.75f, 1.35f)
    }
    private fun ui(v: Float): Int = dp(v * uiScale())

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.attributes.preferredDisplayModeId =
                display?.supportedModes?.maxByOrNull { it.refreshRate }?.modeId ?: 0
        }

        rootLayout = FrameLayout(this)
        setContentView(rootLayout)
        sensitivity = prefs.getFloat("sensitivity", 1.0f)
        fov = prefs.getFloat("fov", 70.0f)
        renderDistance = prefs.getInt("renderDistance", 6)
        dayNightEnabled = prefs.getBoolean("dayNight", true)
        showLobby()
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

    private fun showLobby() {
        statsUpdater?.let { mainHandler.removeCallbacks(it) }
        rootLayout.removeAllViews()
        rootLayout.post {
            val lobby = FrameLayout(this).apply {
                layoutParams = FrameLayout.LayoutParams(-1, -1)
                background = GradientDrawable(GradientDrawable.Orientation.TL_BR, intArrayOf(
                    Color.rgb(4, 10, 22), Color.rgb(10, 36, 56), Color.rgb(24, 12, 42)
                ))
            }
            val glow = View(this).apply {
                background = GradientDrawable().apply {
                    shape = GradientDrawable.OVAL
                    setColor(Color.argb(60, 45, 215, 255))
                }
                layoutParams = FrameLayout.LayoutParams(ui(560f), ui(560f)).apply {
                    gravity = Gravity.TOP or Gravity.END
                    topMargin = -ui(190f); rightMargin = -ui(160f)
                }
            }
            lobby.addView(glow)

            val content = LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                gravity = Gravity.CENTER
                setPadding(ui(36f), ui(28f), ui(36f), ui(28f))
                layoutParams = FrameLayout.LayoutParams(ui(660f), -2, Gravity.CENTER)
                background = GradientDrawable().apply {
                    setColor(Color.argb(215, 8, 16, 28))
                    cornerRadius = ui(32f).toFloat()
                    setStroke(ui(1.5f), Color.argb(130, 130, 230, 255))
                }
                elevation = ui(20f).toFloat()
            }

            fun text(value: String, size: Float, color: Int) = TextView(this).apply {
                text = value; textSize = size * uiScale(); setTextColor(color); gravity = Gravity.CENTER
            }

            content.addView(text("OMNI CRAFT", 48f, Color.WHITE).apply {
                typeface = Typeface.create(Typeface.DEFAULT, Typeface.BOLD)
                letterSpacing = 0.08f
                setShadowLayer(ui(20f).toFloat(), 0f, ui(6f).toFloat(), Color.argb(230, 0, 0, 0))
            })
            content.addView(text("AAA SURVIVAL • INFINITE WORLDS", 12f, Color.rgb(100, 230, 255)).apply {
                letterSpacing = 0.18f; setPadding(0, 0, 0, ui(8f))
            })
            content.addView(text("Keşfet  •  İnşa Et  •  Hayatta Kal  •  Geliş", 14f, Color.LTGRAY).apply {
                setPadding(0, 0, 0, ui(24f))
            })

            fun createLobbyBtn(label: String, primary: Boolean, onClick: () -> Unit) = Button(this).apply {
                text = label; textSize = (if (primary) 18f else 15f) * uiScale()
                setTextColor(Color.WHITE); isAllCaps = false; stateListAnimator = null
                background = GradientDrawable().apply {
                    setColor(if (primary) Color.rgb(24, 150, 188) else Color.argb(225, 28, 42, 56))
                    cornerRadius = ui(if (primary) 22f else 18f).toFloat()
                    setStroke(ui(if (primary) 2f else 1f), if (primary) Color.rgb(140, 248, 255) else Color.argb(130, 150, 190, 210))
                }
                elevation = ui(if (primary) 10f else 6f).toFloat()
                layoutParams = LinearLayout.LayoutParams(ui(520f), ui(if (primary) 92f else 72f)).apply {
                    setMargins(0, ui(7f), 0, ui(7f))
                }
                setOnClickListener { onClick() }
            }

            content.addView(createLobbyBtn("▶   DÜNYAYA GİR", true) { startGame() })
            content.addView(createLobbyBtn("⚙   AYARLAR", false) { showSettingsDialog() })
            content.addView(createLobbyBtn("✕   ÇIKIŞ", false) { finish() })
            content.addView(text("Rayleigh Sky  •  Living Oceans  •  Voxel AO  •  Smooth Kinematics", 10f, Color.argb(185, 205, 220, 230)).apply {
                setPadding(0, ui(20f), 0, 0)
            })
            lobby.addView(content)
            rootLayout.addView(lobby)
            applyFullScreen()
        }
    }

    private fun startGame() {
        rootLayout.removeAllViews()

        val saveDir = File(filesDir, "world_data").apply { if (!exists()) mkdirs() }.absolutePath
        glView = GLSurfaceView(this).apply {
            setEGLContextClientVersion(3)
            setPreserveEGLContextOnPause(true)
            setRenderer(Engine.GameRenderer(saveDir))
            renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
        }
        rootLayout.addView(glView)

        setupGameHUD(rootLayout)
        applyFullScreen()
    }

    private fun setupGameHUD(root: FrameLayout) {
        val hud = FrameLayout(this).apply {
            layoutParams = FrameLayout.LayoutParams(-1, -1)
        }

        // 1. Hassas Crosshair
        val chV = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(220, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(ui(3.5f), ui(30f)).apply { gravity = Gravity.CENTER }
        }
        val chH = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(220, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(ui(30f), ui(3.5f)).apply { gravity = Gravity.CENTER }
        }
        hud.addView(chV)
        hud.addView(chH)

        // 2. Üst Durum Çubuğu (FPS, XYZ, Can, Açlık, Pause)
        val topBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setPadding(ui(24f), ui(16f), ui(24f), 0)
            layoutParams = FrameLayout.LayoutParams(-1, -2, Gravity.TOP)
        }

        statsTextView = TextView(this).apply {
            text = "XYZ: 0.0, 64.0, 0.0  •  60 FPS"
            textSize = 12f * uiScale()
            setTextColor(Color.WHITE)
            setShadowLayer(4f, 1f, 1f, Color.BLACK)
            layoutParams = LinearLayout.LayoutParams(0, -2, 1.0f)
        }
        topBar.addView(statsTextView)

        val pauseBtn = Button(this).apply {
            text = "❚❚"
            textSize = 14f * uiScale()
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(160, 30, 38, 48))
                cornerRadius = ui(12f).toFloat()
                setStroke(ui(1f), Color.argb(180, 200, 220, 240))
            }
            layoutParams = LinearLayout.LayoutParams(ui(56f), ui(56f))
            setOnClickListener { showPauseMenu() }
        }
        topBar.addView(pauseBtn)
        hud.addView(topBar)

        // 3. Dinamik Yüzen Joystick Tabanı ve Tutamaç
        joyBaseView = View(this).apply {
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(85, 25, 35, 45))
                setStroke(ui(2f), Color.argb(160, 100, 200, 240))
            }
            layoutParams = FrameLayout.LayoutParams(ui(180f), ui(180f)).apply {
                gravity = Gravity.BOTTOM or Gravity.START
                setMargins(ui(48f), 0, 0, ui(48f))
            }
        }
        joyKnobView = View(this).apply {
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(210, 45, 185, 235))
                setStroke(ui(2f), Color.WHITE)
            }
            layoutParams = FrameLayout.LayoutParams(ui(72f), ui(72f)).apply {
                gravity = Gravity.BOTTOM or Gravity.START
                setMargins(ui(102f), 0, 0, ui(102f))
            }
        }
        hud.addView(joyBaseView)
        hud.addView(joyKnobView)

        // 4. Sağ Eylem Butonları (KIR, KOY, ZIPLA, EĞİL, KOŞ) - Bağımsız Çoklu Dokunma
        val rightActions = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL
            layoutParams = FrameLayout.LayoutParams(-2, -2, Gravity.BOTTOM or Gravity.END).apply {
                setMargins(0, 0, ui(36f), ui(36f))
            }
        }

        val row1 = LinearLayout(this).apply { orientation = LinearLayout.HORIZONTAL }
        val row2 = LinearLayout(this).apply { orientation = LinearLayout.HORIZONTAL }

        fun makeActionButton(icon: Drawable?, bgCol: Int, sizeDp: Float = 84f, onTouch: (Boolean) -> Unit) = ImageButton(this).apply {
            if (icon != null) setImageDrawable(icon)
            scaleType = ImageView.ScaleType.CENTER_INSIDE
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(bgCol)
                setStroke(ui(1.5f), Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(ui(sizeDp), ui(sizeDp)).apply { setMargins(ui(6f), ui(6f), ui(6f), ui(6f)) }
            setOnTouchListener { _, event ->
                when (event.actionMasked) {
                    MotionEvent.ACTION_DOWN -> { onTouch(true); true }
                    MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> { onTouch(false); true }
                    else -> false
                }
            }
        }

        val breakBtn = makeActionButton(VectorIconFactory.createBreakIcon(this), Color.argb(175, 195, 45, 45)) { active ->
            Engine.nativeSetBreaking(active)
        }
        val placeBtn = makeActionButton(VectorIconFactory.createPlaceIcon(this), Color.argb(175, 35, 160, 85)) { active ->
            if (active) Engine.nativeTapPlaceAt(rootLayout.width * 0.5f, rootLayout.height * 0.5f)
        }
        val jumpBtn = makeActionButton(VectorIconFactory.createJumpIcon(this), Color.argb(175, 35, 105, 185)) { active ->
            Engine.nativeSetJumpState(active)
        }

        val sprintToggleBtn = Button(this).apply {
            text = "KOŞ"
            textSize = 12f * uiScale()
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(160, 50, 60, 75))
                setStroke(ui(1f), Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(ui(64f), ui(64f)).apply { setMargins(ui(6f), ui(6f), ui(6f), ui(6f)) }
            setOnClickListener {
                isSprinting = !isSprinting
                Engine.nativeSetSprintState(isSprinting)
                (background as GradientDrawable).setColor(if (isSprinting) Color.rgb(220, 140, 20) else Color.argb(160, 50, 60, 75))
            }
        }

        val sneakToggleBtn = Button(this).apply {
            text = "EĞİL"
            textSize = 12f * uiScale()
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(160, 50, 60, 75))
                setStroke(ui(1f), Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(ui(64f), ui(64f)).apply { setMargins(ui(6f), ui(6f), ui(6f), ui(6f)) }
            setOnClickListener {
                isSneaking = !isSneaking
                Engine.nativeSetSneakState(isSneaking)
                (background as GradientDrawable).setColor(if (isSneaking) Color.rgb(180, 50, 180) else Color.argb(160, 50, 60, 75))
            }
        }

        row1.addView(sprintToggleBtn)
        row1.addView(breakBtn)
        row1.addView(placeBtn)

        row2.addView(sneakToggleBtn)
        row2.addView(jumpBtn)

        rightActions.addView(row1)
        rightActions.addView(row2)
        hud.addView(rightActions)

        // 5. 9'lu Glassmorphism Hotbar
        val hotbarContainer = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            layoutParams = FrameLayout.LayoutParams(-2, -2, Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL).apply {
                bottomMargin = ui(16f)
            }
        }

        val hotbarSlots = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            background = GradientDrawable().apply {
                setColor(Color.argb(185, 18, 26, 38))
                cornerRadius = ui(14f).toFloat()
                setStroke(ui(1.5f), Color.argb(150, 120, 200, 240))
            }
            setPadding(ui(4f), ui(4f), ui(4f), ui(4f))
        }

        val initialBlockIds = intArrayOf(9, 4, 19, 32, 36, 1, 2, 7, 44)

        for (i in 0 until 9) {
            val slotContainer = FrameLayout(this).apply {
                background = GradientDrawable().apply {
                    setColor(if (i == 0) Color.argb(190, 80, 140, 180) else Color.argb(70, 0, 0, 0))
                    cornerRadius = ui(10f).toFloat()
                    setStroke(if (i == 0) ui(2.5f) else ui(1f), if (i == 0) Color.WHITE else Color.GRAY)
                }
                layoutParams = LinearLayout.LayoutParams(ui(68f), ui(68f)).apply { setMargins(ui(2.5f), ui(2.5f), ui(2.5f), ui(2.5f)) }
            }

            val blockIcon = ImageView(this).apply {
                setImageDrawable(BlockIconFactory.getIcon(this@Activity, initialBlockIds[i]))
                scaleType = ImageView.ScaleType.FIT_CENTER
                layoutParams = FrameLayout.LayoutParams(ui(54f), ui(54f)).apply { gravity = Gravity.CENTER }
            }
            slotContainer.addView(blockIcon)

            val countText = TextView(this).apply {
                text = if (i == 3) "16" else "64"
                textSize = 10f * uiScale()
                setTextColor(Color.WHITE)
                setShadowLayer(3f, 1f, 1f, Color.BLACK)
                layoutParams = FrameLayout.LayoutParams(-2, -2, Gravity.BOTTOM or Gravity.END).apply {
                    setMargins(0, 0, ui(6f), ui(2f))
                }
            }
            slotContainer.addView(countText)

            slotContainer.setOnTouchListener { _, event ->
                if (event.actionMasked == MotionEvent.ACTION_DOWN) {
                    Engine.nativeSelectSlot(i)
                    for (j in 0 until hotbarSlots.childCount) {
                        (hotbarSlots.getChildAt(j).background as GradientDrawable).apply {
                            setColor(if (j == i) Color.argb(190, 80, 140, 180) else Color.argb(70, 0, 0, 0))
                            setStroke(if (j == i) ui(2.5f) else ui(1f), if (j == i) Color.WHITE else Color.GRAY)
                        }
                    }
                }
                true
            }

            hotbarSlots.addView(slotContainer)
        }
        hotbarContainer.addView(hotbarSlots)

        val invBtn = Button(this).apply {
            text = "•••"
            textSize = 16f * uiScale()
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(185, 28, 40, 56))
                cornerRadius = ui(14f).toFloat()
                setStroke(ui(1.5f), Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(ui(68f), ui(68f)).apply { marginStart = ui(10f) }
            setOnClickListener { showInventoryDialog() }
        }
        hotbarContainer.addView(invBtn)
        hud.addView(hotbarContainer)

        // 6. Eşzamanlı Çoklu Dokunmatik Katman (Multi-Touch Camera & Virtual Joystick)
        hud.setOnTouchListener { _, event ->
            val pIdx = event.actionIndex
            val pId = event.getPointerId(pIdx)
            val px = event.getX(pIdx)
            val py = event.getY(pIdx)

            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                    if (px < root.width * 0.40f && joyPointerId == -1) {
                        joyPointerId = pId
                        joyOriginX = joyBaseView?.let { it.x + it.width / 2f } ?: px
                        joyOriginY = joyBaseView?.let { it.y + it.height / 2f } ?: py
                        handleJoystick(px, py)
                    } else if (px >= root.width * 0.35f && camPointerId == -1) {
                        camPointerId = pId
                        lastCamX = px
                        lastCamY = py
                        touchDownX = px
                        touchDownY = py
                        touchDownTime = System.currentTimeMillis()
                    }
                }
                MotionEvent.ACTION_MOVE -> {
                    for (i in 0 until event.pointerCount) {
                        val currentId = event.getPointerId(i)
                        val x = event.getX(i)
                        val y = event.getY(i)

                        if (currentId == joyPointerId) {
                            handleJoystick(x, y)
                        } else if (currentId == camPointerId) {
                            val dx = (x - lastCamX) * 0.22f * sensitivity
                            val dy = (y - lastCamY) * 0.22f * sensitivity
                            Engine.nativeCameraInput(dx, dy)
                            lastCamX = x
                            lastCamY = y
                        }
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                    if (pId == joyPointerId) {
                        joyPointerId = -1
                        Engine.nativeInput(0f, 0f)
                        resetJoystickKnob()
                    }
                    if (pId == camPointerId) {
                        val duration = System.currentTimeMillis() - touchDownTime
                        val distMoved = Math.hypot((lastCamX - touchDownX).toDouble(), (lastCamY - touchDownY).toDouble())
                        if (duration < 240 && distMoved < 25.0) {
                            Engine.nativeTapPlaceAt(lastCamX, lastCamY)
                        }
                        camPointerId = -1
                    }
                }
                MotionEvent.ACTION_CANCEL -> {
                    joyPointerId = -1
                    camPointerId = -1
                    Engine.nativeInput(0f, 0f)
                    resetJoystickKnob()
                }
            }
            true
        }

        root.addView(hud)

        // İstatistik Döngüsü
        statsUpdater = object : Runnable {
            override fun run() {
                try {
                    if (Engine.nativeIsInitialized()) {
                        val json = JSONObject(Engine.nativeGetPlayerStats())
                        val fps = json.optDouble("fps", 60.0)
                        val x = json.optDouble("x", 0.0)
                        val y = json.optDouble("y", 64.0)
                        val z = json.optDouble("z", 0.0)
                        statsTextView?.text = String.format("XYZ: %.1f, %.1f, %.1f  •  %.0f FPS", x, y, z, fps)
                    }
                } catch (_: Exception) {}
                mainHandler.postDelayed(this, 300)
            }
        }
        mainHandler.post(statsUpdater!!)

        glView?.postDelayed({
            if (Engine.nativeIsInitialized()) {
                Engine.nativeSetFov(fov)
                Engine.nativeSetRenderDistance(renderDistance)
                Engine.nativeSetDayNight(dayNightEnabled)
            }
        }, 150)
    }

    private fun handleJoystick(touchX: Float, touchY: Float) {
        var dx = touchX - joyOriginX
        var dy = touchY - joyOriginY
        val maxRadius = ui(75f).toFloat()
        val dist = Math.hypot(dx.toDouble(), dy.toDouble()).toFloat()

        if (dist > maxRadius) {
            dx = (dx / dist) * maxRadius
            dy = (dy / dist) * maxRadius
        }

        joyKnobView?.let {
            it.translationX = dx
            it.translationY = dy
        }

        Engine.nativeInput(dx / maxRadius, -dy / maxRadius)
    }

    private fun resetJoystickKnob() {
        joyKnobView?.animate()?.translationX(0f)?.translationY(0f)?.setDuration(120)?.start()
    }

    private fun showInventoryDialog() {
        val dialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.argb(238, 14, 20, 30))
        }

        val title = TextView(this).apply {
            text = "ENVANTER VE ÜRETİM"
            textSize = 20f * uiScale()
            setTextColor(Color.WHITE)
            typeface = Typeface.DEFAULT_BOLD
            setPadding(0, ui(16f), 0, ui(16f))
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

                val slotContainer = FrameLayout(this).apply {
                    background = GradientDrawable().apply {
                        setColor(Color.argb(130, 40, 52, 70))
                        setStroke(ui(1.5f), Color.argb(150, 160, 200, 230))
                        cornerRadius = ui(8f).toFloat()
                    }
                    layoutParams = GridLayout.LayoutParams().apply {
                        width = ui(80f)
                        height = ui(80f)
                        setMargins(ui(3.5f), ui(3.5f), ui(3.5f), ui(3.5f))
                    }
                }

                if (count > 0 && id > 0) {
                    val icon = ImageView(this).apply {
                        setImageDrawable(BlockIconFactory.getIcon(this@Activity, id))
                        scaleType = ImageView.ScaleType.FIT_CENTER
                        layoutParams = FrameLayout.LayoutParams(ui(60f), ui(60f)).apply { gravity = Gravity.CENTER }
                    }
                    slotContainer.addView(icon)

                    val countText = TextView(this).apply {
                        text = "$count"
                        textSize = 11f * uiScale()
                        setTextColor(Color.WHITE)
                        setShadowLayer(3f, 1f, 1f, Color.BLACK)
                        layoutParams = FrameLayout.LayoutParams(-2, -2, Gravity.BOTTOM or Gravity.END).apply {
                            setMargins(0, 0, ui(6f), ui(2f))
                        }
                    }
                    slotContainer.addView(countText)
                }

                grid.addView(slotContainer)
            }
        } catch (_: Exception) {}

        layout.addView(grid)

        val closeBtn = Button(this).apply {
            text = "Kapat"
            textSize = 14f * uiScale()
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(210, 50, 70, 95))
                cornerRadius = ui(12f).toFloat()
                setStroke(ui(1.5f), Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(ui(220f), ui(72f)).apply { topMargin = ui(24f) }
            setOnClickListener { dialog.dismiss(); applyFullScreen() }
        }
        layout.addView(closeBtn)

        dialog.setContentView(layout)
        dialog.show()
    }

    private fun showSettingsDialog() {
        val dialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.rgb(16, 24, 36))
            setPadding(ui(40f), ui(28f), ui(40f), ui(28f))
        }

        fun label(t: String) = TextView(this).apply { text = t; textSize = 15f * uiScale(); setTextColor(Color.WHITE); setPadding(0, ui(8f), 0, ui(4f)) }
        layout.addView(label("AYARLAR").apply { textSize = 24f * uiScale(); typeface = Typeface.DEFAULT_BOLD })

        val sl = label("Kamera Hassasiyeti: ${(sensitivity * 100).toInt()}%")
        layout.addView(sl)
        layout.addView(SeekBar(this).apply {
            max = 200
            progress = (sensitivity * 100).toInt().coerceIn(20, 200)
            setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
                override fun onProgressChanged(b: SeekBar?, p: Int, u: Boolean) {
                    sensitivity = (p.coerceAtLeast(20) / 100f)
                    sl.text = "Kamera Hassasiyeti: ${(sensitivity * 100).toInt()}%"
                }
                override fun onStartTrackingTouch(b: SeekBar?) {}
                override fun onStopTrackingTouch(b: SeekBar?) {}
            })
        })

        val fl = label("Görüş Alanı (FOV): ${fov.toInt()}°")
        layout.addView(fl)
        layout.addView(SeekBar(this).apply {
            max = 40
            progress = (fov - 55).toInt()
            setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
                override fun onProgressChanged(b: SeekBar?, p: Int, u: Boolean) {
                    fov = (55 + p).toFloat()
                    fl.text = "Görüş Alanı (FOV): ${fov.toInt()}°"
                    if (Engine.nativeIsInitialized()) Engine.nativeSetFov(fov)
                }
                override fun onStartTrackingTouch(b: SeekBar?) {}
                override fun onStopTrackingTouch(b: SeekBar?) {}
            })
        })

        val rl = label("Dünya Görüş Mesafesi: $renderDistance chunk")
        layout.addView(rl)
        layout.addView(SeekBar(this).apply {
            max = 5
            progress = (renderDistance - 3).coerceIn(0, 5)
            setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
                override fun onProgressChanged(b: SeekBar?, p: Int, u: Boolean) {
                    renderDistance = (3 + p).coerceIn(3, 8)
                    rl.text = "Dünya Görüş Mesafesi: $renderDistance chunk"
                    if (Engine.nativeIsInitialized()) Engine.nativeSetRenderDistance(renderDistance)
                }
                override fun onStartTrackingTouch(b: SeekBar?) {}
                override fun onStopTrackingTouch(b: SeekBar?) {}
            })
        })

        layout.addView(Switch(this).apply {
            text = "Dinamik Gündüz / Gece Döngüsü"
            textSize = 15f * uiScale()
            setTextColor(Color.WHITE)
            isChecked = dayNightEnabled
            setOnCheckedChangeListener { _, v ->
                dayNightEnabled = v
                if (Engine.nativeIsInitialized()) Engine.nativeSetDayNight(v)
            }
        })

        layout.addView(Button(this).apply {
            text = "Kaydet ve Geri Dön"
            textSize = 15f * uiScale()
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.rgb(24, 140, 180))
                cornerRadius = ui(14f).toFloat()
            }
            layoutParams = LinearLayout.LayoutParams(ui(380f), ui(88f)).apply { topMargin = ui(24f) }
            setOnClickListener {
                prefs.edit().putFloat("sensitivity", sensitivity).putFloat("fov", fov).putInt("renderDistance", renderDistance).putBoolean("dayNight", dayNightEnabled).apply()
                if (Engine.nativeIsInitialized()) {
                    Engine.nativeSetFov(fov)
                    Engine.nativeSetRenderDistance(renderDistance)
                    Engine.nativeSetDayNight(dayNightEnabled)
                }
                dialog.dismiss()
                applyFullScreen()
            }
        })

        dialog.setContentView(layout)
        dialog.show()
        applyFullScreen()
    }

    private fun showPauseMenu() {
        val dialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.argb(235, 14, 20, 28))
        }

        val title = TextView(this).apply {
            text = "OYUN DURAKLATILDI"
            textSize = 22f * uiScale()
            setTextColor(Color.WHITE)
            typeface = Typeface.DEFAULT_BOLD
            setPadding(0, 0, 0, ui(28f))
        }
        layout.addView(title)

        fun createMenuBtn(txt: String, onClick: () -> Unit): Button {
            return Button(this).apply {
                text = txt
                textSize = 15f * uiScale()
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    cornerRadius = ui(14f).toFloat()
                    setColor(Color.argb(210, 40, 56, 75))
                    setStroke(ui(1.5f), Color.WHITE)
                }
                layoutParams = LinearLayout.LayoutParams(ui(340f), ui(80f)).apply { setMargins(0, ui(8f), 0, ui(8f)) }
                setOnClickListener { onClick() }
            }
        }

        layout.addView(createMenuBtn("Oyuna Dön") {
            dialog.dismiss()
            applyFullScreen()
        })
        layout.addView(createMenuBtn("Ayarlar") {
            showSettingsDialog()
        })
        layout.addView(createMenuBtn("Ana Menüye Çık") {
            Engine.nativeSaveWorld()
            dialog.dismiss()
            showLobby()
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
        glView?.onResume()
        applyFullScreen()
    }

    override fun onPause() {
        super.onPause()
        glView?.onPause()
        Engine.nativeSaveWorld()
    }

    override fun onDestroy() {
        super.onDestroy()
        statsUpdater?.let { mainHandler.removeCallbacks(it) }
        Engine.nativeDestroy()
    }
}
