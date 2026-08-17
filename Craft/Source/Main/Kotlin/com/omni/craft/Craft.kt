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
import android.util.Log
import android.view.*
import android.widget.*
import java.io.File
import java.io.StringWriter
import java.io.PrintWriter
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
                topCol = Color.rgb(85, 175, 50)
                sideLCol = Color.rgb(100, 70, 45)
                sideRCol = Color.rgb(125, 85, 55)
            }
            2 -> { // Dirt
                topCol = Color.rgb(125, 85, 55)
                sideLCol = Color.rgb(95, 65, 40)
                sideRCol = Color.rgb(135, 95, 60)
            }
            3 -> { // Stone
                topCol = Color.rgb(150, 150, 150)
                sideLCol = Color.rgb(115, 115, 115)
                sideRCol = Color.rgb(170, 170, 170)
            }
            4 -> { // Cobble
                topCol = Color.rgb(125, 125, 125)
                sideLCol = Color.rgb(95, 95, 95)
                sideRCol = Color.rgb(140, 140, 140)
            }
            5 -> { // Sand
                topCol = Color.rgb(230, 220, 160)
                sideLCol = Color.rgb(195, 185, 130)
                sideRCol = Color.rgb(245, 235, 175)
            }
            7, 10 -> { // Logs
                topCol = Color.rgb(165, 130, 85)
                sideLCol = Color.rgb(95, 70, 45)
                sideRCol = Color.rgb(120, 90, 60)
            }
            9, 11 -> { // Planks
                topCol = Color.rgb(180, 135, 80)
                sideLCol = Color.rgb(145, 105, 60)
                sideRCol = Color.rgb(200, 150, 90)
            }
            19 -> { // Diamond Ore
                topCol = Color.rgb(145, 145, 145)
                sideLCol = Color.rgb(45, 215, 215)
                sideRCol = Color.rgb(170, 170, 170)
            }
            32 -> { // Crafting Table
                topCol = Color.rgb(185, 140, 85)
                sideLCol = Color.rgb(150, 110, 65)
                sideRCol = Color.rgb(135, 95, 55)
            }
            36 -> { // Torch
                topCol = Color.rgb(255, 220, 50)
                sideLCol = Color.rgb(130, 90, 50)
                sideRCol = Color.rgb(160, 115, 65)
            }
            44 -> { // TNT
                topCol = Color.rgb(215, 55, 55)
                sideLCol = Color.rgb(175, 45, 45)
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
        p.color = Color.rgb(175, 125, 75)
        p.strokeWidth = size * 0.10f
        c.drawLine(size * 0.25f, size * 0.75f, size * 0.70f, size * 0.30f, p)

        p.color = Color.WHITE
        p.strokeWidth = size * 0.12f
        val arcRect = RectF(size * 0.40f, size * 0.12f, size * 0.88f, size * 0.60f)
        c.drawArc(arcRect, 180f, 120f, false, p)
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
    external fun nativeSelectSlot(slot: Int)
    external fun nativeSaveWorld()
    external fun nativeGetInventory(): String
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
            val dt = ((now - lastTimeNs) / 1_000_000_000.0).toFloat().coerceIn(0.001f, 0.04f)
            lastTimeNs = now
            nativeFrame(dt)
        }
    }
}

class Activity : AndroidActivity() {
    private lateinit var rootLayout: FrameLayout
    private var glView: GLSurfaceView? = null

    private var joyPointerId = -1
    private var joyCenterX = 0f
    private var joyCenterY = 0f

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
    private val prefs by lazy { getSharedPreferences("omni_settings", Context.MODE_PRIVATE) }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.attributes.preferredDisplayModeId =
                display?.supportedModes?.maxByOrNull { it.refreshRate }?.modeId ?: 0
        }

        rootLayout = FrameLayout(this)
        setContentView(rootLayout)
        sensitivity=prefs.getFloat("sensitivity",1.0f)
        fov=prefs.getFloat("fov",70.0f)
        renderDistance=prefs.getInt("renderDistance",6)
        dayNightEnabled=prefs.getBoolean("dayNight",true)
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
        rootLayout.removeAllViews()

        val lobby = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.rgb(20, 24, 30))
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
        }

        val title = TextView(this).apply {
            text = "MINECRAFT : OMNI"
            textSize = 32f
            setTextColor(Color.WHITE)
            typeface = Typeface.DEFAULT_BOLD
            setShadowLayer(8f, 2f, 2f, Color.BLACK)
            setPadding(0, 0, 0, 40)
        }
        lobby.addView(title)

        fun createLobbyBtn(text: String, onClick: () -> Unit): Button {
            return Button(this).apply {
                setText(text)
                textSize = 16f
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    setColor(Color.argb(220, 60, 65, 75))
                    cornerRadius = 10f
                    setStroke(2, Color.WHITE)
                }
                layoutParams = LinearLayout.LayoutParams(400, 110).apply {
                    setMargins(0, 10, 0, 10)
                }
                setOnClickListener { onClick() }
            }
        }

        lobby.addView(createLobbyBtn("DÜNYAYA GİR") { startGame() })
        lobby.addView(createLobbyBtn("AYARLAR") { showSettingsDialog() })
        lobby.addView(createLobbyBtn("ÇIKIŞ") { finish() })
        lobby.addView(TextView(this).apply { text="Omni Craft • Sonsuz prosedürel dünya • Otomatik kayıt"; textSize=12f; setTextColor(Color.LTGRAY); setPadding(0,24,0,0) })

        rootLayout.addView(lobby)
        applyFullScreen()
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
        val hud = FrameLayout(this)

        // 1. Crosshair
        val chV = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(180, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(4, 32).apply { gravity = Gravity.CENTER }
        }
        val chH = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(180, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(32, 4).apply { gravity = Gravity.CENTER }
        }
        hud.addView(chV)
        hud.addView(chH)

        // 2. Sol Joystick Tabanı
        val joyBase = View(this).apply {
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(80, 40, 40, 40))
                setStroke(3, Color.argb(150, 200, 200, 200))
            }
            layoutParams = FrameLayout.LayoutParams(260, 260).apply {
                gravity = Gravity.BOTTOM or Gravity.START
                setMargins(60, 0, 0, 60)
            }
        }
        hud.addView(joyBase)

        // 3. Sağ Eylem Butonları (KIR ve ZIPLA)
        val rightActionArea = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            ).apply {
                gravity = Gravity.BOTTOM or Gravity.END
                setMargins(0, 0, 45, 45)
            }
        }

        val breakBtn = ImageButton(this).apply {
            setImageDrawable(VectorIconFactory.createBreakIcon(this@Activity))
            scaleType = ImageView.ScaleType.CENTER_INSIDE
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(160, 180, 45, 45))
                setStroke(2, Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(125, 125).apply { setMargins(8, 8, 8, 8) }
            setOnTouchListener { _, event ->
                when (event.actionMasked) {
                    MotionEvent.ACTION_DOWN -> { Engine.nativeSetBreaking(true); true }
                    MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> { Engine.nativeSetBreaking(false); true }
                    else -> false
                }
            }
        }

        val jumpBtn = ImageButton(this).apply {
            setImageDrawable(VectorIconFactory.createJumpIcon(this@Activity))
            scaleType = ImageView.ScaleType.CENTER_INSIDE
            background = GradientDrawable().apply {
                shape = GradientDrawable.OVAL
                setColor(Color.argb(160, 60, 60, 60))
                setStroke(2, Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(125, 125).apply { setMargins(8, 8, 8, 8) }
            setOnTouchListener { _, event ->
                when (event.actionMasked) {
                    MotionEvent.ACTION_DOWN -> { Engine.nativeSetJumpState(true); true }
                    MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> { Engine.nativeSetJumpState(false); true }
                    else -> false
                }
            }
        }

        rightActionArea.addView(breakBtn)
        rightActionArea.addView(jumpBtn)
        hud.addView(rightActionArea)

        // 4. MCPE 9'lu Hotbar
        val hotbarContainer = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
            ).apply {
                gravity = Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL
                bottomMargin = 14
            }
        }

        val hotbarSlots = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            background = GradientDrawable().apply {
                setColor(Color.argb(170, 25, 25, 25))
                cornerRadius = 10f
                setStroke(2, Color.argb(180, 100, 100, 100))
            }
            setPadding(4, 4, 4, 4)
        }

        val initialBlockIds = intArrayOf(9, 4, 19, 32, 36, 1, 2, 7, 44)

        for (i in 0 until 9) {
            val slotContainer = FrameLayout(this).apply {
                background = GradientDrawable().apply {
                    setColor(if (i == 0) Color.argb(180, 130, 130, 130) else Color.argb(60, 0, 0, 0))
                    cornerRadius = 8f
                    setStroke(if (i == 0) 3 else 1, if (i == 0) Color.WHITE else Color.GRAY)
                }
                layoutParams = LinearLayout.LayoutParams(92, 92).apply { setMargins(3, 3, 3, 3) }
            }

            val blockIcon = ImageView(this).apply {
                setImageDrawable(BlockIconFactory.getIcon(this@Activity, initialBlockIds[i]))
                scaleType = ImageView.ScaleType.FIT_CENTER
                layoutParams = FrameLayout.LayoutParams(70, 70).apply { gravity = Gravity.CENTER }
            }
            slotContainer.addView(blockIcon)

            val countText = TextView(this).apply {
                text = if (i == 3) "16" else "64"
                textSize = 10f
                setTextColor(Color.WHITE)
                setShadowLayer(3f, 1f, 1f, Color.BLACK)
                layoutParams = FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT
                ).apply {
                    gravity = Gravity.BOTTOM or Gravity.END
                    setMargins(0, 0, 6, 2)
                }
            }
            slotContainer.addView(countText)

            slotContainer.setOnClickListener {
                Engine.nativeSelectSlot(i)
                for (j in 0 until hotbarSlots.childCount) {
                    (hotbarSlots.getChildAt(j).background as GradientDrawable).apply {
                        setColor(if (j == i) Color.argb(180, 130, 130, 130) else Color.argb(60, 0, 0, 0))
                        setStroke(if (j == i) 3 else 1, if (j == i) Color.WHITE else Color.GRAY)
                    }
                }
            }

            hotbarSlots.addView(slotContainer)
        }
        hotbarContainer.addView(hotbarSlots)

        val invBtn = Button(this).apply {
            text = "•••"
            textSize = 15f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(170, 40, 40, 40))
                cornerRadius = 10f
                setStroke(2, Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(92, 92).apply { marginStart = 10 }
            setOnClickListener { showInventoryDialog() }
        }
        hotbarContainer.addView(invBtn)
        hud.addView(hotbarContainer)

        // 5. Duraklatma Butonu
        val pauseBtn = Button(this).apply {
            text = "II"
            textSize = 14f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(140, 35, 35, 35))
                cornerRadius = 8f
                setStroke(1, Color.WHITE)
            }
            layoutParams = FrameLayout.LayoutParams(80, 80).apply {
                gravity = Gravity.TOP or Gravity.CENTER_HORIZONTAL
                topMargin = 18
            }
            setOnClickListener { showPauseMenu() }
        }
        hud.addView(pauseBtn)

        // Çoklu Dokunmatik & Ekrana Dokunarak Blok Yerleştirme
        hud.setOnTouchListener { _, event ->
            val pointerIndex = event.actionIndex
            val pId = event.getPointerId(pointerIndex)
            val x = event.getX(pointerIndex)
            val y = event.getY(pointerIndex)

            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                    if (x < root.width * 0.4f && joyPointerId == -1) {
                        joyPointerId = pId
                        joyCenterX = joyBase.x + joyBase.width / 2f
                        joyCenterY = joyBase.y + joyBase.height / 2f
                        handleJoystick(x, y)
                    } else if (x >= root.width * 0.35f && camPointerId == -1) {
                        camPointerId = pId
                        lastCamX = x
                        lastCamY = y
                        touchDownX = x
                        touchDownY = y
                        touchDownTime = System.currentTimeMillis()
                    }
                }
                MotionEvent.ACTION_MOVE -> {
                    for (i in 0 until event.pointerCount) {
                        val currentId = event.getPointerId(i)
                        val px = event.getX(i)
                        val py = event.getY(i)

                        if (currentId == joyPointerId) {
                            handleJoystick(px, py)
                        } else if (currentId == camPointerId) {
                            val dx = (px - lastCamX) * 0.22f * sensitivity
                            val dy = (py - lastCamY) * 0.22f * sensitivity
                            Engine.nativeCameraInput(dx, dy)
                            lastCamX = px
                            lastCamY = py
                        }
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                    if (pId == joyPointerId) {
                        joyPointerId = -1
                        Engine.nativeInput(0f, 0f)
                    }
                    if (pId == camPointerId) {
                        val touchDuration = System.currentTimeMillis() - touchDownTime
                        val distMoved = Math.hypot((lastCamX - touchDownX).toDouble(), (lastCamY - touchDownY).toDouble())
                        // Ekrandaki bloğa kısa dokunulduğunda o noktaya blok yerleştir
                        if (touchDuration < 260 && distMoved < 35.0) {
                            Engine.nativeTapPlaceAt(lastCamX, lastCamY)
                        }
                        camPointerId = -1
                    }
                }
                MotionEvent.ACTION_CANCEL -> {
                    joyPointerId = -1
                    camPointerId = -1
                    Engine.nativeInput(0f, 0f)
                }
            }
            true
        }

        root.addView(hud)
        glView?.postDelayed({ if (Engine.nativeIsInitialized()) { Engine.nativeSetFov(fov); Engine.nativeSetRenderDistance(renderDistance); Engine.nativeSetDayNight(dayNightEnabled) } }, 120)
    }

    private fun handleJoystick(touchX: Float, touchY: Float) {
        var dx = touchX - joyCenterX
        var dy = touchY - joyCenterY
        val maxRadius = 130f
        val dist = Math.hypot(dx.toDouble(), dy.toDouble()).toFloat()

        if (dist > maxRadius) {
            dx = (dx / dist) * maxRadius
            dy = (dy / dist) * maxRadius
        }
        Engine.nativeInput(dx / maxRadius, -dy / maxRadius)
    }

    private fun showInventoryDialog() {
        val dialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.argb(235, 20, 20, 20))
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

                val slotContainer = FrameLayout(this).apply {
                    background = GradientDrawable().apply {
                        setColor(Color.argb(120, 50, 50, 50))
                        setStroke(2, Color.argb(150, 180, 180, 180))
                        cornerRadius = 8f
                    }
                    layoutParams = GridLayout.LayoutParams().apply {
                        width = 105
                        height = 105
                        setMargins(4, 4, 4, 4)
                    }
                }

                if (count > 0 && id > 0) {
                    val icon = ImageView(this).apply {
                        setImageDrawable(BlockIconFactory.getIcon(this@Activity, id))
                        scaleType = ImageView.ScaleType.FIT_CENTER
                        layoutParams = FrameLayout.LayoutParams(75, 75).apply { gravity = Gravity.CENTER }
                    }
                    slotContainer.addView(icon)

                    val countText = TextView(this).apply {
                        text = "$count"
                        textSize = 11f
                        setTextColor(Color.WHITE)
                        setShadowLayer(3f, 1f, 1f, Color.BLACK)
                        layoutParams = FrameLayout.LayoutParams(
                            ViewGroup.LayoutParams.WRAP_CONTENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT
                        ).apply {
                            gravity = Gravity.BOTTOM or Gravity.END
                            setMargins(0, 0, 6, 2)
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
            textSize = 14f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(200, 70, 70, 70))
                cornerRadius = 10f
                setStroke(2, Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(260, 95).apply { topMargin = 25 }
            setOnClickListener { dialog.dismiss(); applyFullScreen() }
        }
        layout.addView(closeBtn)

        dialog.setContentView(layout)
        dialog.show()
    }

    private fun showSettingsDialog() {
        val dialog=Dialog(this,android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val layout=LinearLayout(this).apply{orientation=LinearLayout.VERTICAL;gravity=Gravity.CENTER;setBackgroundColor(Color.rgb(20,24,30));setPadding(40,30,40,30)}
        fun label(t:String)=TextView(this).apply{text=t;textSize=16f;setTextColor(Color.WHITE);setPadding(0,10,0,5)}
        layout.addView(label("AYARLAR").apply{textSize=24f;typeface=Typeface.DEFAULT_BOLD})
        val sl=label("Kamera Hassasiyeti: ${(sensitivity*100).toInt()}%");layout.addView(sl)
        layout.addView(SeekBar(this).apply{max=200;progress=(sensitivity*100).toInt().coerceIn(20,200);setOnSeekBarChangeListener(object:SeekBar.OnSeekBarChangeListener{override fun onProgressChanged(b:SeekBar?,p:Int,u:Boolean){sensitivity=(p.coerceAtLeast(20)/100f);sl.text="Kamera Hassasiyeti: ${(sensitivity*100).toInt()}%"};override fun onStartTrackingTouch(b:SeekBar?){ };override fun onStopTrackingTouch(b:SeekBar?){ }})})
        val fl=label("Görüş Alanı: ${fov.toInt()}°");layout.addView(fl)
        layout.addView(SeekBar(this).apply{max=40;progress=(fov-55).toInt();setOnSeekBarChangeListener(object:SeekBar.OnSeekBarChangeListener{override fun onProgressChanged(b:SeekBar?,p:Int,u:Boolean){fov=(55+p).toFloat();fl.text="Görüş Alanı: ${fov.toInt()}°";if(Engine.nativeIsInitialized())Engine.nativeSetFov(fov)};override fun onStartTrackingTouch(b:SeekBar?){ };override fun onStopTrackingTouch(b:SeekBar?){ }})})
        val rl=label("Dünya Görüş Mesafesi: $renderDistance chunk");layout.addView(rl)
        layout.addView(SeekBar(this).apply{max=3;progress=(renderDistance-3).coerceIn(0,3);setOnSeekBarChangeListener(object:SeekBar.OnSeekBarChangeListener{override fun onProgressChanged(b:SeekBar?,p:Int,u:Boolean){renderDistance=(3+p).coerceIn(3,6);rl.text="Dünya Görüş Mesafesi: $renderDistance chunk";if(Engine.nativeIsInitialized())Engine.nativeSetRenderDistance(renderDistance)};override fun onStartTrackingTouch(b:SeekBar?){ };override fun onStopTrackingTouch(b:SeekBar?){ }})})
        layout.addView(Switch(this).apply{text="Gündüz / Gece Döngüsü";textSize=16f;setTextColor(Color.WHITE);isChecked=dayNightEnabled;setOnCheckedChangeListener{_,v->dayNightEnabled=v;if(Engine.nativeIsInitialized())Engine.nativeSetDayNight(v)}})
        layout.addView(Button(this).apply{text="Kaydet ve Geri Dön";layoutParams=LinearLayout.LayoutParams(420,105).apply{topMargin=24};setOnClickListener{prefs.edit().putFloat("sensitivity",sensitivity).putFloat("fov",fov).putInt("renderDistance",renderDistance).putBoolean("dayNight",dayNightEnabled).apply();if(Engine.nativeIsInitialized()){Engine.nativeSetFov(fov);Engine.nativeSetRenderDistance(renderDistance);Engine.nativeSetDayNight(dayNightEnabled)};dialog.dismiss();applyFullScreen()}})
        dialog.setContentView(layout);dialog.show();applyFullScreen()
    }

    private fun showPauseMenu() {
        val dialog = Dialog(this, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setBackgroundColor(Color.argb(235, 18, 18, 18))
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
                    setColor(Color.argb(200, 55, 55, 55))
                    setStroke(2, Color.WHITE)
                }
                layoutParams = LinearLayout.LayoutParams(380, 100).apply { setMargins(0, 10, 0, 10) }
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
        Engine.nativeDestroy()
    }
}
