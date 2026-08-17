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
import android.os.VibrationEffect
import android.os.Vibrator
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
        logDir = File(docsDir, "Omni_Craft").apply { if (!exists()) mkdirs() }
        if (logDir == null || !logDir!!.exists()) {
            logDir = File(context.getExternalFilesDir(null), "Omni_Craft").apply { if (!exists()) mkdirs() }
        }
        Thread.setDefaultUncaughtExceptionHandler { _, throwable ->
            val sw = StringWriter()
            throwable.printStackTrace(PrintWriter(sw))
            logError("Kritik Hata: $sw")
        }
    }

    fun logError(msg: String) {
        Log.e(TAG, msg)
        try {
            logDir?.let {
                val f = File(it, "engine.log")
                f.appendText("[HATA] $msg\n")
            }
        } catch (_: Exception) {}
    }

    fun getLogDirPath(): String = logDir?.absolutePath ?: ""
}

// 10. Özellik: Canlı Eşya ve Et Görselleri Fabrikası (Icon Factory)
object BlockIconFactory {
    private val iconCache = HashMap<Int, Bitmap>()

    fun getIcon(context: Context, blockId: Int, size: Int = 80): Drawable {
        val cached = iconCache[blockId]
        if (cached != null) return BitmapDrawable(context.resources, cached)

        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG)

        val half = size / 2f
        val quarter = size / 4f
        val eighth = size / 8f

        // 2D Eşya İkonları Çizimi
        if (blockId >= 256) {
            when (blockId) {
                258 -> { // Çiğ Domuz Eti (Raw Pork)
                    p.color = Color.rgb(225, 115, 115)
                    c.drawRoundRect(RectF(size * 0.2f, size * 0.25f, size * 0.8f, size * 0.75f), 12f, 12f, p)
                    p.color = Color.rgb(255, 210, 210)
                    c.drawCircle(size * 0.4f, size * 0.5f, size * 0.12f, p)
                }
                260 -> { // Çiğ Sığır Eti (Raw Beef)
                    p.color = Color.rgb(180, 45, 45)
                    c.drawRoundRect(RectF(size * 0.15f, size * 0.2f, size * 0.85f, size * 0.8f), 14f, 14f, p)
                    p.color = Color.WHITE
                    c.drawRect(RectF(size * 0.45f, size * 0.4f, size * 0.65f, size * 0.6f), p)
                }
                262 -> { // Çiğ Koyun Eti (Raw Mutton)
                    p.color = Color.rgb(205, 75, 75)
                    c.drawRoundRect(RectF(size * 0.2f, size * 0.25f, size * 0.8f, size * 0.75f), 10f, 10f, p)
                }
                263 -> { // Yün (Wool)
                    p.color = Color.rgb(240, 240, 240)
                    c.drawCircle(half, half, size * 0.35f, p)
                }
                268 -> { // Elma (Apple)
                    p.color = Color.rgb(225, 30, 30)
                    c.drawCircle(half, half + 4f, size * 0.32f, p)
                    p.color = Color.rgb(85, 165, 40)
                    c.drawRect(RectF(half - 2f, size * 0.15f, half + 4f, size * 0.3f), p)
                }
                else -> {
                    p.color = Color.rgb(180, 180, 180)
                    c.drawRoundRect(RectF(size * 0.2f, size * 0.3f, size * 0.8f, size * 0.7f), 8f, 8f, p)
                }
            }
            iconCache[blockId] = bmp
            return BitmapDrawable(context.resources, bmp)
        }

        // 3D Blok İzometrik Çizimi
        val topCol: Int
        val sideLCol: Int
        val sideRCol: Int

        when (blockId) {
            1 -> { topCol = Color.rgb(85, 185, 50); sideLCol = Color.rgb(100, 70, 45); sideRCol = Color.rgb(125, 85, 55) }
            2 -> { topCol = Color.rgb(125, 85, 55); sideLCol = Color.rgb(95, 65, 40); sideRCol = Color.rgb(135, 95, 60) }
            3 -> { topCol = Color.rgb(155, 155, 158); sideLCol = Color.rgb(120, 120, 122); sideRCol = Color.rgb(175, 175, 178) }
            4 -> { topCol = Color.rgb(130, 130, 130); sideLCol = Color.rgb(100, 100, 100); sideRCol = Color.rgb(145, 145, 145) }
            5 -> { topCol = Color.rgb(235, 222, 160); sideLCol = Color.rgb(200, 188, 130); sideRCol = Color.rgb(248, 238, 178) }
            7, 10, 13 -> { topCol = Color.rgb(168, 132, 85); sideLCol = Color.rgb(98, 72, 45); sideRCol = Color.rgb(122, 92, 60) }
            9, 11, 14 -> { topCol = Color.rgb(182, 138, 82); sideLCol = Color.rgb(148, 108, 62); sideRCol = Color.rgb(202, 152, 92) }
            19 -> { topCol = Color.rgb(145, 145, 148); sideLCol = Color.rgb(45, 230, 230); sideRCol = Color.rgb(170, 170, 175) }
            32 -> { topCol = Color.rgb(188, 142, 86); sideLCol = Color.rgb(152, 112, 66); sideRCol = Color.rgb(138, 98, 56) }
            36 -> { topCol = Color.rgb(255, 225, 55); sideLCol = Color.rgb(135, 92, 50); sideRCol = Color.rgb(165, 118, 65) }
            44 -> { topCol = Color.rgb(225, 50, 50); sideLCol = Color.rgb(180, 40, 40); sideRCol = Color.WHITE }
            else -> { topCol = Color.rgb(150, 150, 150); sideLCol = Color.rgb(115, 115, 115); sideRCol = Color.rgb(175, 175, 175) }
        }

        val topPath = Path().apply {
            moveTo(half, eighth); lineTo(size - eighth, quarter + eighth); lineTo(half, half + eighth); lineTo(eighth, quarter + eighth); close()
        }
        p.color = topCol; c.drawPath(topPath, p)

        val leftPath = Path().apply {
            moveTo(eighth, quarter + eighth); lineTo(half, half + eighth); lineTo(half, size - eighth); lineTo(eighth, size - quarter); close()
        }
        p.color = sideLCol; c.drawPath(leftPath, p)

        val rightPath = Path().apply {
            moveTo(half, half + eighth); lineTo(size - eighth, quarter + eighth); lineTo(size - eighth, size - quarter); lineTo(half, size - eighth); close()
        }
        p.color = sideRCol; c.drawPath(rightPath, p)

        p.style = Paint.Style.STROKE; p.strokeWidth = 1.5f; p.color = Color.argb(120, 0, 0, 0)
        c.drawPath(topPath, p); c.drawPath(leftPath, p); c.drawPath(rightPath, p)

        iconCache[blockId] = bmp
        return BitmapDrawable(context.resources, bmp)
    }
}

object MinecraftIconFactory {
    fun createHeart(context: Context, state: Int, size: Int = 30): Drawable {
        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG)

        p.color = Color.rgb(35, 10, 10); p.style = Paint.Style.FILL
        c.drawRoundRect(RectF(1f, 1f, size - 1f, size - 1f), 3f, 3f, p)

        if (state == 2) {
            p.color = Color.rgb(235, 30, 30); c.drawRoundRect(RectF(3f, 3f, size - 3f, size - 3f), 2f, 2f, p)
            p.color = Color.rgb(255, 140, 140); c.drawRect(RectF(4f, 4f, size * 0.45f, size * 0.45f), p)
        } else if (state == 1) {
            p.color = Color.rgb(235, 30, 30); c.drawRect(RectF(3f, 3f, size * 0.5f, size - 3f), p)
            p.color = Color.rgb(75, 45, 45); c.drawRect(RectF(size * 0.5f, 3f, size - 3f, size - 3f), p)
        } else {
            p.color = Color.rgb(75, 45, 45); c.drawRoundRect(RectF(3f, 3f, size - 3f, size - 3f), 2f, 2f, p)
        }
        return BitmapDrawable(context.resources, bmp)
    }

    fun createHunger(context: Context, state: Int, size: Int = 30): Drawable {
        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG)

        p.color = Color.rgb(40, 20, 10); p.style = Paint.Style.FILL
        c.drawRoundRect(RectF(1f, 1f, size - 1f, size - 1f), 3f, 3f, p)

        if (state == 2) {
            p.color = Color.rgb(195, 105, 35); c.drawRoundRect(RectF(3f, 3f, size - 3f, size - 3f), 2f, 2f, p)
            p.color = Color.rgb(245, 185, 95); c.drawRect(RectF(4f, 4f, size * 0.45f, size * 0.45f), p)
        } else if (state == 1) {
            p.color = Color.rgb(195, 105, 35); c.drawRect(RectF(size * 0.5f, 3f, size - 3f, size - 3f), p)
            p.color = Color.rgb(65, 45, 30); c.drawRect(RectF(3f, 3f, size * 0.5f, size - 3f), p)
        } else {
            p.color = Color.rgb(65, 45, 30); c.drawRoundRect(RectF(3f, 3f, size - 3f, size - 3f), 2f, 2f, p)
        }
        return BitmapDrawable(context.resources, bmp)
    }

    // 6. Özellik: 10 Baloncuklu Oksijen Göstergesi (Bubble Bar)
    fun createBubble(context: Context, full: Boolean, size: Int = 30): Drawable {
        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG)

        if (full) {
            p.color = Color.rgb(35, 140, 235); p.style = Paint.Style.FILL
            c.drawCircle(size / 2f, size / 2f, size * 0.40f, p)
            p.color = Color.WHITE
            c.drawCircle(size * 0.38f, size * 0.38f, size * 0.12f, p)
        } else {
            p.color = Color.argb(100, 30, 80, 140); p.style = Paint.Style.STROKE; p.strokeWidth = 2f
            c.drawCircle(size / 2f, size / 2f, size * 0.35f, p)
        }
        return BitmapDrawable(context.resources, bmp)
    }
}

object VectorIconFactory {
    fun createBreakIcon(context: Context, size: Int = 90): Drawable {
        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.STROKE; strokeCap = Paint.Cap.ROUND; color = Color.rgb(215, 160, 95); strokeWidth = size * 0.10f
        }
        c.drawLine(size * 0.25f, size * 0.75f, size * 0.70f, size * 0.30f, p)
        p.color = Color.WHITE; p.strokeWidth = size * 0.12f
        c.drawArc(RectF(size * 0.40f, size * 0.12f, size * 0.88f, size * 0.60f), 180f, 120f, false, p)
        return BitmapDrawable(context.resources, bmp)
    }

    fun createJumpIcon(context: Context, size: Int = 90): Drawable {
        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.STROKE; strokeWidth = size * 0.12f; color = Color.WHITE; strokeCap = Paint.Cap.ROUND; strokeJoin = Paint.Join.ROUND
        }
        val path = Path().apply {
            moveTo(size * 0.25f, size * 0.55f); lineTo(size * 0.50f, size * 0.28f); lineTo(size * 0.75f, size * 0.55f)
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
    external fun nativeDropHeldItem()
    external fun nativeSetJumpState(holding: Boolean)
    external fun nativeSelectSlot(slot: Int)
    external fun nativeSaveWorld()
    external fun nativeGetInventory(): String
    external fun nativeGetHotbar(): String
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
            nativeInit(width, height, saveDir)
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

    private var joyPointerId = -1
    private var joyOriginX = 0f
    private var joyOriginY = 0f
    private var joyKnobView: View? = null
    private var joyBaseView: View? = null

    private var breakPointerId = -1
    private var jumpPointerId = -1
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

    private val mainHandler = Handler(Looper.getMainLooper())
    private var statsUpdater: Runnable? = null
    private var statsTextView: TextView? = null
    private var healthBar: LinearLayout? = null
    private var hungerBar: LinearLayout? = null
    private var oxygenBar: LinearLayout? = null
    private var hotbarSlotsLayout: LinearLayout? = null

    private val vibrator by lazy { getSystemService(Context.VIBRATOR_SERVICE) as? Vibrator }
    private val prefs by lazy { getSharedPreferences("omni_settings", Context.MODE_PRIVATE) }

    private fun dp(v: Float): Int = (v * resources.displayMetrics.density + 0.5f).toInt()

    // 9. Özellik: Titreşim Geri Bildirimi (Haptic Feedback)
    private fun triggerHaptic(durationMs: Long = 30) {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                vibrator?.vibrate(VibrationEffect.createOneShot(durationMs, VibrationEffect.DEFAULT_AMPLITUDE))
            } else {
                @Suppress("DEPRECATION")
                vibrator?.vibrate(durationMs)
            }
        } catch (_: Exception) {}
    }

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

            val content = LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                gravity = Gravity.CENTER
                setPadding(dp(32f), dp(24f), dp(32f), dp(24f))
                layoutParams = FrameLayout.LayoutParams(dp(440f), -2, Gravity.CENTER)
                background = GradientDrawable().apply {
                    setColor(Color.argb(215, 8, 16, 28))
                    cornerRadius = dp(24f).toFloat()
                    setStroke(dp(1.5f), Color.argb(130, 130, 230, 255))
                }
                elevation = dp(16f).toFloat()
            }

            fun text(value: String, size: Float, color: Int) = TextView(this).apply {
                text = value; textSize = size; setTextColor(color); gravity = Gravity.CENTER
            }

            content.addView(text("OMNI CRAFT", 34f, Color.WHITE).apply {
                typeface = Typeface.create(Typeface.DEFAULT, Typeface.BOLD)
                letterSpacing = 0.06f
                setShadowLayer(dp(12f).toFloat(), 0f, dp(4f).toFloat(), Color.argb(230, 0, 0, 0))
            })
            content.addView(text("SURVIVAL • INFINITE WORLDS", 11f, Color.rgb(100, 230, 255)).apply {
                letterSpacing = 0.14f; setPadding(0, 0, 0, dp(6f))
            })
            content.addView(text("Keşfet  •  İnşa Et  •  Hayatta Kal", 13f, Color.LTGRAY).apply {
                setPadding(0, 0, 0, dp(18f))
            })

            fun createLobbyBtn(label: String, primary: Boolean, onClick: () -> Unit) = Button(this).apply {
                text = label; textSize = if (primary) 16f else 14f
                setTextColor(Color.WHITE); isAllCaps = false; stateListAnimator = null
                background = GradientDrawable().apply {
                    setColor(if (primary) Color.rgb(24, 150, 188) else Color.argb(225, 28, 42, 56))
                    cornerRadius = dp(16f).toFloat()
                    setStroke(dp(1.5f), if (primary) Color.rgb(140, 248, 255) else Color.argb(130, 150, 190, 210))
                }
                layoutParams = LinearLayout.LayoutParams(dp(360f), dp(if (primary) 54f else 46f)).apply {
                    setMargins(0, dp(5f), 0, dp(5f))
                }
                setOnClickListener { onClick() }
            }

            content.addView(createLobbyBtn("▶   DÜNYAYA GİR", true) { startGame() })
            content.addView(createLobbyBtn("⚙   AYARLAR", false) { showSettingsDialog() })
            content.addView(createLobbyBtn("✕   ÇIKIŞ", false) { finish() })
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

        // 1. Crosshair
        val chV = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(200, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(dp(2.5f), dp(20f)).apply { gravity = Gravity.CENTER }
        }
        val chH = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(200, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(dp(20f), dp(2.5f)).apply { gravity = Gravity.CENTER }
        }
        hud.addView(chV)
        hud.addView(chH)

        // 2. Üst Durum Çubuğu (XYZ/FPS, 10 Kalp, 10 Açlık, 10 Oksijen, Pause Butonu)
        val topBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(16f), dp(10f), dp(16f), 0)
            layoutParams = FrameLayout.LayoutParams(-1, -2, Gravity.TOP)
        }

        statsTextView = TextView(this).apply {
            text = "XYZ: 0.0, 64.0, 0.0 • 60 FPS"
            textSize = 11f
            setTextColor(Color.WHITE)
            setShadowLayer(3f, 1f, 1f, Color.BLACK)
            layoutParams = LinearLayout.LayoutParams(0, -2, 1.0f)
        }
        topBar.addView(statsTextView)

        val statusBars = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL
            layoutParams = LinearLayout.LayoutParams(-2, -2).apply { marginEnd = dp(16f) }
        }

        healthBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            setPadding(0, 0, 0, dp(2f))
        }
        hungerBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            setPadding(0, 0, 0, dp(2f))
        }
        oxygenBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
        }

        for (i in 0 until 10) {
            val heart = ImageView(this).apply {
                setImageDrawable(MinecraftIconFactory.createHeart(this@Activity, 2, dp(12f)))
                layoutParams = LinearLayout.LayoutParams(dp(12f), dp(12f)).apply { setMargins(dp(0.5f), 0, dp(0.5f), 0) }
            }
            healthBar?.addView(heart)

            val hunger = ImageView(this).apply {
                setImageDrawable(MinecraftIconFactory.createHunger(this@Activity, 2, dp(12f)))
                layoutParams = LinearLayout.LayoutParams(dp(12f), dp(12f)).apply { setMargins(dp(0.5f), 0, dp(0.5f), 0) }
            }
            hungerBar?.addView(hunger)

            val bubble = ImageView(this).apply {
                setImageDrawable(MinecraftIconFactory.createBubble(this@Activity, true, dp(11f)))
                layoutParams = LinearLayout.LayoutParams(dp(11f), dp(11f)).apply { setMargins(dp(0.5f), 0, dp(0.5f), 0) }
            }
            oxygenBar?.addView(bubble)
        }

        statusBars.addView(healthBar)
        statusBars.addView(hungerBar)
        statusBars.addView(oxygenBar)
        topBar.addView(statusBars)

        val pauseBtn = Button(this).apply {
            text = "❚❚"
            textSize = 12f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(160, 30, 38, 48))
                cornerRadius = dp(8f).toFloat()
                setStroke(dp(1f), Color.argb(180, 200, 220, 240))
            }
            layoutParams = LinearLayout.LayoutParams(dp(38f), dp(38f))
            setOnClickListener { showPauseMenu() }
        }
        topBar.addView(pauseBtn)
        hud.addView(topBar)

        // 3. Joystick Görseli
        joyBaseView = View(this).apply {
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(70, 25, 35, 45))
                setStroke(dp(1.5f), Color.argb(140, 100, 200, 240))
            }
            layoutParams = FrameLayout.LayoutParams(dp(110f), dp(110f)).apply {
                gravity = Gravity.BOTTOM or Gravity.START
                setMargins(dp(24f), 0, 0, dp(24f))
            }
        }
        joyKnobView = View(this).apply {
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(200, 45, 185, 235))
                setStroke(dp(1.5f), Color.WHITE)
            }
            layoutParams = FrameLayout.LayoutParams(dp(44f), dp(44f)).apply {
                gravity = Gravity.BOTTOM or Gravity.START
                setMargins(dp(57f), 0, 0, dp(57f))
            }
        }
        hud.addView(joyBaseView)
        hud.addView(joyKnobView)

        // 4. Sağ Aksiyon Görselleri (KIR, ZIPLA, AT)
        val breakVisual = ImageView(this).apply {
            setImageDrawable(VectorIconFactory.createBreakIcon(this@Activity, dp(54f)))
            scaleType = ImageView.ScaleType.CENTER_INSIDE
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(175, 195, 45, 45))
                setStroke(dp(1.5f), Color.WHITE)
            }
            layoutParams = FrameLayout.LayoutParams(dp(54f), dp(54f)).apply {
                gravity = Gravity.BOTTOM or Gravity.END
                setMargins(0, 0, dp(24f), dp(88f))
            }
        }
        val jumpVisual = ImageView(this).apply {
            setImageDrawable(VectorIconFactory.createJumpIcon(this@Activity, dp(60f)))
            scaleType = ImageView.ScaleType.CENTER_INSIDE
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(175, 35, 105, 185))
                setStroke(dp(1.5f), Color.WHITE)
            }
            layoutParams = FrameLayout.LayoutParams(dp(60f), dp(60f)).apply {
                gravity = Gravity.BOTTOM or Gravity.END
                setMargins(0, 0, dp(24f), dp(20f))
            }
        }
        val dropBtnVisual = Button(this).apply {
            text = "AT"
            textSize = 10f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(160, 60, 70, 85))
                setStroke(dp(1f), Color.WHITE)
            }
            layoutParams = FrameLayout.LayoutParams(dp(42f), dp(42f)).apply {
                gravity = Gravity.BOTTOM or Gravity.END
                setMargins(0, 0, dp(86f), dp(94f))
            }
        }
        hud.addView(breakVisual)
        hud.addView(jumpVisual)
        hud.addView(dropBtnVisual)

        // 5. 9'lu Hotbar
        val hotbarContainer = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            layoutParams = FrameLayout.LayoutParams(-2, -2, Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL).apply {
                bottomMargin = dp(8f)
            }
        }

        hotbarSlotsLayout = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            background = GradientDrawable().apply {
                setColor(Color.argb(185, 18, 26, 38))
                cornerRadius = dp(8f).toFloat()
                setStroke(dp(1f), Color.argb(140, 120, 200, 240))
            }
            setPadding(dp(2f), dp(2f), dp(2f), dp(2f))
        }

        for (i in 0 until 9) {
            val slotContainer = FrameLayout(this).apply {
                background = GradientDrawable().apply {
                    setColor(if (i == 0) Color.argb(190, 80, 140, 180) else Color.argb(70, 0, 0, 0))
                    cornerRadius = dp(6f).toFloat()
                    setStroke(if (i == 0) dp(2f) else dp(1f), if (i == 0) Color.WHITE else Color.GRAY)
                }
                layoutParams = LinearLayout.LayoutParams(dp(38f), dp(38f)).apply { setMargins(dp(1.5f), dp(1.5f), dp(1.5f), dp(1.5f)) }
            }

            val blockIcon = ImageView(this).apply {
                scaleType = ImageView.ScaleType.FIT_CENTER
                layoutParams = FrameLayout.LayoutParams(dp(28f), dp(28f)).apply { gravity = Gravity.CENTER }
            }
            slotContainer.addView(blockIcon)

            val countText = TextView(this).apply {
                text = ""
                textSize = 8.5f
                setTextColor(Color.WHITE)
                setShadowLayer(2f, 1f, 1f, Color.BLACK)
                layoutParams = FrameLayout.LayoutParams(-2, -2, Gravity.BOTTOM or Gravity.END).apply {
                    setMargins(0, 0, dp(3f), dp(1f))
                }
            }
            slotContainer.addView(countText)

            hotbarSlotsLayout?.addView(slotContainer)
        }
        hotbarContainer.addView(hotbarSlotsLayout)

        val invBtn = Button(this).apply {
            text = "•••"
            textSize = 11f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(185, 28, 40, 56))
                cornerRadius = dp(8f).toFloat()
                setStroke(dp(1f), Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(dp(38f), dp(38f)).apply { marginStart = dp(6f) }
            setOnClickListener { showInventoryDialog() }
        }
        hotbarContainer.addView(invBtn)
        hud.addView(hotbarContainer)

        // 6. MASTER BİRLEŞİK ÇOKLU DOKUNMATİK KATMANI
        hud.setOnTouchListener { _, event ->
            val w = rootLayout.width.toFloat()
            val h = rootLayout.height.toFloat()

            val breakRect = RectF(w - dp(90f), h - dp(150f), w, h - dp(75f))
            val jumpRect = RectF(w - dp(90f), h - dp(75f), w, h)
            val dropRect = RectF(w - dp(135f), h - dp(145f), w - dp(80f), h - dp(85f))
            val hotbarRect = RectF(w * 0.30f, h - dp(56f), w * 0.70f, h)

            val pIdx = event.actionIndex
            val pId = event.getPointerId(pIdx)
            val px = event.getX(pIdx)
            val py = event.getY(pIdx)

            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                    if (dropRect.contains(px, py)) {
                        Engine.nativeDropHeldItem()
                        triggerHaptic(40)
                    } else if (breakRect.contains(px, py) && breakPointerId == -1) {
                        breakPointerId = pId
                        Engine.nativeSetBreaking(true)
                        triggerHaptic(25)
                    } else if (jumpRect.contains(px, py) && jumpPointerId == -1) {
                        jumpPointerId = pId
                        Engine.nativeSetJumpState(true)
                    } else if (hotbarRect.contains(px, py)) {
                        val slotW = hotbarRect.width() / 9f
                        val sel = ((px - hotbarRect.left) / slotW).toInt().coerceIn(0, 8)
                        Engine.nativeSelectSlot(sel)
                        updateHotbarVisuals(sel)
                        triggerHaptic(20)
                    } else if (px < w * 0.40f && joyPointerId == -1) {
                        joyPointerId = pId
                        joyOriginX = joyBaseView?.let { it.x + it.width / 2f } ?: px
                        joyOriginY = joyBaseView?.let { it.y + it.height / 2f } ?: py
                        handleJoystick(px, py)
                    } else if (px >= w * 0.35f && camPointerId == -1) {
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
                        val curId = event.getPointerId(i)
                        val x = event.getX(i)
                        val y = event.getY(i)

                        if (curId == joyPointerId) {
                            handleJoystick(x, y)
                        } else if (curId == camPointerId) {
                            val dx = (x - lastCamX) * 0.22f * sensitivity
                            val dy = (y - lastCamY) * 0.22f * sensitivity
                            Engine.nativeCameraInput(dx, dy)
                            lastCamX = x
                            lastCamY = y
                        }
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                    if (pId == breakPointerId) {
                        breakPointerId = -1
                        Engine.nativeSetBreaking(false)
                    }
                    if (pId == jumpPointerId) {
                        jumpPointerId = -1
                        Engine.nativeSetJumpState(false)
                    }
                    if (pId == joyPointerId) {
                        joyPointerId = -1
                        Engine.nativeInput(0f, 0f)
                        resetJoystickKnob()
                    }
                    if (pId == camPointerId) {
                        val duration = System.currentTimeMillis() - touchDownTime
                        val distMoved = Math.hypot((lastCamX - touchDownX).toDouble(), (lastCamY - touchDownY).toDouble())
                        if (duration < 240 && distMoved < 20.0 && !breakRect.contains(lastCamX, lastCamY) && !jumpRect.contains(lastCamX, lastCamY) && !dropRect.contains(lastCamX, lastCamY)) {
                            Engine.nativeTapPlaceAt(lastCamX, lastCamY)
                            triggerHaptic(30)
                        }
                        camPointerId = -1
                    }
                }
                MotionEvent.ACTION_CANCEL -> {
                    if (pId == breakPointerId) { breakPointerId = -1; Engine.nativeSetBreaking(false) }
                    if (pId == jumpPointerId) { jumpPointerId = -1; Engine.nativeSetJumpState(false) }
                    if (pId == joyPointerId) { joyPointerId = -1; Engine.nativeInput(0f, 0f); resetJoystickKnob() }
                    if (pId == camPointerId) { camPointerId = -1 }
                }
            }
            true
        }

        root.addView(hud)

        statsUpdater = object : Runnable {
            override fun run() {
                try {
                    if (Engine.nativeIsInitialized()) {
                        val json = JSONObject(Engine.nativeGetPlayerStats())
                        val fps = json.optDouble("fps", 60.0)
                        val x = json.optDouble("x", 0.0)
                        val y = json.optDouble("y", 64.0)
                        val z = json.optDouble("z", 0.0)
                        val hp = json.optDouble("hp", 20.0).toFloat()
                        val hunger = json.optDouble("hunger", 20.0).toFloat()
                        val oxygen = json.optDouble("oxygen", 20.0).toFloat()

                        statsTextView?.text = String.format("XYZ: %.1f, %.1f, %.1f  •  %.0f FPS", x, y, z, fps)

                        healthBar?.let { hb ->
                            for (i in 0 until 10) {
                                val heartHp = (hp - i * 2.0f).coerceIn(0.0f, 2.0f)
                                val state = if (heartHp >= 1.5f) 2 else if (heartHp >= 0.5f) 1 else 0
                                (hb.getChildAt(i) as ImageView).setImageDrawable(MinecraftIconFactory.createHeart(this@Activity, state, dp(12f)))
                            }
                        }

                        hungerBar?.let { ub ->
                            for (i in 0 until 10) {
                                val hungerHp = (hunger - i * 2.0f).coerceIn(0.0f, 2.0f)
                                val state = if (hungerHp >= 1.5f) 2 else if (hungerHp >= 0.5f) 1 else 0
                                (ub.getChildAt(i) as ImageView).setImageDrawable(MinecraftIconFactory.createHunger(this@Activity, state, dp(12f)))
                            }
                        }

                        // Oksijen Göstergesi
                        oxygenBar?.let { ob ->
                            if (oxygen < 20.0f) {
                                ob.visibility = View.VISIBLE
                                for (i in 0 until 10) {
                                    val bubbleFull = (oxygen > i * 2.0f)
                                    (ob.getChildAt(i) as ImageView).setImageDrawable(MinecraftIconFactory.createBubble(this@Activity, bubbleFull, dp(11f)))
                                }
                            } else {
                                ob.visibility = View.GONE
                            }
                        }

                        refreshHotbarFromNative()
                    }
                } catch (_: Exception) {}
                mainHandler.postDelayed(this, 120)
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

    private fun updateHotbarVisuals(selectedSlot: Int) {
        hotbarSlotsLayout?.let { layout ->
            for (j in 0 until layout.childCount) {
                (layout.getChildAt(j).background as GradientDrawable).apply {
                    setColor(if (j == selectedSlot) Color.argb(190, 80, 140, 180) else Color.argb(70, 0, 0, 0))
                    setStroke(if (j == selectedSlot) dp(2f) else dp(1f), if (j == selectedSlot) Color.WHITE else Color.GRAY)
                }
            }
        }
    }

    private fun refreshHotbarFromNative() {
        try {
            val hbData = JSONArray(Engine.nativeGetHotbar())
            hotbarSlotsLayout?.let { layout ->
                for (i in 0 until hbData.length()) {
                    val obj = hbData.getJSONObject(i)
                    val id = obj.getInt("id")
                    val count = obj.getInt("count")

                    val slot = layout.getChildAt(i) as FrameLayout
                    val icon = slot.getChildAt(0) as ImageView
                    val countTxt = slot.getChildAt(1) as TextView

                    if (count > 0 && id > 0) {
                        icon.setImageDrawable(BlockIconFactory.getIcon(this@Activity, id, dp(28f)))
                        icon.visibility = View.VISIBLE
                        countTxt.text = if (count > 1) "$count" else ""
                    } else {
                        icon.setImageDrawable(null)
                        icon.visibility = View.INVISIBLE
                        countTxt.text = ""
                    }
                }
            }
        } catch (_: Exception) {}
    }

    private fun handleJoystick(touchX: Float, touchY: Float) {
        var dx = touchX - joyOriginX
        var dy = touchY - joyOriginY
        val maxRadius = dp(45f).toFloat()
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
            text = "ENVANTER"
            textSize = 18f
            setTextColor(Color.WHITE)
            typeface = Typeface.DEFAULT_BOLD
            setPadding(0, dp(14f), 0, dp(14f))
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
                        setStroke(dp(1.5f), Color.argb(150, 160, 200, 230))
                        cornerRadius = dp(6f).toFloat()
                    }
                    layoutParams = GridLayout.LayoutParams().apply {
                        width = dp(46f)
                        height = dp(46f)
                        setMargins(dp(2.5f), dp(2.5f), dp(2.5f), dp(2.5f))
                    }
                }

                if (count > 0 && id > 0) {
                    val icon = ImageView(this).apply {
                        setImageDrawable(BlockIconFactory.getIcon(this@Activity, id, dp(36f)))
                        scaleType = ImageView.ScaleType.FIT_CENTER
                        layoutParams = FrameLayout.LayoutParams(dp(34f), dp(34f)).apply { gravity = Gravity.CENTER }
                    }
                    slotContainer.addView(icon)

                    val countText = TextView(this).apply {
                        text = "$count"
                        textSize = 9f
                        setTextColor(Color.WHITE)
                        setShadowLayer(2f, 1f, 1f, Color.BLACK)
                        layoutParams = FrameLayout.LayoutParams(-2, -2, Gravity.BOTTOM or Gravity.END).apply {
                            setMargins(0, 0, dp(4f), dp(1f))
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
            textSize = 13f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(210, 50, 70, 95))
                cornerRadius = dp(10f).toFloat()
                setStroke(dp(1f), Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(dp(160f), dp(48f)).apply { topMargin = dp(18f) }
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
            setPadding(dp(32f), dp(20f), dp(32f), dp(20f))
        }

        fun label(t: String) = TextView(this).apply { text = t; textSize = 14f; setTextColor(Color.WHITE); setPadding(0, dp(6f), 0, dp(3f)) }
        layout.addView(label("AYARLAR").apply { textSize = 20f; typeface = Typeface.DEFAULT_BOLD })

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
            textSize = 14f
            setTextColor(Color.WHITE)
            isChecked = dayNightEnabled
            setOnCheckedChangeListener { _, v ->
                dayNightEnabled = v
                if (Engine.nativeIsInitialized()) Engine.nativeSetDayNight(v)
            }
        })

        layout.addView(Button(this).apply {
            text = "Kaydet ve Geri Dön"
            textSize = 14f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.rgb(24, 140, 180))
                cornerRadius = dp(12f).toFloat()
            }
            layoutParams = LinearLayout.LayoutParams(dp(260f), dp(50f)).apply { topMargin = dp(18f) }
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
            textSize = 19f
            setTextColor(Color.WHITE)
            typeface = Typeface.DEFAULT_BOLD
            setPadding(0, 0, 0, dp(20f))
        }
        layout.addView(title)

        fun createMenuBtn(txt: String, onClick: () -> Unit): Button {
            return Button(this).apply {
                text = txt
                textSize = 14f
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    cornerRadius = dp(12f).toFloat()
                    setColor(Color.argb(210, 40, 56, 75))
                    setStroke(dp(1.5f), Color.WHITE)
                }
                layoutParams = LinearLayout.LayoutParams(dp(240f), dp(48f)).apply { setMargins(0, dp(6f), 0, dp(6f)) }
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
