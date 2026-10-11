#include <list>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <string>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"
#include "Menu/Menu.h"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"
#include "KittyMemory/MemoryPatch.h"
#include "KittyScanner/KittyScanner.h"

// ============================================================
//  JUSTICE RIVALS 3 - MOD MENU
//  God Mode + Vida Infinita + Armadura Infinita
//  by MEXAMAFIA
// ============================================================

// RVAs encontradas en el dump.cs
#define RVA_TakeDamage    0x1AD5564
#define RVA_GetHealth     0x1AD5790
#define RVA_SetArmour     0x1AD5788

// Variables globales (se activan desde el menú)
bool godModeEnabled = false;
bool infiniteHealthEnabled = false;
bool infiniteArmourEnabled = false;

// ============================================================
//  HOOK 1: TakeDamage - BLOQUEA TODO EL DAÑO
// ============================================================
void (*old_TakeDamage)(void* self, float damage, void* force, void* enemy, bool hpOnly);

void new_TakeDamage(void* self, float damage, void* force, void* enemy, bool hpOnly) {
    if (godModeEnabled) {
        return;
    }
    old_TakeDamage(self, damage, force, enemy, hpOnly);
}

// ============================================================
//  HOOK 2: GetHealth - VIDA AL MÁXIMO
// ============================================================
float (*old_GetHealth)(void* self);

float new_GetHealth(void* self) {
    if (infiniteHealthEnabled) {
        return 999999.0f;
    }
    return old_GetHealth(self);
}

// ============================================================
//  HOOK 3: SetArmour - ARMADURA AL MÁXIMO
// ============================================================
void (*old_SetArmour)(void* self, float armour);

void new_SetArmour(void* self, float armour) {
    if (infiniteArmourEnabled) {
        old_SetArmour(self, 999999.0f);
        return;
    }
    old_SetArmour(self, armour);
}

// ============================================================
//  LISTA DE FEATURES DEL MENÚ
// ============================================================
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
        OBFUSCATE("Toggle_God Mode"),
        OBFUSCATE("Toggle_Vida Infinita"),
        OBFUSCATE("Toggle_Armadura Infinita"),
        OBFUSCATE("Category_--- Info ---"),
        OBFUSCATE("RichTextView_Mod by MEXAMAFIA"),
    };

    int TotalFeatures = sizeof(features) / sizeof(const char *);
    ret = (jobjectArray)env->NewObjectArray(TotalFeatures, env->FindClass("java/lang/String"), env->NewStringUTF(""));
    for (int i = 0; i < TotalFeatures; i++) {
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    }
    return ret;
}

// ============================================================
//  ACTUALIZACIÓN DE FEATURES (cuando tocas un toggle)
// ============================================================
void Changes(JNIEnv *env, jclass clazz, jobject obj, jint feature, jboolean boolean, jstring str) {
    switch (feature) {
        case 0: godModeEnabled = boolean; break;
        case 1: infiniteHealthEnabled = boolean; break;
        case 2: infiniteArmourEnabled = boolean; break;
    }
}

// ============================================================
//  LIB TARGET
// ============================================================
#define targetLibName OBFUSCATE("libil2cpp.so")

// ============================================================
//  HILO PRINCIPAL: INSTALA LOS HOOKS
// ============================================================
void *hack_thread(void *) {
    LOGI(OBFUSCATE("Mod thread iniciado"));

    // Esperar a que libil2cpp.so esté cargada
    do {
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));

    LOGI(OBFUSCATE("libil2cpp.so cargada"));

    // Instalar los hooks con Dobby
    HOOK(targetLibName, RVA_TakeDamage, new_TakeDamage, old_TakeDamage);
    HOOK(targetLibName, RVA_GetHealth, new_GetHealth, old_GetHealth);
    HOOK(targetLibName, RVA_SetArmour, new_SetArmour, old_SetArmour);

    LOGI(OBFUSCATE("Hooks instalados correctamente"));
    return nullptr;
}

// ============================================================
//  CONSTRUCTOR: ARRANCA EL HILO
// ============================================================
__attribute__((constructor))
void lib_main() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}

// ============================================================
//  JNI_OnLoad: SE EJECUTA AL CARGAR LA LIBRERÍA
// ============================================================
extern "C" jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}
