-keepattributes SourceFile,LineNumberTable
-keepattributes *Annotation*
-keepattributes Signature

-keepclasseswithmembernames class * {
    native <methods>;
}

-keep class com.omni.craft.** { *; }
-keepclassmembers class com.omni.craft.** { *; }

-keep public class * extends android.app.Activity
-keep public class * extends android.app.Application
