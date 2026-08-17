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
import java.text.SimpleDateFormat
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
    private var logDir: File? = null

    fun init(context: Context) {
        val docsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS)
        logDir = File(docsDir, "Craft_Log").apply { if (!exists()) mkdirs() }
        Thread.setDefaultUncaughtExceptionHandler { _, throwable ->
            val sw = StringWriter()
            throwable.printStackTrace(PrintWriter(sw))
            Log.e(TAG, "Fatal Crash: $sw")
        }
    }

    fun getLogDirPath(): String = logDir?.absolutePath ?: ""
}

// 1. Kodla Saf Piksel Art Minecraft Blok Önizleme Üreteci
object BlockIconFactory {
    private val iconCache = HashMap<Int, Bitmap>()

    fun getIcon(context: Context, blockId: Int, size: Int = 80): Drawable {
        val cached = iconCache[blockId]
        if (cached != null) return BitmapDrawable(context.resources, cached)

        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG)

        // 3D İzometrik Voxel Blok Çizimi
        val half = size / 2f
        val quarter = size / 4f
        val eighth = size / 8f

        val topCol: Int
        val sideLCol: Int
        val sideRCol: Int

        when (blockId) {
            1 -> { // Grass
                topCol = Color.rgb(80, 175, 45)
                sideLCol = Color.rgb(90, 60, 40)
                sideRCol = Color.rgb(115, 75, 50)
            }
            2 -> { // Dirt
                topCol = Color.rgb(115, 75, 50)
                sideLCol = Color.rgb(90, 60, 40)
                sideRCol = Color.rgb(130, 85, 55)
            }
            3 -> { // Stone
                topCol = Color.rgb(140, 140, 140)
                sideLCol = Color.rgb(110, 110, 110)
                sideRCol = Color.rgb(160, 160, 160)
            }
            4 -> { // Cobble
                topCol = Color.rgb(120, 120, 120)
                sideLCol = Color.rgb(95, 95, 95)
                sideRCol = Color.rgb(140, 140, 140)
            }
            5 -> { // Sand
                topCol = Color.rgb(225, 215, 155)
                sideLCol = Color.rgb(195, 185, 125)
                sideRCol = Color.rgb(240, 230, 170)
            }
            7, 10 -> { // Oak / Birch Log
                topCol = Color.rgb(160, 125, 80)
                sideLCol = Color.rgb(90, 65, 40)
                sideRCol = Color.rgb(115, 85, 55)
            }
            9, 11 -> { // Planks
                topCol = Color.rgb(175, 130, 75)
                sideLCol = Color.rgb(140, 100, 55)
                sideRCol = Color.rgb(195, 145, 85)
            }
            19 -> { // Diamond Ore
                topCol = Color.rgb(140, 140, 140)
                sideLCol = Color.rgb(50, 210, 210)
                sideRCol = Color.rgb(160, 160, 160)
            }
            32 -> { // Crafting Table
                topCol = Color.rgb(180, 135, 80)
                sideLCol = Color.rgb(145, 105, 60)
                sideRCol = Color.rgb(130, 90, 50)
            }
            36 -> { // Torch
                topCol = Color.YELLOW
                sideLCol = Color.rgb(130, 90, 50)
                sideRCol = Color.rgb(160, 115, 65)
            }
            44 -> { // TNT
                topCol = Color.rgb(200, 50, 50)
                sideLCol = Color.rgb(160, 40, 40)
                sideRCol = Color.WHITE
            }
            else -> {
                topCol = Color.rgb(150, 150, 150)
                sideLCol = Color.rgb(110, 110, 110)
                sideRCol = Color.rgb(180, 180, 180)
            }
        }

        // Üst Yüzey (Elmas Şeklinde Eksen)
        val topPath = Path().apply {
            moveTo(half, eighth)
            lineTo(size - eighth, quarter + eighth)
            lineTo(half, half + eighth)
            lineTo(eighth, quarter + eighth)
            close()
        }
        p.color = topCol
        c.drawPath(topPath, p)

        // Sol Yüzey
        val leftPath = Path().apply {
            moveTo(eighth, quarter + eighth)
            lineTo(half, half + eighth)
            lineTo(half, size - eighth)
            lineTo(eighth, size - quarter)
            close()
        }
        p.color = sideLCol
        c.drawPath(leftPath, p)

        // Sağ Yüzey
        val rightPath = Path().apply {
            moveTo(half, half + eighth)
            lineTo(size - eighth, quarter + eighth)
            lineTo(size - eighth, size - quarter)
            lineTo(half, size - eighth)
            close()
        }
        p.color = sideRCol
        c.drawPath(rightPath, p)

        iconCache[blockId] = bmp
        return BitmapDrawable(context.resources, bmp)
    }
}

// 2. Vektörel Buton İkon Çizicisi (Metin Yok: Kazma, 3D Küp, Zıplama Oku)
object VectorIconFactory {
    fun createBreakIcon(context: Context, size: Int = 110): Drawable {
        val bmp = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888)
        val c = Canvas(bmp)
        val p = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.STROKE
            strokeCap = Paint.Cap.ROUND
        }

        // Kazma Sapı
        p.color = Color.rgb(170, 120, 70)
        p.strokeWidth = size * 0.10f
        c.drawLine(size * 0.25f, size * 0.75f, size * 0.70f, size * 0.30f, p)

        // Kazma Ucu (Demir Arkı)
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
            style = Paint.Style.STROKE
            strokeWidth = size * 0.08f
            color = Color.WHITE
            strokeJoin = Paint.Join.ROUND
        }

        // 3D Vektörel İzometrik Küp İskeleti
        val h = size / 2f
        val q = size / 4f
        val e = size * 0.15f

        // Dış hatlar
        c.drawLine(h, e, size - e, q + e * 0.5f, p)
        c.drawLine(size - e, q + e * 0.5f, size - e, size - q, p)
        c.drawLine(size - e, size - q, h, size - e * 0.5f, p)
        c.drawLine(h, size - e * 0.5f, e, size - q, p)
        c.drawLine(e, size - q, e, q + e * 0.5f, p)
        c.drawLine(e, q + e * 0.5f, h, e, p)

        // İç bağlantı Y çizgisi
        c.drawLine(h, e, h, h + e * 0.5f, p)
        c.drawLine(h, h + e * 0.5f, size - e, q + e * 0.5f, p)
        c.drawLine(h, h + e * 0.5f, e, q + e * 0.5f, p)
        c.drawLine(h, h + e * 0.5f, h, size - e * 0.5f, p)

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

        // MCPE Yukarı Zıplama Vektörü
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
            layoutParams = FrameLayout.LayoutParams(4, 32).apply { gravity = Gravity.CENTER }
        }
        val chH = View(this).apply {
            background = GradientDrawable().apply { setColor(Color.argb(180, 255, 255, 255)) }
            layoutParams = FrameLayout.LayoutParams(32, 4).apply { gravity = Gravity.CENTER }
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

        fun createDpadBtn(symbol: String, rule: Int): Button {
            return Button(this).apply {
                text = symbol
                textSize = 18f
                setTextColor(Color.WHITE)
                background = GradientDrawable().apply {
                    setColor(Color.argb(120, 30, 30, 30))
                    cornerRadius = 14f
                    setStroke(2, Color.argb(140, 200, 200, 200))
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

        // 3. Sağ Vektörel SVG Eylem Butonları (Yazısız: Kazma, 3D Küp, Zıpla)
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

        fun createSvgActionBtn(iconDrawable: Drawable, bgColor: Int, onClick: () -> Unit): ImageButton {
            return ImageButton(this).apply {
                setImageDrawable(iconDrawable)
                scaleType = ImageView.ScaleType.CENTER_INSIDE
                background = GradientDrawable().apply {
                    shape = GradientDrawable.OVAL
                    setColor(bgColor)
                    setStroke(2, Color.argb(200, 255, 255, 255))
                }
                layoutParams = LinearLayout.LayoutParams(125, 125).apply {
                    setMargins(8, 8, 8, 8)
                }
                setOnClickListener { onClick() }
            }
        }

        rightActionArea.addView(createSvgActionBtn(VectorIconFactory.createBreakIcon(this), Color.argb(160, 180, 45, 45)) { Engine.nativeTap(0) })
        rightActionArea.addView(createSvgActionBtn(VectorIconFactory.createPlaceIcon(this), Color.argb(160, 45, 150, 55)) { Engine.nativeTap(1) })
        rightActionArea.addView(createSvgActionBtn(VectorIconFactory.createJumpIcon(this),  Color.argb(160, 60, 60, 60))   { Engine.nativeJump() })
        hud.addView(rightActionArea)

        // 4. MCPE 9'lu Hotbar (Blok Önizlemeli + Sayı Göstergeli)
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

        // Slot Block ID listesi: Tahta, Kırıktaş, Elmas, Çalışma Masası, Meşale, Çimen, Toprak, Kütük, TNT
        val initialBlockIds = intArrayOf(9, 4, 19, 32, 36, 1, 2, 7, 44)

        for (i in 0 until 9) {
            val slotContainer = FrameLayout(this).apply {
                background = GradientDrawable().apply {
                    setColor(if (i == 0) Color.argb(180, 130, 130, 130) else Color.argb(60, 0, 0, 0))
                    cornerRadius = 8f
                    setStroke(if (i == 0) 3 else 1, if (i == 0) Color.WHITE else Color.GRAY)
                }
                layoutParams = LinearLayout.LayoutParams(92, 92).apply {
                    setMargins(3, 3, 3, 3)
                }
            }

            // Blok Önizleme Resmi
            val blockIcon = ImageView(this).apply {
                setImageDrawable(BlockIconFactory.getIcon(this@Activity, initialBlockIds[i]))
                scaleType = ImageView.ScaleType.FIT_CENTER
                layoutParams = FrameLayout.LayoutParams(70, 70).apply {
                    gravity = Gravity.CENTER
                }
            }
            slotContainer.addView(blockIcon)

            // Eşya Miktarı
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

        // MCPE "•••" Envanter Butonu
        val invBtn = Button(this).apply {
            text = "•••"
            textSize = 15f
            setTextColor(Color.WHITE)
            background = GradientDrawable().apply {
                setColor(Color.argb(170, 40, 40, 40))
                cornerRadius = 10f
                setStroke(2, Color.WHITE)
            }
            layoutParams = LinearLayout.LayoutParams(92, 92).apply {
                marginStart = 10
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

        // Dokunmatik Kamera Katmanı
        hud.setOnTouchListener { _, event ->
            val pIdx = event.actionIndex
            val pId = event.getPointerId(pIdx)
            val x = event.getX(pIdx)
            val y = event.getY(pIdx)

            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
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
                        layoutParams = FrameLayout.LayoutParams(75, 75).apply {
                            gravity = Gravity.CENTER
                        }
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
