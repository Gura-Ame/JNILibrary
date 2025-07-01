#include <jvmti.h>
#include <jni.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <cstring>

std::string toCppString(JNIEnv *env, jstring str) {
    const char *utf = env->GetStringUTFChars(str, nullptr);
    std::string name(utf);
    env->ReleaseStringUTFChars(str, utf);
    return name;
}

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

static std::pmr::unordered_map<std::string, std::vector<unsigned char> > classBytecodeMap;
static std::mutex mapMutex;
static jvmtiEnv *jvmti = nullptr;

static void JNICALL onClassLoad(jvmtiEnv *, JNIEnv *, jclass, jobject, const char *name,
                                jobject, jint, const unsigned char *, jint *out_len, unsigned char **out_data) {
    std::lock_guard lock(mapMutex);
    auto it = classBytecodeMap.find(name);
    if (it == classBytecodeMap.end()) return;

    const auto &data = it->second;
    *out_len = static_cast<jint>(data.size());
    *out_data = static_cast<unsigned char *>(malloc(data.size()));
    memcpy(*out_data, data.data(), data.size());
    printf("[+] Replaced class: %s (%d bytes)\n", name, *out_len);
}

static bool initJvmti(JNIEnv *env) {
    if (jvmti) return true;

    JavaVM *jvm = nullptr;
    if (env->GetJavaVM(&jvm) != JNI_OK || !jvm ||
        jvm->GetEnv(reinterpret_cast<void **>(&jvmti), JVMTI_VERSION_1_2) != JNI_OK || !jvmti) {
        printf("[-] Failed to obtain JVMTI\n");
        return false;
    }

    jvmtiCapabilities caps{};
    if (jvmti->GetPotentialCapabilities(&caps) != JVMTI_ERROR_NONE ||
        jvmti->AddCapabilities(&caps) != JVMTI_ERROR_NONE) {
        printf("[-] Failed to add capabilities\n");
        return false;
    }

    jvmtiEventCallbacks cb{};
    cb.ClassFileLoadHook = onClassLoad;
    jvmti->SetEventCallbacks(&cb, sizeof(cb));
    jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, nullptr);
    return true;
}

static std::string getInternalName(JNIEnv *env, jobject cls) {
    jclass clsCls = env->FindClass("java/lang/Class");
    jmethodID mid = env->GetMethodID(clsCls, "getName", "()Ljava/lang/String;");
    jstring jname = (jstring) env->CallObjectMethod(cls, mid);
    std::string name = toCppString(env, jname);
    std::ranges::replace(name, '.', '/');
    env->DeleteLocalRef(jname);
    return name;
}

extern "C" JNIEXPORT void JNICALL
Java_org_example_Native_retransformClass(JNIEnv *env, jclass, jobjectArray classes, jobjectArray bytesArray) {
    if (!initJvmti(env)) return;

    const jsize count = env->GetArrayLength(classes);
    if (count != env->GetArrayLength(bytesArray)) {
        printf("[-] Array length mismatch\n");
        return;
    }

    std::vector<jclass> toRetransform; {
        std::lock_guard lock(mapMutex);
        classBytecodeMap.clear();
        for (jsize i = 0; i < count; ++i) {
            auto cls = static_cast<jclass>(env->GetObjectArrayElement(classes, i));
            auto arr = static_cast<jbyteArray>(env->GetObjectArrayElement(bytesArray, i));
            if (!cls || !arr) continue;

            std::string name = getInternalName(env, cls);
            jsize len = env->GetArrayLength(arr);
            jbyte *data = env->GetByteArrayElements(arr, nullptr);

            classBytecodeMap[name] = std::vector(
                reinterpret_cast<unsigned char *>(data),
                reinterpret_cast<unsigned char *>(data) + len
            );

            jboolean mod = JNI_FALSE;
            jvmti->IsModifiableClass(cls, &mod);
            printf("Class %d modifiable: %s\n", i, mod ? "true" : "false");

            env->ReleaseByteArrayElements(arr, data, JNI_ABORT);
            env->DeleteLocalRef(arr);
            toRetransform.push_back(cls);
        }
    }

    jvmtiError err = jvmti->RetransformClasses(toRetransform.size(), toRetransform.data());
    printf("%s\n", err == JVMTI_ERROR_NONE ? "[+] Retransform success" : "[-] Retransform failed");
}

extern "C" JNIEXPORT void JNICALL
Java_org_example_Native_redefineClass(JNIEnv *env, jclass, jobjectArray classes, jobjectArray bytesArray) {
    if (!initJvmti(env)) return;

    const jsize count = env->GetArrayLength(classes);
    if (count != env->GetArrayLength(bytesArray)) {
        printf("[-] Mismatched array lengths\n");
        return;
    }

    std::vector<jvmtiClassDefinition> defs(count);
    std::vector<jbyte *> ptrs(count);
    std::vector<jclass> classRefs(count);
    std::vector<jbyteArray> arrayRefs(count);

    for (jsize i = 0; i < count; ++i) {
        jclass cls = static_cast<jclass>(env->GetObjectArrayElement(classes, i));
        jbyteArray arr = static_cast<jbyteArray>(env->GetObjectArrayElement(bytesArray, i));
        classRefs[i] = cls;
        arrayRefs[i] = arr;

        if (!cls || !arr) {
            defs[i] = {nullptr, 0, nullptr};
            continue;
        }

        jsize len = env->GetArrayLength(arr);
        jbyte *data = env->GetByteArrayElements(arr, nullptr);
        defs[i] = {cls, len, reinterpret_cast<unsigned char *>(data)};
        ptrs[i] = data;
    }

    jvmtiError err = jvmti->RedefineClasses(count, defs.data());
    printf("%s for %d classes%s", err == JVMTI_ERROR_NONE ? "[+] Redefine success" : "[-] Redefine failed", count, err==JVMTI_ERROR_NONE ? ".\n" : getErrorName(err));

    for (jsize i = 0; i < count; ++i) {
        if (ptrs[i]) env->ReleaseByteArrayElements(arrayRefs[i], ptrs[i], JNI_ABORT);
        if (classRefs[i]) env->DeleteLocalRef(classRefs[i]);
        if (arrayRefs[i]) env->DeleteLocalRef(arrayRefs[i]);
    }
}

static jclass optionalClass;
static jmethodID ofMethod;
static jmethodID emptyMethod;

void prepareOptional(JNIEnv *env) {
    if (optionalClass == nullptr) {
        optionalClass = env->FindClass("java/util/Optional");
    }
    if (ofMethod == nullptr) {
        ofMethod = env->GetStaticMethodID(optionalClass, "of", "(Ljava/lang/Object;)Ljava/util/Optional;");
    }
    if (emptyMethod == nullptr) {
        emptyMethod = env->GetStaticMethodID(optionalClass, "empty", "()Ljava/util/Optional;");
    }
}

jobject ofOptional(JNIEnv *env, jobject obj) {
    prepareOptional(env);

    if (obj != nullptr) {
        return env->CallStaticObjectMethod(optionalClass, ofMethod, obj);
    }

    return env->CallStaticObjectMethod(optionalClass, emptyMethod);
}

extern "C" JNIEXPORT jobject JNICALL
Java_org_example_Native_accessClass(JNIEnv *env, jclass, jstring className) {
    std::string name = toCppString(env, className);
    std::ranges::replace(name, '.', '/');

    jclass cls = env->FindClass(name.c_str());
    return ofOptional(env, cls);
}
