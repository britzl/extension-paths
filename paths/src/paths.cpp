#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "paths.h"

#define EXTENSION_NAME Paths
#define LIB_NAME "Paths"
#define MODULE_NAME "paths"

// Defold SDK
#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

/**
 * IOS/OSX (declared in paths.mm)
 */
#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_OSX)


/**
 * LINUX
 */
#elif defined(DM_PLATFORM_LINUX)

#include <unistd.h> // readlink
#include <sys/auxv.h> // getauxval
#include <libgen.h> // dirname

int Platform_ApplicationPath(char* path_out, uint32_t path_len)
{
    ssize_t ret = readlink("/proc/self/exe", path_out, path_len);
    if (ret < 0 || ret > path_len)
    {
        const char* relative_path = (const char*)getauxval(AT_EXECFN); // Pathname used to execute program
        if (!relative_path)
        {
            path_out[0] = '.';
            path_out[1] = '\n';
        }
        else
        {
            char *absolute_path = realpath(relative_path, NULL); // realpath() resolve a pathname
            if (!absolute_path)
            {
                path_out[0] = '.';
                path_out[1] = '\n';
            }
            else
            {
                if (dmStrlCpy(path_out, dirname(absolute_path), path_len) >= path_len) // dirname() returns the string up to, but not including, the final '/'
                {
                    path_out[0] = '.';
                    path_out[1] = '\n';
                }
                free(absolute_path);
            }
        }
    }
    return 1;
}




/**
 * HTML5
 */
#elif defined(DM_PLATFORM_HTML5)

#include <emscripten.h>

int Platform_ApplicationPath(char* path_out, uint32_t path_len)
{
    char* bundlePath = (char*)EM_ASM_INT({
        var jsString = location.href.substring(0, location.href.lastIndexOf("/"));
        var lengthBytes = lengthBytesUTF8(jsString)+1; // 'jsString.length' would return the length of the string as UTF-16 units, but Emscripten C strings operate as UTF-8.
        var stringOnWasmHeap = _malloc(lengthBytes);
        stringToUTF8(jsString, stringOnWasmHeap, lengthBytes+1);
        return stringOnWasmHeap;
    },0);

    if (dmStrlCpy(path_out, bundlePath, path_len) >= path_len)
    {
        path_out[0] = 0;
        return 0;
    }
    return 1;
}


/**
 * WINDOWS
 */
#elif defined(DM_PLATFORM_WINDOWS)

int Platform_ApplicationPath(char* path_out, uint32_t path_len)
{
    assert(path_len > 0);
    assert(path_len >= MAX_PATH);
    size_t ret = GetModuleFileNameA(GetModuleHandle(NULL), path_out, path_len);
    if (ret > 0 && ret < path_len) {
        size_t i = strlen(path_out);
        do
        {
            i -= 1;
            if (path_out[i] == '\\')
            {
                path_out[i] = 0;
                break;
            }
        }
        while (i >= 0);
    }
    else
    {
        path_out[0] = '.';
        path_out[1] = '\n';
    }
    return 1;
}


/**
 * ANDROID
 */
#elif defined(DM_PLATFORM_ANDROID)

int Platform_ApplicationPath(char* path_out, uint32_t path_len)
{
    struct android_app* app = dmGraphics::GetNativeAndroidApp();
    ANativeActivity* activity = app->activity;
    JNIEnv* env = 0;
    activity->vm->AttachCurrentThread( &env, 0);

    jclass activity_class = env->FindClass("android/app/NativeActivity");
    jmethodID get_files_dir_method = env->GetMethodID(activity_class, "getFilesDir", "()Ljava/io/File;");
    jobject files_dir_obj = env->CallObjectMethod(activity->clazz, get_files_dir_method);
    jclass file_class = env->FindClass("java/io/File");
    jmethodID getPathMethod = env->GetMethodID(file_class, "getParent", "()Ljava/lang/String;");
    jstring path_obj = (jstring) env->CallObjectMethod(files_dir_obj, getPathMethod);

    int res = 1;

    if (path_obj) {
        const char* filesDir = env->GetStringUTFChars(path_obj, NULL);

        if (dmStrlCpy(path_out, filesDir, path_len) >= path_len) {
            res = 0;
        }
        env->ReleaseStringUTFChars(path_obj, filesDir);
    } else {
        res = 0;
    }
    activity->vm->DetachCurrentThread();
    return res;
}

#endif // platforms






static int ApplicationPath(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    char application_path[4096 + 2]; // Linux PATH_MAX is defined to 4096. Windows MAX_PATH is 260.
    int r = Platform_ApplicationPath(application_path, sizeof(application_path));
    if (r != 1)
    {
        luaL_error(L, "Unable to locate application path: (%d)", r);
    }
    lua_pushstring(L, application_path);

    return 1;
}



static const luaL_reg Module_methods[] =
{
    {"application", ApplicationPath},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializePaths(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializePaths(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizePaths(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizePaths(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}


DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializePaths, AppFinalizePaths, InitializePaths, 0, 0, FinalizePaths)
