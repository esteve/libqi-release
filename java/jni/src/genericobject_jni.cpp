/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qi/future.hpp>

#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qimessaging/c/object_c.h>
#include <qimessaging/c/future_c.h>

#include "genericobject_jni.hpp"
#include "jobjectconverter.hpp"
#include "jnitools.hpp"

static MethodInfoHandler gInfoHandler;

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectCreate()
{
  return (jlong) qi_object_create();
}

void    Java_com_aldebaran_qimessaging_GenericObject_qiObjectDestroy(JNIEnv *env, jobject jobj, jlong pObject)
{
  gInfoHandler.pop(jobj);
  qi_object_destroy(reinterpret_cast<qi_object_t*>(pObject));
}

static jobject qi_generic_call_java(JNIEnv *env, qi::ObjectPtr object, const std::string& strMethodName, jobjectArray listParams)
{
  qi::GenericFunctionParameters params;
  std::string signature;
  jsize size;
  jsize i = 0;

  size = env->GetArrayLength(listParams);
  while (i < size)
  {
    jobject current = env->GetObjectArrayElement(listParams, i);
    qi::GenericValuePtr val = qi::GenericValuePtr(current).clone();
    params.push_back(val);
    signature += val.signature();
    i++;
  }

  qi::Future<qi::GenericValuePtr> fut;
  try {
    fut = object->metaCall(strMethodName + "::(" + signature + ")", params);
  } catch (std::bad_alloc &e)
  {
    throwJavaError(env, e.what());
    return 0;
  } catch (std::runtime_error &e)
  {
    throwJavaError(env, e.what());
    return 0;
  }

  qi::Promise<jobject> out;
  if (fut.hasError())
  {
    out.setError(fut.error());
    throwJavaError(env, out.future().error().c_str());
    return 0;
  }

  jobject ret = fut->value().to<jobject>();
  delete fut;
  return ret;
}

jobject Java_com_aldebaran_qimessaging_GenericObject_qiObjectCall(JNIEnv *env, jobject jobj, jlong pObject, jstring jmethod, jobjectArray args)
{
  qi::ObjectPtr obj = *(reinterpret_cast<qi::ObjectPtr *>(pObject));
  jobject      ret = 0;
  std::string  method;

  // Init JVM singleton and attach current thread to JVM
  JVM(env);
  JVM()->AttachCurrentThread((envPtr) &env, (void*) 0);

  if (!obj)
  {
    qiLogError("qimessaging.java") << "Given object not valid.";
    throwJavaError(env, "Given object is not valid.");
    return ret;
  }

  // Get method name and parameters C style.
  method = toStdString(env, jmethod);
  return qi_generic_call_java(env, obj, method, (jobjectArray) args);
}

qi::GenericValuePtr java_call
(std::string signature, void* data, const qi::GenericFunctionParameters& params)
{
  qi::GenericValuePtr res;
  jvalue*             args = new jvalue[params.size()];
  int                 index = 0;
  JNIEnv*             env = 0;
  qi_method_info*     info = reinterpret_cast<qi_method_info*>(data);
  std::vector<std::string>  sigInfo = qi::signatureSplit(signature);

  // Attach JNIEnv to current thread to avoid segfault in jni functions. (eventloop dependent)
  if (JVM()->AttachCurrentThread((envPtr) &env, (void *) 0) != JNI_OK || env == 0)
  {
    qiLogError("qimessaging.java") << "Cannot attach callback thread to Java VM";
    throwJavaError(env, "Cannot attach callback thread to Java VM");
    return res;
  }

  // Check value of method info structure
  if (info == 0)
  {
    qiLogError("qimessaging.java") << "Internal method informations are not valid";
    throwJavaError(env, "Internal method informations are not valid");
    return res;
  }

  // Translate parameters from GenericValues to jobjects
  qi::GenericFunctionParameters::const_iterator it = params.begin();
  qi::GenericFunctionParameters::const_iterator end = params.end();
  for(; it != end; it++)
  {
    jvalue value;
    value.l = JObject_from_GenericValue(*it);
    args[index] = value;
    index++;
  }

  // Finally call method
  // If method is declared as static, it won't work, doc that.
  jclass cls = env->FindClass(info->className.c_str());
  jmethodID mid = env->GetMethodID(cls, sigInfo[1].c_str(), toJavaSignature(signature).c_str());
  if (!mid)
  {
    qiLogError("qimessaging.jni") << "Cannot find java method " << sigInfo[1] << toJavaSignature(signature).c_str();
    throw new std::runtime_error("Cannot find method");
  }

  jobject ret = env->CallObjectMethodA(info->instance, mid, args);

  delete[] args;
  if (sigInfo[0] == "")
  {
    env->DeleteLocalRef(ret);
    return qi::GenericValuePtr(qi::typeOf<void>());
  }

  return GenericValue_from_JObject(ret).first;
}

jlong Java_com_aldebaran_qimessaging_GenericObject_qiObjectRegisterMethod
(JNIEnv *env, jobject jobj, jlong pObjectBuilder, jstring method, jobject instance, jstring className)
{
  qi::GenericObjectBuilder  *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  std::string                signature = toStdString(env, method);
  std::string                complete_signature = signature;
  qi_method_info*            data;
  std::vector<std::string>  sigInfo;

  // Keep a pointer on JavaVM singleton if not already set.
  JVM(env);

  // Create a new global reference on object instance.
  // jobject structure are local reference and are destroyed when returning to JVM
  // Fixme : May leak global ref.
  instance = env->NewGlobalRef(instance);

  // Create a struct holding a jobject instance, jmethodId id and other needed thing for callback
  // Pass it to void * data to register_method
  // In java_callback, use it directly so we don't have to find method again
  data = new qi_method_info(instance, signature, jobj, toStdString(env, className));
  gInfoHandler.push(data);

  // Bind method signature on generic java callback
  sigInfo = qi::signatureSplit(signature);
  signature = sigInfo[1];
  signature.append("::");
  signature.append(sigInfo[2]);
  int ret = ob->xAdvertiseMethod(sigInfo[0], signature,
    makeDynamicGenericFunction(
        boost::bind(&java_call, complete_signature, data, _1)).dropFirstArgument());

  return (jlong) ret;
}
