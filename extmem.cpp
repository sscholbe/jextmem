#include "extmem.h"
#include <Windows.h>
#include <sstream>

jclass FindWin32Exception(JNIEnv *env) {
    jclass win32ex = env->FindClass("extmem/Win32Exception");
    if (win32ex == NULL) {
        env->ThrowNew(env->FindClass("java/lang/ClassNotFoundException"), "Win32Exception not found");
        return NULL;
    }
    return win32ex;
}

JNIEXPORT jlong JNICALL Java_extmem_Memory_openProcess(JNIEnv *env, jclass obj, jstring windowTitle) {
    jclass win32ex = FindWin32Exception(env);
    if (!win32ex) {
        return -1;
    }

    jboolean copy;
    const char *utf = env->GetStringUTFChars(windowTitle, &copy);

    // Find the window by its title
    HWND window = FindWindowA(NULL, utf);
    if (!window) {
        env->ThrowNew(win32ex, "Could not find window");
        return -1;
    }

    if (copy) {
        env->ReleaseStringUTFChars(windowTitle, utf);
    }

    DWORD pid;

    // Find the process ID of the window
    GetWindowThreadProcessId(window, &pid);
    if (!pid) {
        DWORD err = GetLastError();
        std::ostringstream ss;
        ss << "GetWindowThreadProcessId() failed with error code 0x" << std::hex << err;
        env->ThrowNew(win32ex, ss.str().c_str());
        return -1;
    }

    // Open the process using the PID
    HANDLE handle = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (!handle) {
        DWORD err = GetLastError();
        std::ostringstream ss;
        ss << "OpenProcess() failed with error code 0x" << std::hex << err;
        env->ThrowNew(win32ex, ss.str().c_str());
        return -1;
    }

    return (jlong) handle;
}

JNIEXPORT void JNICALL Java_extmem_Memory_closeProcess(JNIEnv *env, jclass obj, jlong handle) {
    jclass win32ex = FindWin32Exception(env);
    if (!win32ex) {
        return;
    }

    if (!CloseHandle((HANDLE) handle)) {
        DWORD err = GetLastError();
        std::ostringstream ss;
        ss << "CloseHandle() failed with error code 0x" << std::hex << err;
        env->ThrowNew(win32ex, ss.str().c_str());
    }
}

JNIEXPORT jbyteArray JNICALL Java_extmem_Memory_readBytes(JNIEnv *env, jclass obj, jlong handle, jlong address, jint count) {
    jclass win32ex = FindWin32Exception(env);
    if (!win32ex) {
        return NULL;
    }

    jbyte *data = new jbyte[count];
    jbyteArray bytes = env->NewByteArray(count);
    if (!data || !bytes) {
        env->ThrowNew(env->FindClass("java/lang/OutOfMemoryError"), "Could not allocate temporary storage");
        return NULL;
    }

    SIZE_T read;
    if (!ReadProcessMemory((HANDLE) handle, (LPCVOID) address, data, count, &read)) {
        DWORD err = GetLastError();
        std::ostringstream ss;
        ss << "ReadProcessMemory() failed with error code 0x" << std::hex << err;
        env->ThrowNew(win32ex, ss.str().c_str());
        return NULL;
    }

    if (count != read) {
        std::ostringstream ss;
        ss << "Number of expected (" << count << ") and read (" << read << ") bytes mismatch";
        env->ThrowNew(win32ex, ss.str().c_str());
        return NULL;
    }

    env->SetByteArrayRegion(bytes, 0, count, data);
    return bytes;
}
