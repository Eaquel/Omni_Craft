plugins {
    id("com.android.application") apply false
    id("org.jetbrains.kotlin.android") apply false
}

tasks.register<Delete>("clean") {
    delete(rootProject.layout.buildDirectory)
}
