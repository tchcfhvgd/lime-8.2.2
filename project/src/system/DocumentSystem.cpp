#include <system/DocumentSystem.h>

namespace lime
{

    DocumentSystem::DocumentSystem(const char *treeUri)
    {
        JNIEnv *env = (JNIEnv *)JNI::GetEnv();
        jclass cls = env->FindClass("org/haxe/lime/DocumentSystem");
        jmethodID constructor = env->GetMethodID(cls, "<init>", "(Ljava/lang/String;)V");
        jstring jTreeUri = env->NewStringUTF(treeUri);
        jobject obj = env->NewObject(cls, constructor, jTreeUri);

        if (obj == nullptr)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Lime", "Failed to create a DocumentSystem Java object: %p", pthread_self());
            return;
        }

        this->javaObject = env->NewGlobalRef(obj);
        env->DeleteLocalRef(jTreeUri);
        env->DeleteLocalRef(obj);
    }

    DocumentSystem::~DocumentSystem()
    {
        JNIEnv *env = (JNIEnv *)JNI::GetEnv();
        if (this->javaObject != nullptr)
        {
            env->DeleteGlobalRef(this->javaObject);
        }
    }

    const QuickVec<unsigned char> DocumentSystem::readBytes(const char *path)
    {
        JNIEnv *env = (JNIEnv *)JNI::GetEnv();
        jmethodID mid = env->GetMethodID(env->GetObjectClass(this->javaObject), "readBytes", "(Ljava/lang/String;)[B");
        jstring jPath = env->NewStringUTF(path);
        jobject jResult = env->CallObjectMethod(this->javaObject, mid, jPath);

        if (env->ExceptionCheck() || jResult == NULL)
        {
            if (env->ExceptionCheck())
            {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
            env->DeleteLocalRef(jPath);
            return QuickVec<unsigned char>();
        }

        jboolean copy;
        int length = env->GetArrayLength((jarray)jResult);
        jbyte *jData = env->GetByteArrayElements((jbyteArray)jResult, &copy);
        unsigned char data[length];

        for (int i = 0; i < length; i++)
        {
            data[i] = static_cast<unsigned char>(jData[i]);
        }

        env->ReleaseByteArrayElements((jbyteArray)jResult, jData, JNI_ABORT);
        env->DeleteLocalRef(jPath);
        env->DeleteLocalRef(jResult);

        QuickVec<unsigned char> results;
        results.append(data, length);

        return results;
    }

    void DocumentSystem::writeBytes(const char *path, Bytes *data)
    {
        JNIEnv *env = (JNIEnv *)JNI::GetEnv();
        jmethodID mid = env->GetMethodID(env->GetObjectClass(this->javaObject), "writeBytes", "(Ljava/lang/String;[B)V");
        jstring jPath = env->NewStringUTF(path);
        jbyteArray jData = env->NewByteArray(data->length);

        env->SetByteArrayRegion(jData, 0, data->length, reinterpret_cast<const jbyte *>(data->b));
        env->CallVoidMethod(this->javaObject, mid, jPath, jData);

        env->DeleteLocalRef(jPath);
        env->DeleteLocalRef(jData);
    }

}