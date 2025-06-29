#include <algorithm>
#include <jvmti.h>
#include <jni.h>
#include <cstdio>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

static std::pmr::unordered_map<std::string, std::vector<unsigned char>> classBytecodeMap;
static std::mutex mapMutex;
static jvmtiEnv *global_jvm_ti = nullptr;

const char *getErrorName(const jvmtiError err) {
    switch (err) {
        case JVMTI_ERROR_NONE: return "JVMTI_ERROR_NONE";
        case JVMTI_ERROR_INVALID_THREAD: return "JVMTI_ERROR_INVALID_THREAD";
        case JVMTI_ERROR_INVALID_THREAD_GROUP: return "JVMTI_ERROR_INVALID_THREAD_GROUP";
        case JVMTI_ERROR_INVALID_PRIORITY: return "JVMTI_ERROR_INVALID_PRIORITY";
        case JVMTI_ERROR_THREAD_NOT_SUSPENDED: return "JVMTI_ERROR_THREAD_NOT_SUSPENDED";
        case JVMTI_ERROR_THREAD_SUSPENDED: return "JVMTI_ERROR_THREAD_SUSPENDED";
        case JVMTI_ERROR_THREAD_NOT_ALIVE: return "JVMTI_ERROR_THREAD_NOT_ALIVE";
        case JVMTI_ERROR_INVALID_OBJECT: return "JVMTI_ERROR_INVALID_OBJECT";
        case JVMTI_ERROR_INVALID_CLASS: return "JVMTI_ERROR_INVALID_CLASS";
        case JVMTI_ERROR_CLASS_NOT_PREPARED: return "JVMTI_ERROR_CLASS_NOT_PREPARED";
        case JVMTI_ERROR_INVALID_METHODID: return "JVMTI_ERROR_INVALID_METHODID";
        case JVMTI_ERROR_INVALID_LOCATION: return "JVMTI_ERROR_INVALID_LOCATION";
        case JVMTI_ERROR_INVALID_FIELDID: return "JVMTI_ERROR_INVALID_FIELDID";
        case JVMTI_ERROR_INVALID_MODULE: return "JVMTI_ERROR_INVALID_MODULE";
        case JVMTI_ERROR_NO_MORE_FRAMES: return "JVMTI_ERROR_NO_MORE_FRAMES";
        case JVMTI_ERROR_OPAQUE_FRAME: return "JVMTI_ERROR_OPAQUE_FRAME";
        case JVMTI_ERROR_TYPE_MISMATCH: return "JVMTI_ERROR_TYPE_MISMATCH";
        case JVMTI_ERROR_INVALID_SLOT: return "JVMTI_ERROR_INVALID_SLOT";
        case JVMTI_ERROR_DUPLICATE: return "JVMTI_ERROR_DUPLICATE";
        case JVMTI_ERROR_NOT_FOUND: return "JVMTI_ERROR_NOT_FOUND";
        case JVMTI_ERROR_INVALID_MONITOR: return "JVMTI_ERROR_INVALID_MONITOR";
        case JVMTI_ERROR_NOT_MONITOR_OWNER: return "JVMTI_ERROR_NOT_MONITOR_OWNER";
        case JVMTI_ERROR_INTERRUPT: return "JVMTI_ERROR_INTERRUPT";
        case JVMTI_ERROR_INVALID_CLASS_FORMAT: return "JVMTI_ERROR_INVALID_CLASS_FORMAT";
        case JVMTI_ERROR_CIRCULAR_CLASS_DEFINITION: return "JVMTI_ERROR_CIRCULAR_CLASS_DEFINITION";
        case JVMTI_ERROR_FAILS_VERIFICATION: return "JVMTI_ERROR_FAILS_VERIFICATION";
        case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_ADDED: return
                    "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_ADDED";
        case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED: return
                    "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED";
        case JVMTI_ERROR_INVALID_TYPESTATE: return "JVMTI_ERROR_INVALID_TYPESTATE";
        case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED: return
                    "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED";
        case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED: return
                    "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED";
        case JVMTI_ERROR_UNSUPPORTED_VERSION: return "JVMTI_ERROR_UNSUPPORTED_VERSION";
        case JVMTI_ERROR_NAMES_DONT_MATCH: return "JVMTI_ERROR_NAMES_DONT_MATCH";
        case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_MODIFIERS_CHANGED: return
                    "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_MODIFIERS_CHANGED";
        case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_MODIFIERS_CHANGED: return
                    "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_MODIFIERS_CHANGED";
        case JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_ATTRIBUTE_CHANGED: return
                    "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_ATTRIBUTE_CHANGED";
        case JVMTI_ERROR_UNMODIFIABLE_CLASS: return "JVMTI_ERROR_UNMODIFIABLE_CLASS";
        case JVMTI_ERROR_UNMODIFIABLE_MODULE: return "JVMTI_ERROR_UNMODIFIABLE_MODULE";
        case JVMTI_ERROR_NOT_AVAILABLE: return "JVMTI_ERROR_NOT_AVAILABLE";
        case JVMTI_ERROR_MUST_POSSESS_CAPABILITY: return "JVMTI_ERROR_MUST_POSSESS_CAPABILITY";
        case JVMTI_ERROR_NULL_POINTER: return "JVMTI_ERROR_NULL_POINTER";
        case JVMTI_ERROR_ABSENT_INFORMATION: return "JVMTI_ERROR_ABSENT_INFORMATION";
        case JVMTI_ERROR_INVALID_EVENT_TYPE: return "JVMTI_ERROR_INVALID_EVENT_TYPE";
        case JVMTI_ERROR_ILLEGAL_ARGUMENT: return "JVMTI_ERROR_ILLEGAL_ARGUMENT";
        case JVMTI_ERROR_NATIVE_METHOD: return "JVMTI_ERROR_NATIVE_METHOD";
        case JVMTI_ERROR_CLASS_LOADER_UNSUPPORTED: return "JVMTI_ERROR_CLASS_LOADER_UNSUPPORTED";
        case JVMTI_ERROR_OUT_OF_MEMORY: return "JVMTI_ERROR_OUT_OF_MEMORY";
        case JVMTI_ERROR_ACCESS_DENIED: return "JVMTI_ERROR_ACCESS_DENIED";
        case JVMTI_ERROR_WRONG_PHASE: return "JVMTI_ERROR_WRONG_PHASE";
        case JVMTI_ERROR_INTERNAL: return "JVMTI_ERROR_INTERNAL";
        case JVMTI_ERROR_UNATTACHED_THREAD: return "JVMTI_ERROR_UNATTACHED_THREAD";
        case JVMTI_ERROR_INVALID_ENVIRONMENT: return "JVMTI_ERROR_INVALID_ENVIRONMENT";
        default: return "Unknown JVMTI error";
    }
}

void JNICALL classFileLoadHook(
    jvmtiEnv *JvmTi,
    JNIEnv *env,
    jclass class_being_redefined,
    jobject loader,
    const char *name,
    jobject protection_domain,
    jint class_data_len,
    const unsigned char *class_data,
    jint *new_class_data_len,
    unsigned char **new_class_data
) {
    printf("[+] Transforming class: %s\n", name);
    std::lock_guard lock(mapMutex);

    const auto it = classBytecodeMap.find(name);
    if (it != classBytecodeMap.end()) {
        const auto& data = it->second;
        auto* new_data = static_cast<unsigned char*>(malloc(data.size()));
        memcpy(new_data, data.data(), data.size());

        *new_class_data_len = static_cast<jint>(data.size());
        *new_class_data = new_data;

        printf("[+] Replaced class: %s (%d bytes)\n", name, *new_class_data_len);
    } else {
        *new_class_data = nullptr;
        *new_class_data_len = 0;
    }
}

bool ensureJvmTi(JNIEnv *env) {
    if (global_jvm_ti) return true;

    JavaVM *jvm = nullptr;
    if (env->GetJavaVM(&jvm) != JNI_OK || jvm == nullptr) {
        printf("[-] GetJavaVM failed\n");
        return false;
    }

    if (jvm->GetEnv(reinterpret_cast<void **>(&global_jvm_ti), JVMTI_VERSION_1_2) != JNI_OK || !global_jvm_ti) {
        printf("[-] GetEnv for JvmTi failed\n");
        return false;
    }

    jvmtiCapabilities caps{};
    jvmtiError err = global_jvm_ti->GetPotentialCapabilities(&caps);
    if (err != JVMTI_ERROR_NONE) {
        printf("[-] GetPotentialCapabilities failed: %s\n", getErrorName(err));
        return false;
    }

    err = global_jvm_ti->AddCapabilities(&caps);
    if (err != JVMTI_ERROR_NONE) {
        printf("[-] AddCapabilities failed: %s\n", getErrorName(err));
        return false;
    }

    jvmtiEventCallbacks callbacks{};
    callbacks.ClassFileLoadHook = &classFileLoadHook;
    global_jvm_ti->SetEventCallbacks(&callbacks, sizeof(callbacks));
    global_jvm_ti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);

    return true;
}

extern "C"
JNIEXPORT void JNICALL Java_org_example_Native_retransformClass
  (JNIEnv* env, jclass clazz, jobjectArray classes, jobjectArray bytesArray) {
    JavaVM* jvm;
    env->GetJavaVM(&jvm);

    if (!global_jvm_ti) {
        jvm->GetEnv(reinterpret_cast<void **>(&global_jvm_ti), JVMTI_VERSION_1_2);
        jvmtiCapabilities caps = {};
        global_jvm_ti->GetPotentialCapabilities(&caps);
        global_jvm_ti->AddCapabilities(&caps);

        jvmtiEventCallbacks callbacks = {};
        callbacks.ClassFileLoadHook = classFileLoadHook;
        global_jvm_ti->SetEventCallbacks(&callbacks, sizeof(callbacks));
        global_jvm_ti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);
    }

    jsize count = env->GetArrayLength(classes);
    if (count != env->GetArrayLength(bytesArray)) {
        printf("[-] Array length mismatch\n");
        return;
    }

    std::lock_guard lock(mapMutex);
    classBytecodeMap.clear();

    std::vector<jclass> toRetransform;

    for (jsize i = 0; i < count; ++i) {
        jobject classObj = env->GetObjectArrayElement(classes, i);
        jclass klass = env->GetObjectClass(classObj);
        auto byteArray = (jbyteArray) env->GetObjectArrayElement(bytesArray, i);

        if (!classObj || !byteArray) continue;

        jclass classClass = env->FindClass("java/lang/Class");
        jmethodID getName = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
        auto nameStr = (jstring) env->CallObjectMethod(classObj, getName);

        const char* utfName = env->GetStringUTFChars(nameStr, nullptr);
        std::string internalName = utfName;
        std::ranges::replace(internalName, '.', '/');
        env->ReleaseStringUTFChars(nameStr, utfName);
        env->DeleteLocalRef(nameStr);

        jsize len = env->GetArrayLength(byteArray);
        jbyte* data = env->GetByteArrayElements(byteArray, nullptr);

        classBytecodeMap[internalName] = std::vector(reinterpret_cast<unsigned char *>(data), reinterpret_cast<unsigned char *>(data) + len);

        jboolean modifiable = JNI_FALSE;
        global_jvm_ti->IsModifiableClass(klass, &modifiable);
        printf("Class %d is modifiable: %s\n", i, modifiable ? "true" : "false");

        env->ReleaseByteArrayElements(byteArray, data, JNI_ABORT);
        toRetransform.push_back(klass);

        env->DeleteLocalRef(klass);
        env->DeleteLocalRef(byteArray);
    }

    jvmtiError err = global_jvm_ti->RetransformClasses(toRetransform.size(), toRetransform.data());
    if (err != JVMTI_ERROR_NONE) {
        printf("[-] RetransformClasses failed: %s\n", getErrorName(err));
    } else {
        printf("[+] Retransform success\n");
    }
}


extern "C"
JNIEXPORT void JNICALL Java_org_example_Native_redefineClass(JNIEnv *env, jclass clazz, jobjectArray classes,
                                                             jobjectArray bytesArray) {
    if (!ensureJvmTi(env)) return;
    printf("[+]");

    const jsize count = env->GetArrayLength(classes);
    const jsize bytesCount = env->GetArrayLength(bytesArray);
    if (count != bytesCount) {
        printf("[-] Mismatched array lengths\n");
        return;
    }

    std::vector<jvmtiClassDefinition> defs(count);
    std::vector<jbyte *> bytePtrs(count);
    std::vector<jclass> classRefs(count);
    std::vector<jbyteArray> byteArrayRefs(count);

    for (jsize i = 0; i < count; ++i) {
        auto obj = (jclass) env->GetObjectArrayElement(classes, i);
        auto byteArr = (jbyteArray) env->GetObjectArrayElement(bytesArray, i);

        classRefs[i] = obj;
        byteArrayRefs[i] = byteArr;

        if (!obj || !byteArr) {
            printf("[-] Null class or byte[] at index %d\n", i);
            defs[i] = {nullptr, 0, nullptr};
            bytePtrs[i] = nullptr;
            continue;
        }

        jsize len = env->GetArrayLength(byteArr);
        jbyte *data = env->GetByteArrayElements(byteArr, nullptr);

        if (!data) {
            printf("[-] Failed to get byte array at index %d\n", i);
            defs[i] = {nullptr, 0, nullptr};
            bytePtrs[i] = nullptr;
            continue;
        }

        defs[i] = {obj, len, reinterpret_cast<const unsigned char *>(data)};
        bytePtrs[i] = data;
    }

    // 安全地呼叫
    jvmtiError err = global_jvm_ti->RedefineClasses(count, defs.data());
    if (err != JVMTI_ERROR_NONE) {
        printf("[-] RedefineClasses failed: %s\n", getErrorName(err));
    } else {
        printf("[+] RedefineClasses success for %d classes\n", count);
    }

    // 釋放所有元素
    for (jsize i = 0; i < count; ++i) {
        if (bytePtrs[i]) {
            env->ReleaseByteArrayElements(byteArrayRefs[i], bytePtrs[i], JNI_ABORT);
        }
        if (classRefs[i]) env->DeleteLocalRef(classRefs[i]);
        if (byteArrayRefs[i]) env->DeleteLocalRef(byteArrayRefs[i]);
    }

}
