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
#include "Includes/Utils.hpp"
#include "Includes/Macros.h"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "KittyMemory/MemoryPatch.hpp"

bool godModeEnabled = false;
bool infiniteHealthEnabled = false;
bool infiniteArmourEnabled = false;

#define RVA_TakeDamage    0x1AD5564
#define RVA_GetHealth     0x1AD5790
#define RVA_SetArmour     0x1AD5788

void (*old_TakeDamage)(void* self, float damage, void* force, void* enemy, bool hpOnly);

void new_TakeDamage(void* self, float damage, void* force, void* enemy, bool hpOnly) {
    if (godModeEnabled) return;
    old_TakeDamage(self, damage, force, enemy, hpOnly);
}

float (*old_GetHealth)(void* self);

float new_GetHealth(void* self) {
    if (infiniteHealthEnabled) return 999999.0f;
    return old_GetHealth(self);
}

void (*old_SetArmour)(void* self, float armour);

void new_SetArmour(void* self, float armour) {
    if (infiniteArmourEnabled) {
        old_SetArmour(self, 999999.0f);
        return;
    }
    old_SetArmour(self, armour);
}

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

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum, jint intValue, jlong longValue, jboolean boolean, jstring str) {
    switch (featNum) {
        case 0: godModeEnabled = boolean; break;
        case 1: infiniteHealthEnabled = boolean; break;
        case 2: infiniteArmourEnabled = boolean; break;
    }
}

#define targetLibName OBFUSCATE("libil2cpp.so")

void *hack_thread(void *) {
    LOGI(OBFUSCATE("Mod thread iniciado"));
    do {
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));
    LOGI(OBFUSCATE("libil2cpp.so cargada"));
    HOOK(targetLibName, RVA_TakeDamage, new_TakeDamage, old_TakeDamage);
    HOOK(targetLibName, RVA_GetHealth, new_GetHealth, old_GetHealth);
    HOOK(targetLibName, RVA_SetArmour, new_SetArmour, old_SetArmour);
    LOGI(OBFUSCATE("Hooks instalados"));
    return nullptr;
}

__attribute__((constructor))
void lib_main() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}
